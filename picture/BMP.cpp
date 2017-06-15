#include "stdafx.h"
#include "BMP.h"
#include "generalgeo.h"

DWORD ALPHABLEND::lastcolor1 = 0;
DWORD ALPHABLEND::lastresult1 = 0;
DWORD ALPHABLEND::lastcolor2 = 0;
DWORD ALPHABLEND::lastresult2 = 0;

byte **ALPHABLEND::alphablendtable1 = NULL;
byte **ALPHABLEND::alphablendtable2 = NULL;

byte ***ALPHABLEND::alphablendcube1 = NULL;

BMP::BMP()
{
	//set_new_handler(newhandlerfunc);
	width = 0;
	height = 0;

	data = NULL;
	bAlpha = false;
}

BMP::~BMP()
{
	if (data)//释放
	{
		for (int i = 0; i < height; i++)
		{
			SAFE_DELETE_LIST(data[i]);
		}
		DELETE_LIST(data);
	}
}

void BMP::SetAlpha(bool alpha)
{
	bAlpha = alpha;
}

bool BMP::Load(LPDIRECT3DSURFACE9 surf)
{
	if (!surf)
		return false;

	//surf数据
	D3DSURFACE_DESC surfaceDesc;
	surf->GetDesc(&surfaceDesc);

	//获得surf长宽
	int oldheight = height;//用于释放原图
	width = surfaceDesc.Width;
	height = surfaceDesc.Height;
	if (width <= 0 || height <= 0)
		return false;

	//清除原图
	if (data)
	{
		for (int i = 0; i < oldheight; i++)
		{
			SAFE_DELETE_LIST(data[i]);
		}
		DELETE_LIST(data);
	}

	//分配新BMP图
	try
	{
		data = new byte*[height];
		for (int i = 0; i < height; i++)
		{
			data[i] = new byte[4 * width];
		}
	}
	catch (bad_alloc &memExp)
	{
		//失败以后，要么abort要么重分配
		cerr << memExp.what() << endl;
		data = NULL;

		return false;
	}

	//锁定surface区域
	D3DLOCKED_RECT lockedRect;
	surf->LockRect(&lockedRect, NULL, NULL);
	DWORD *surfData = (DWORD*)lockedRect.pBits;

	//拷贝
	int pitch = lockedRect.Pitch / 4;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			((DWORD*)data[i])[j] = surfData[i*pitch + j];
		}
	}

	//解锁surface区域
	surf->UnlockRect();

	return true;
}

bool BMP::UpLoad(LPDIRECT3DSURFACE9 surf)
{
	if (!surf || isEmpty())
		return false;

	//surf数据
	D3DSURFACE_DESC surfaceDesc;
	surf->GetDesc(&surfaceDesc);

	//锁定surface区域
	D3DLOCKED_RECT lockedRect;
	surf->LockRect(&lockedRect, NULL, NULL);
	DWORD *surfData = (DWORD*)lockedRect.pBits;

	//拷贝
	int index = 0;
	int wrange = min((int)surfaceDesc.Width, width);//需要拷贝的区域
	int hrange = min((int)surfaceDesc.Height, height);
	for (int i = 0; i < hrange; i++)
	{
		for (int j = 0; j < wrange; j++)
		{
			index = i*lockedRect.Pitch / 4 + j;

			surfData[index] = ((DWORD*)data[i])[j];
		}
	}
	surf->UnlockRect();

	return true;
}

bool MYCALL1 BMP::Gray()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			byte *ppix = (byte*)&((DWORD*)data[i])[j];
			byte gray = RGB2GRAY(ppix[0], ppix[1], ppix[2]);
			ppix[0] = ppix[1] = ppix[2] = gray;
		}
	}
	return true;
}

bool MYCALL1 BMP::Inverse()
{
	for (int i = 0; i < height; i++)
	{
		byte *ppix = data[i];
		for (int j = 0; j < width; j++)
		{
			/*ppix[0] = ~ppix[0];
			ppix[1] = ~ppix[1];*/
			*(UINT16*)ppix = ~(*(UINT16*)ppix);
			ppix[2] = ~ppix[2];
			ppix += 4;
		}
	}
	return true;
}

