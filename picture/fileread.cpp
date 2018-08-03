#include "stdafx.h"

#include "fileread.h"

#ifdef USE_GDIGUI

//BLENDFUNCTION bf = { 0 };
//bf.BlendOp = AC_SRC_OVER;
//bf.BlendFlags = 0;
//bf.AlphaFormat = 0;
//bf.SourceConstantAlpha = 10;
//showpDC->AlphaBlend(base.x, base.y
//	, width, height
//	, mDC->pDC, 0, 0, width, height, bf);

void CrossImage(CImage & img)
{
	//对透明像素进行转换
	for (int i = 0; i < img.GetWidth(); i++)
	{
		for (int j = 0; j < img.GetHeight(); j++)
		{
			UCHAR *cr = (UCHAR*)img.GetPixelAddress(i, j);
			cr[0] = cr[0] * cr[3] / 255;
			cr[1] = cr[1] * cr[3] / 255;
			cr[2] = cr[2] * cr[3] / 255;
		}
	}
}

void memPic::SetSize(int width, int height)
{
	if (!m_Image.IsNull() && width > 0 && height > 0)
	{
		CImage temp;
		if (info.pictype == PICTYPE_PNG && info.colortype == 6)
		{
			temp.Create(width, height, 32, CImage::createAlphaChannel);
		}
		else
		{
			temp.Create(width, height, 24, 0);
		}

		//处理失真
		HDC destdc = temp.GetDC();
		SetStretchBltMode(destdc, COLORONCOLOR);
		//SetBrushOrgEx(destdc, 0, 0, NULL);

		m_Image.StretchBlt(destdc, { 0,0,(LONG)width,(LONG)height });//缩放
		temp.ReleaseDC();

		//销毁并新建m_Image
		m_Image.Destroy();
		if (info.pictype == PICTYPE_PNG && info.colortype == 6)
			m_Image.Create(width, height, 32, CImage::createAlphaChannel);
		else
			m_Image.Create(width, height, 24, 0);
		temp.Draw(m_Image.GetDC(), { 0,0,(LONG)width,(LONG)height });//拷贝回m_Image

																	 //TRANSLATION_CIMAGE(m_Image, 1, -1);//测试平移
		m_Image.ReleaseDC();
		temp.Destroy();
		//m_Image = temp;
	}
}

bool memPic::Read_1(WCHAR * filename, CDC *pDC)
{
	if (!info.ReadFile(filename))
		return false;
	UINT64 ImageLength = info.bytecount; // 文件字节个数
	int WIDTH = info.width, HEIGHT = info.height; //图像尺寸

												  //新建文件字节数组空间
	BYTE *ppic = new BYTE[(size_t)ImageLength];//可能丢失数据
											   //读取数据
	ifstream in(filename, ios::binary | ios::_Nocreate | 0x02);
	in.read((char*)ppic, info.bytecount);
	in.close();

	//导入到void＊，生成IStream *
	HGLOBAL hGlobal;
	IStream *pStm;
	void *pvData;
	hGlobal = GlobalAlloc(GMEM_MOVEABLE, (size_t)ImageLength);
	CreateStreamOnHGlobal(hGlobal, FALSE, &pStm);
	pvData = (BYTE*)GlobalLock(hGlobal);
	memcpy(pvData, ppic, (size_t)ImageLength);
	//清空文件字节数组空间
	delete[] ppic;

	//导入到IPicture＊
	ULARGE_INTEGER pSeek;
	LARGE_INTEGER dlibMove = { 0 };
	IPicture *pPic;
	pStm->Seek(dlibMove, STREAM_SEEK_SET, &pSeek);
	if (FAILED(OleLoadPicture(pStm, (UINT32)ImageLength, TRUE, IID_IPicture, (LPVOID*)&pPic)))
	{
		//PNG return:CTL_E_INVALIDPICTURE
		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
		return false;
	}

	//新建memDCBMP
	draft = new memDCBMP;
	MyDrawPrepareOne(pDC, draft, 0, 0, CRect(0, 0, WIDTH, HEIGHT));

	//Image img(pStm, 0);
	//导入图片到memDCBMP（存储）
	OLE_XSIZE_HIMETRIC m_Height, m_Width;
	pPic->get_Height(&m_Height);//获取图像高度
	pPic->get_Width(&m_Width);
	pPic->Render(draft->pDC->m_hDC, 0, HEIGHT, WIDTH, -HEIGHT, 0, 0, m_Width, m_Height, NULL);

	//清除申请空间
	if (pPic)
		pPic->Release();
	GlobalUnlock(hGlobal);
	GlobalFree(hGlobal);

	return true;
}

