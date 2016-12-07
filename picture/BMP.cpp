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
	alphaon = false;
}

void BMP::SetAlpha(bool alpha)
{
	alphaon = alpha;
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
	int wrange = min(surfaceDesc.Width, width);//需要拷贝的区域
	int hrange = min(surfaceDesc.Height, height);
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
		for (int j = 0; j < width; j++)
		{
			byte *ppix = (byte*)&((DWORD*)data[i])[j];
			ppix[0] = 255 - ppix[0];
			ppix[1] = 255 - ppix[1];
			ppix[2] = 255 - ppix[2];
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

Surfer::Surfer() {
	surf = NULL;
	pdev = NULL;
	pbmp = NULL;
	pbmpw = NULL;
	pbmph = NULL;
	pbufw = NULL;
	pbufh = NULL;

	surfrefreshcount = 0;

	QueryPerformanceFrequency(&freq);
	wheeltick.QuadPart = 0;
	lastwheeltick.QuadPart = 0;

	clip = true;
	backcolor = BACKCOLOR_INIT;
	surfzoom = 1.0f;
	oldsurfzoom = 1.0f;
	actualzoomx = 1.0f;
	actualzoomy = 1.0f;
	zoomw = 0;
	zoomh = 0;

	surffailed = true;
	surfclipped = false;

	ZEROPOINT(surfsrc)
	ZEROPOINT(basepoint)
	ZEROPOINT(surfbase)
	ZERORECT(surfrect)

	ZEROPOINT(surfsize)
	picclipped = false;
	outsideclient = false;

	SETPOINT(picpixel, -1, -1)
	picpixelcolor = 0;
	cursorpos = CURSORPOS_BLANK;
	
	//alphablend表初始化
	ALPHABLEND::InitAlphBlendTable(TRANSPARENTBACK_FILLDENSITY, TRANSPARENTBACK_HOLLOWDENSITY);
}

bool Surfer::BindDevice(LPDIRECT3DDEVICE9 device)
{
	pdev = device;
	return pdev != NULL;
}

bool Surfer::BindBuf(UINT * bufw, UINT * bufh)
{
	pbufw = bufw;
	pbufh = bufh;
	return pbufw && pbufh;
}

void Surfer::SetBackcolor(D3DCOLOR bc)
{
	backcolor = bc;
}

bool Surfer::BindBMP(BMP * bmp)
{
	pbmp = bmp;
	if (pbmp)//设置BMP尺寸指针
	{
		pbmpw = &pbmp->width;
		pbmph = &pbmp->height;
	}

	return pbmp != NULL;
}

void Surfer::Refresh()
{
	zoomw = surfzoom*pbmp->width;
	zoomh = surfzoom*pbmp->height;
	//将图片居中
	SurfCenter(*pbufw, *pbufh);
	//导入为surface
	SurfRenew(false);
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

void Surfer::SetBasePoint(POINT bp)
{
	basepoint = bp;
}

void Surfer::Clear()
{
	SAFE_RELEASE(surf);

	ZEROPOINT(surfsrc);
	ZEROPOINT(surfbase);

	cursorpos = CURSORPOS_BLANK;
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

bool PicPack::LoadFile(LPDIRECT3DDEVICE9 pdev, WCHAR file[])
{
	FALSE_RETURN(pdev);

	//获取图像信息
	HRESULT hr;
	ZeroMemory(&d3dimginfo, sizeof(D3DXIMAGE_INFO));
	D3DXGetImageInfoFromFile(file, &d3dimginfo);
	/*if (d3dimginfo.Width == 0 || d3dimginfo.Height == 0)
	{
	return false;
	}*/
	//获取基本信息
	FALSE_RETURN(myimginfo.GetFile(file));

	//创建与图像匹配表面
	LPDIRECT3DSURFACE9 tempsurf = NULL;
	hr = pdev->CreateOffscreenPlainSurface(
		(LONG)myimginfo.width, (LONG)myimginfo.height, D3DFMT_A8R8G8B8
		, D3DPOOL_SYSTEMMEM, &tempsurf, NULL);
	FAILED_RETURN(hr);
	//装载图像
	hr = D3DXLoadSurfaceFromFileW(
		tempsurf, NULL, NULL, file
		, NULL, D3DX_FILTER_NONE, 0x00000000, NULL);
	FAILED_RETURN(hr);

	//存入BMP
	bmp.Load(tempsurf);//从tempsurface加载图片
	bmp.SetAlpha(myimginfo.channels == 4);//设置alpha

	//清除surface
	SAFE_RELEASE(tempsurf);

	return true;
}

bool PicPack::SaveFile(LPDIRECT3DDEVICE9 pdev, WCHAR file[])
{
	if (!pdev || bmp.isEmpty())
	{
		return false;
	}
	HRESULT hr;

	LPDIRECT3DSURFACE9 tempsurf = NULL;
	hr = pdev->CreateOffscreenPlainSurface(
		bmp.width, bmp.height
		, D3DFMT_A8R8G8B8
		, D3DPOOL_SYSTEMMEM
		, &tempsurf
		, NULL);
	FAILED_RETURN(hr);

	FALSE_RETURN(bmp.UpLoad(tempsurf));//从tempsurface加载图片
	hr = D3DXSaveSurfaceToFile(file, d3dimginfo.ImageFileFormat, tempsurf, NULL, NULL);//保存到文件

	return !FAILED(hr);
}

WCHAR * PicPack::GetPicInfoStrW()
{
	WCHAR picinfostr[256];
	//图像尺寸、格式
	swprintf_s(picinfostr, _T("pic size: %d× %d\n\
		format: %S\n\
		file size: %lld Bytes  %.3fMB\n\
		depth: %d\n\
		channels: %d\n\
		alpha: %d")
		, bmp.width, bmp.height
		, GetFMTStr(d3dimginfo.Format).c_str()
		, myimginfo.bytecount, myimginfo.bytecount / 1048576.0f
		, myimginfo.generaldepth
		, myimginfo.channels
		, bmp.alphaon);
	return picinfostr;
}