bool MYCALL1 BMP::InverseAlpha()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			byte *ppix = (byte*)&((DWORD*)data[i])[j];
			ppix[3] = 255 - ppix[3];
		}
	}
	return true;
}

bool MYCALL1 BMP::InverseAll()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			byte *ppix = (byte*)&((DWORD*)data[i])[j];
			ppix[0] = 255 - ppix[0];
			ppix[1] = 255 - ppix[1];
			ppix[2] = 255 - ppix[2];
			ppix[3] = 255 - ppix[3];
		}
	}
	return true;
}

bool MYCALL1 BMP::LOSE_R()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			((byte*)&((DWORD*)data[i])[j])[2] = 0;
		}
	}
	return true;
}

bool MYCALL1 BMP::LOSE_G()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			((byte*)&((DWORD*)data[i])[j])[1] = 0;
		}
	}
	return true;
}

bool MYCALL1 BMP::LOSE_B()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			((byte*)&((DWORD*)data[i])[j])[0] = 0;
		}
	}
	return true;
}

bool MYCALL1 BMP::RGB2HSV()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			byte *ppix = (byte*)&((DWORD*)data[i])[j];

			byte r = ppix[2];
			byte g = ppix[1];
			byte b = ppix[0];
			byte max = 0;
			byte min = 1;
			if (g > b)
			{
				max = 1;
				min = 0;
			}
			if (r > ppix[max])
				max = 2;
			else if (r < ppix[min])
				min = 2;
			byte minvalue = ppix[min];
			byte maxvalue = ppix[max];

			ppix[0] = maxvalue;//v
			ppix[1] = (maxvalue == 0 ? 0 : ROUND_BYTE(255.0f*(maxvalue - minvalue) / maxvalue));//s

			if (maxvalue == minvalue)
				ppix[2] = 0;
			else
			{
				if (max == 2)
				{
					ppix[2] = ROUND_BYTE(255.0f * (60 * (g - b) / (maxvalue - minvalue)) / 360);
				}
				else if (max == 1)
				{
					ppix[2] = ROUND_BYTE(255.0f * (120 + 60 * (b - r) / (maxvalue - minvalue)) / 360);
				}
				else
				{
					ppix[2] = ROUND_BYTE(255.0f * (240 + 60 * (r - g) / (maxvalue - minvalue)) / 360);
				}
			}
		}
	}
	return true;
}

void MYCALL1 BMP::Clear()
{
	if (data)//释放二维空间
	{
		for (int i = 0; i < height; i++)
		{
			SAFE_DELETE_LIST(data[i]);
		}

		DELETE_LIST(data);
	}
	width = 0;
	height = 0;
}

void Surfer::CalcBindState()
{
	bHasPic = pBmp && pBmpW && pBmpH;
	bHasDevice = pDevice && pBufferW && pBufferH;
	bHasBoth = bHasDevice && bHasPic;
}

Surfer::Surfer() {
	surf = NULL;
	pDevice = NULL;
	pBmp = NULL;
	pBmpW = NULL;
	pBmpH = NULL;
	pBufferW = NULL;
	pBufferH = NULL;
	bHasPic = false;
	bHasDevice = false;
	bHasBoth = false;

	//surfrenewcount = 0;
	renewTime = -0.001f;
	sampleSchema = SAMPLE_SCHEMA_UNKNOWN;
	surferInfoStr[0] = (WCHAR)'\0';

	QueryPerformanceFrequency(&freq);
	wheeltick.QuadPart = 0;
	lastwheeltick.QuadPart = 0;

	bClip = true;
	BackgroundColor = BACKCOLOR_INIT;
	surfZoom = 1.0f;
	lastSurfZoom = 1.0f;
	actualZoomX = 1.0f;
	actualZoomY = 1.0f;
	zoomW = 0;
	zoomH = 0;

	bSurfFailed = true;
	bSurfClipped = false;

	ZEROPOINT(surfBase)
	ZEROPOINT(basePoint)
	ZEROPOINT(surfDest)
	ZERORECT(rcSurf)

	ZEROPOINT(surfSize)
	bPicClipped = false;
	bOutClient = false;

	SETPOINT(cursorPixel, -1, -1)
	bCursorOnPic = false;
	cursorColor = 0;
	cursorPos = CURSORPOS_BLANK;
	
	// alphablend表初始化
	ALPHABLEND::InitAlphBlendTable(TRANSPARENTBACK_FILLDENSITY, TRANSPARENTBACK_HOLLOWDENSITY);

	// 信息字符串初始化
	SetInfoStr();
}