bool memPic::Read(WCHAR * filename)
{
	if (!info.ReadFile(filename))
		return false;

	if (!m_Image.IsNull())//如果已有图片，先detach
		m_Image.Detach();

	if (!SUCCEEDED(m_Image.Load(filename)))
		return false;

	if (info.pictype == PICTYPE_PNG && info.colortype == 6)
	{
		CrossImage(m_Image);
	}

	return true;
}

bool memPic::Show_1(CDC * pDC, int x, int y, BYTE alpha)
{
	if (!draft)
		return false;

	pDC->BitBlt(x, y, info.width, info.height
		, draft->pDC, 0, 0, SRCCOPY);

	return true;
}

bool memPic::Show(CDC * pDC, int x, int y, BYTE alpha, bool redrawback, RECT srcrect)
{
	if (redrawback)
	{
		RECT refreshrect = { x - 1, y - 1, x + m_Image.GetWidth(), y + m_Image.GetHeight() };
		pDC->FillSolidRect(&refreshrect, RGB(240, 240, 240));
	}

	if (!m_Image.IsNull())
	{
		if (redrawback && !(srcrect.left == 0 && srcrect.top == 0 && srcrect.right == 0 && srcrect.bottom == 0))
		{
			RECT destrect = { x, y, x + WIDTHOF(srcrect) , y + HEIGHTOF(srcrect) };
			m_Image.AlphaBlend(pDC->GetSafeHdc(), destrect, srcrect, 255, AC_SRC_OVER);
		}
		else
		{
			m_Image.AlphaBlend(pDC->GetSafeHdc(), { x,y }, alpha, AC_SRC_OVER);
			//m_Image.BitBlt(pDC->GetSafeHdc(), { x,y });
		}
	}

	return true;
}

bool memPic::LoadImg(WCHAR * file, int w, int h)
{
	if (file)
		Read(file);
	else
		return false;
	SetSize(w, h);

	return true;
}

void File::SetDir(WCHAR *d)
{
	if (d)
	{
		wcscpy_s(dir, d);
	}
}

void File::SetSuffix(WCHAR *suf)
{
	if (suf)
	{
		wcscpy_s(suffix, suf);
	}
}

void File::SetType(WCHAR *t)
{
	if (t)
	{
		wcscpy_s(type, t);
	}
}

bool File::Open(WCHAR * file, ios_base::openmode _Mode)
{
	if (!file)
		return false;

	wcscpy_s(name, file);//更新文件名

	WCHAR fulldir[MAX_PATH];
	wcscpy_s(fulldir, dir);
	wcscat_s(fulldir, name);
	wcscat_s(fulldir, suffix);
	wcscat_s(fulldir, type);

	fin.open(fulldir, (_Mode & (~ios_base::out)) | ios_base::in);
	fout.open(fulldir, (_Mode & (~ios_base::in)) | ios_base::out);

	if (!fin.is_open() || !fout.is_open())
		return false;

	return true;
}

void File::Close()
{
	SAFE_CLOSEFSTREAM(fin)
		SAFE_CLOSEFSTREAM(fout)
}

#endif // USE_GDIGUI