Surfer::~Surfer()
{
	SAFE_RELEASE(surf);
}

bool Surfer::BindDevice(LPDIRECT3DDEVICE9 device)
{
	pDevice = device;

	CalcBindState();

	return pDevice != NULL;
}

bool Surfer::BindBuf(UINT * pBufW, UINT * pBufH)
{
	pBufferW = pBufW;
	pBufferH = pBufH;

	CalcBindState();

	return pBufferW && pBufferH;
}

void Surfer::SetBackcolor(DWORD color)
{
	BackgroundColor = color;
}

bool Surfer::BindPic(PicPack * ppic, bool renew)
{
	if (ppic)
	{
		// 捆绑指针
		pBmp = ppic->GetPBMP();
		pBmpW = &pBmp->width;
		pBmpH = &pBmp->height;

		// 信息导入
		surfBase = ppic->GetBase();
		surfZoom = ppic->GetZoom();
		lastSurfZoom = surfZoom;

		//zoomW, zoomH更新，这些之后未必重新计算，在这里需要计算
		zoomW = (int)(surfZoom**pBmpW);
		zoomH = (int)(surfZoom**pBmpH);
		actualZoomX = (float)zoomW / *pBmpW;
		actualZoomY = (float)zoomH / *pBmpH;

		// 更新捆绑状态
		CalcBindState();

		if (renew)
			SurfRenew(false);
		SetInfoStr();// 更新信息字符串
	}
	else
	{
		Clear();
	}

	return pBmp != NULL;
}

bool Surfer::DeBindPic(PicPack *ppic)
{
	if (ppic)
	{
		ppic->SetSrc(surfBase);
		ppic->SetZoom(surfZoom);

		CalcBindState();

		return true;
	}
	else
		return false;
}

//void Surfer::Refresh()
//{
//	if (bPicOn)
//	{
//		zoomW = (int)(surfZoom**pBmpW);
//		zoomH = (int)(surfZoom**pBmpH);
//		actualZoomX = (float)zoomW / *pBmpW;
//		actualZoomY = (float)zoomH / *pBmpH;
//
//		SurfRenew(false);
//		SetInfoStr();// 更新信息字符串
//	}
//	else
//	{
//
//	}
//}

void Surfer::SetBasePoint(POINT color)
{
	basePoint = color;
}

void Surfer::PostSurfPosChange()
{
	bOutClient = (
		surfBase.x > zoomW || surfBase.y > zoomH	//区域超出surface右或下
		|| surfBase.x + (int)*pBufferW <= 0			//区域右下不够surface左或上
		|| surfBase.y + (int)*pBufferH <= 0
		);
	bPicClipped = (
		surfBase.x > 0 || surfBase.y > 0			//surface左上上侧需bClip
		|| zoomW - surfBase.x > (int)*pBufferW		//surface右侧需bClip
		|| zoomH - surfBase.y > (int)*pBufferH		//surface下侧需bClip
		);

	SETPOINT(surfSize, (LONG)zoomW, (LONG)zoomH);
	if (bClip)
	{
		// surface左侧clip
		if (surfBase.x > 0)
			surfSize.x -= surfBase.x;
		// surface上侧clip
		if (surfBase.y > 0)
			surfSize.y -= surfBase.y;
		// surface右侧clip
		if ((LONG)zoomW - surfBase.x > (int)*pBufferW)
			surfSize.x -= zoomW - surfBase.x - (int)*pBufferW;
		// surface下侧clip
		if ((LONG)zoomH - surfBase.y > (int)*pBufferH)
			surfSize.y -= zoomH - surfBase.y - (int)*pBufferH;
	}
}

void Surfer::Clear()
{
	SAFE_RELEASE(surf);//释放surface
	pBmp = NULL;
	pBmpW = NULL;
	pBmpH = NULL;

	sampleSchema = SAMPLE_SCHEMA_UNKNOWN;

	zoomW = 0;
	zoomH = 0;

	actualZoomX = 0;
	actualZoomY = 0;
	bSurfFailed = true;
	bSurfClipped = false;

	ZEROPOINT(surfSize)
	bPicClipped = false;
	bOutClient = false;

	ZEROPOINT(cursorPixel)
	cursorColor = 0;
	cursorPos = CURSORPOS_BLANK;//更新标志

	CalcBindState();
	SetInfoStr();
}

void ALPHABLEND::InitAlphBlendTable(byte color1, byte color2)
{
	if (alphablendtable1)
	{
		for (int i = 0; i < 256; i++)
		{
			if (alphablendtable1[i])
				delete[] alphablendtable1[i];
		}
		delete[] alphablendtable1;
	}
	if (alphablendtable2)
	{
		for (int i = 0; i < 256; i++)
		{
			if (alphablendtable2[i])
				delete[] alphablendtable2[i];
		}
		delete[] alphablendtable2;
	}

	alphablendtable1 = new byte*[256];
	alphablendtable2 = new byte*[256];
	for (int i = 0; i < 256; i++)
	{
		alphablendtable1[i] = new byte[256];
		alphablendtable2[i] = new byte[256];
		for (int j = 0; j < 256; j++)//i是源通道值，j是alpha，color1，color2是目标通道值
		{

			alphablendtable1[i][j] = (i*j + (255 - j)*color1) / 255;
			alphablendtable2[i][j] = (i*j + (255 - j)*color2) / 255;
		}
	}
	lastcolor1 = 0;
	lastresult1 = color1;
	lastcolor2 = 0;
	lastresult2 = color2;
}

void ALPHABLEND::InitAlphBlendTCube()
{
	const int alphalevels = 20;//alpha分为多少级（考虑到输入alpha是float形式）

	alphablendcube1 = new byte**[alphalevels];
	for (int i = 0; i < alphalevels; i++)//i,j:src、dest。k:alpha
	{
		alphablendcube1[i] = new byte*[256];
		for (int j = 0; j < 256; j++)
		{
			alphablendcube1[i][j] = new byte[256];

			for (int k = 0; k < 256; k++)
			{
				alphablendcube1[i][j][k]
					= (byte)((i*j + (float)(alphalevels - 1 - i)*k) / (alphalevels - 1));
			}
		}
	}
}

PicPack::PicPack() 
{
	pBmp = new BMP;

	ZeroMemory(&myPicInfo, sizeof(myPicInfo));
	ZeroMemory(&D3DPicInfo, sizeof(D3DPicInfo));
	strFileName[0] = L'\0';

	ZEROPOINT(src)
	zoom = 1.0f;

	bSaved = false;
	bHasDirectory = false;
}

PicPack::~PicPack()
{
	delete pBmp;
	pBmp = NULL;
}

WCHAR * PicPack::GetFileName()
{
	return strFileName;
}

HRESULT PicPack::LoadFile(LPDIRECT3DDEVICE9 pDevice, WCHAR file[])
{
	if (!pDevice)
		return E_FAIL;

	// 获取图片信息
	HRESULT hr;
	ZeroMemory(&D3DPicInfo, sizeof(D3DXIMAGE_INFO));
	D3DXGetImageInfoFromFileW(file, &D3DPicInfo);
	/*if (D3DPicInfo.Width == 0 || D3DPicInfo.Height == 0)
	{
	return false;
	}*/
	//获取基本信息
	if(!myPicInfo.ReadFile(file))
		return E_FAIL;

	// 创建与图片匹配表面
	LPDIRECT3DSURFACE9 tempsurf = NULL;
	hr = pDevice->CreateOffscreenPlainSurface(
		(LONG)myPicInfo.width, (LONG)myPicInfo.height, D3DFMT_A8R8G8B8
		, D3DPOOL_SYSTEMMEM, &tempsurf, NULL);
	FAILED_RETURN(hr);
	// 装载图片
	hr = D3DXLoadSurfaceFromFileW(
		tempsurf, NULL, NULL, file
		, NULL, D3DX_FILTER_NONE, 0x00000000, NULL);
	FAILED_RETURN(hr);
	//const DWORD TT = S_FALSE;
	//S_OK;

	// 存入BMP
	if (!pBmp->Load(tempsurf)) return E_FAIL;//从tempsurface加载图片
	pBmp->SetAlpha(myPicInfo.channels == 4);//设置alpha

	// 文件名
	wcscpy_s(strFileName, file);

	// 状态更新
	bHasDirectory = true;
	bSaved = true;

	// 清除surface
	SAFE_RELEASE(tempsurf);

	return S_OK;
}

bool PicPack::SaveFile(LPDIRECT3DDEVICE9 pDevice, WCHAR file[])
{
	if (!pDevice || pBmp->isEmpty())
	{
		return false;
	}
	HRESULT hr;

	LPDIRECT3DSURFACE9 tempsurf = NULL;
	hr = pDevice->CreateOffscreenPlainSurface(
		pBmp->width, pBmp->height
		, D3DFMT_A8R8G8B8
		, D3DPOOL_SYSTEMMEM
		, &tempsurf
		, NULL);
	FAILED_RETURN_FALSE(hr);

	FALSE_RETURN(pBmp->UpLoad(tempsurf));//从tempsurface加载图片
	hr = D3DXSaveSurfaceToFile(file, D3DPicInfo.ImageFileFormat, tempsurf, NULL, NULL);//保存到文件

	// 文件名
	wcscpy_s(strFileName, file);

	// 状态更新
	bSaved = true;

	return !FAILED(hr);
}

WCHAR * PicPack::GetPicInfoStrW()
{
	//图片尺寸、格式
	swprintf_s(picInfoStr, _T("SIZE: %d × %d\n\
		FORMAT: %S\n\
		FILE: %lld Bytes  [%.3fMB]\n\
		DEPTH: %d\n\
		CHANNEL: %d\n\
		ALPHA: %d")
		, pBmp->width, pBmp->height
		, GetFMTStr(D3DPicInfo.Format).c_str()
		, myPicInfo.bytecount, myPicInfo.bytecount / 1048576.0f
		, myPicInfo.generaldepth
		, myPicInfo.channels
		, pBmp->bAlpha);
	return picInfoStr;
}

string GetFMTStr(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_R8G8B8:
		return "D3DFMT_R8G8B8";
	case D3DFMT_A8R8G8B8:
		return "D3DFMT_A8R8G8B8";
	case D3DFMT_X8R8G8B8:
		return "D3DFMT_X8R8G8B8";
	case D3DFMT_R5G6B5:
		return "D3DFMT_R5G6B5";
	case D3DFMT_X1R5G5B5:
		return "D3DFMT_X1R5G5B5";
	case D3DFMT_A1R5G5B5:
		return "D3DFMT_A1R5G5B5";
	case D3DFMT_A4R4G4B4:
		return "D3DFMT_A4R4G4B4";
	case D3DFMT_R3G3B2:
		return "D3DFMT_R3G3B2";
	case D3DFMT_A8:
		return "D3DFMT_A8";
	case D3DFMT_A8R3G3B2:
		return "D3DFMT_A8R3G3B2";
	case D3DFMT_X4R4G4B4:
		return "D3DFMT_X4R4G4B4";
	case D3DFMT_A2B10G10R10:
		return "D3DFMT_A2B10G10R10";
	case D3DFMT_A8B8G8R8:
		return "D3DFMT_A8B8G8R8";
	case D3DFMT_X8B8G8R8:
		return "D3DFMT_X8B8G8R8";
	case D3DFMT_G16R16:
		return "D3DFMT_G16R16";
	case D3DFMT_A2R10G10B10:
		return "D3DFMT_A2R10G10B10";
	case D3DFMT_A16B16G16R16:
		return "D3DFMT_A16B16G16R16";
	case D3DFMT_A8P8:
		return "D3DFMT_A8P8";
	case D3DFMT_P8:
		return "D3DFMT_P8";
	case D3DFMT_L8:
		return "D3DFMT_L8";
	case D3DFMT_A8L8:
		return "D3DFMT_A8L8";
	case D3DFMT_A4L4:
		return "D3DFMT_A4L4";
	case D3DFMT_V8U8:
		return "D3DFMT_V8U8";
	case D3DFMT_L6V5U5:
		return "D3DFMT_L6V5U5";
	case D3DFMT_X8L8V8U8:
		return "D3DFMT_X8L8V8U8";
	case D3DFMT_Q8W8V8U8:
		return "D3DFMT_Q8W8V8U8";
	case D3DFMT_V16U16:
		return "D3DFMT_V16U16";
	case D3DFMT_A2W10V10U10:
		return "D3DFMT_A2W10V10U10";
	default:
		return "UNKNOWN";
	}
}

PicPackList::PicPackList()
{
	size = PICPACKLIST_SIZE_INIT;
	data = MALLOC(PicPack*, size);
	count = 0;
	cur = 0;

	pLivePicPack = NULL;
	pLiveBMP = NULL;
}

int PicPackList::GetCurPos()
{
	return cur;
}

int PicPackList::GetCount()
{
	return count;
}

PicPack * PicPackList::SetPicPack(int idx)
{
	if (count < 1)
		return NULL;

	if (idx <= count && idx > 0)
	{
		cur = idx;

		pLivePicPack = data[cur - 1];
		pLiveBMP = pLivePicPack->GetPBMP();

		return data[cur - 1];
	}
	else
	{
		cur = 0;
		return NULL;
	}
}

PicPack * PicPackList::SetTailPicPack()
{
	if (count < 1)
		return NULL;

	cur = count;

	pLivePicPack = data[cur - 1];
	pLiveBMP = pLivePicPack->GetPBMP();

	return data[cur - 1];
}

PicPack * PicPackList::SetPrev()
{
	if (count < 1)
		return NULL;

	int oldcur = cur;
	cur--;
	if (cur < 1)
		cur = count;

	if (cur <= count && cur > 0)
	{
		pLivePicPack = data[cur - 1];
		pLiveBMP = pLivePicPack->GetPBMP();

		return data[cur - 1];
	}
	else
	{
		cur = oldcur;

		return NULL;
	}
}

PicPack * PicPackList::SetNext()
{
	if (count < 1)
		return NULL;

	int oldcur = cur;
	cur++;
	if (cur > count)
		cur = 1;

	if (cur <= count && cur > 0)
	{
		pLivePicPack = data[cur - 1];
		pLiveBMP = pLivePicPack->GetPBMP();

		return data[cur - 1];
	}
	else
	{
		cur = oldcur;

		return NULL;
	}
}

PicPack * PicPackList::Drop()
{
	if (count < 1 || cur>count || cur <= 0)
		return NULL;

	delete data[cur - 1];
	for (int i = cur; i <= count - 1; i++)
	{
		data[i - 1] = data[i];
	}
	if (cur == count)
		cur--;
	count--;

	if (count >= 1 && cur <= count && cur > 0)//删除后还有图片
	{
		pLivePicPack = data[cur - 1];
		pLiveBMP = pLivePicPack->GetPBMP();

		return data[cur - 1];
	}
	else//删除后列表中没图片了
	{
		cur = 0;

		pLivePicPack = NULL;
		pLiveBMP = NULL;

		return NULL;
	}
}

//void PicPackList::Add()
//{
//	if (count >= size)
//	{
//		size += 2;
//		data = (PicPack**)realloc(data, size*sizeof(PicPack*));
//	}
//
//	count++;
//	data[count - 1] = new PicPack;
//}

bool PicPackList::Add(PicPack * newpp)
{
	if (newpp)
	{
		if (count >= size)
		{
			size += PICPACKLIST_SIZE_INCREMENT;
			REALLOC(data, PicPack*, size)
		}
		count++;
		data[count - 1] = newpp;
		return true;
	}
	else
	{
		return false;
	}
}

COLOR_F3::COLOR_F3()
{
	r = g = b = 0;
}

COLOR_F3::COLOR_F3(COLORTYPE f)
{
	r = g = b = f; 
}

void COLOR_F3::Normalize()
{
	if (a > 255.0)
		a = 255.0;
	if (r > 255.0)
		r = 255.0;
	if (g > 255.0)
		g = 255.0;
	if (b > 255.0)
		b = 255.0;

	a = (float)(byte)(a + 0.5f);
	r = (float)(byte)(r + 0.5f);
	g = (float)(byte)(g + 0.5f);
	b = (float)(byte)(b + 0.5f);
}
