#include "stdafx.h"
#include "BMP.h"

BMP::BMP()
{
	//set_new_handler(newhandlerfunc);

	data = NULL;
	alphaon = false;
}

bool BMP::Load(LPDIRECT3DSURFACE9 surf)
{
	if (!surf)
		return false;

	//surf数据
	D3DSURFACE_DESC surfaceDesc;
	surf->GetDesc(&surfaceDesc);

	//获得surf长宽
	int /*oldwidth = width,*/ oldheight = height;
	width = surfaceDesc.Width;
	height = surfaceDesc.Height;
	if (width <= 0 || height <= 0)
		return false;


	//清除原图
	if (data)
	{
		for (int i = 0; i < oldheight; i++)
		{
			if (data[i])
			{
				delete[] data[i];
				data[i] = NULL;
			}
		}

		delete[] data;
		data = NULL;
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
	int index = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			index = i*lockedRect.Pitch / 4 + j;

			((DWORD*)data[i])[j] = surfData[index];
		}
	}
	surf->UnlockRect();

	return true;
}

/*
根据位图创建匹配表面
*/
bool BMP::CreateSurf(LPDIRECT3DDEVICE9 device, LPDIRECT3DSURFACE9 *surf, D3DCOLOR color, int zoom)
{
	if (width <= 0 || height <= 0
		|| !data)
		return false;
	if (!device)
		return false;
	if (zoom <= 0)
		return false;

	HRESULT hr;

	//清除原表面
	if (*surf)
	{
		(*surf)->Release();
		(*surf) = NULL;
	}
	//新建匹配表面
	hr = device->CreateOffscreenPlainSurface(
		width*zoom, height*zoom
		, D3DFMT_A8R8G8B8
		, D3DPOOL_SYSTEMMEM
		, surf
		, NULL);

	if (FAILED(hr))
	{
		//MessageBoxW(NULL, L"CreateOffscreenPlainSurface in BMP FAILED!", L"", 0);
		return false;
	}

	//锁定surface区域
	D3DLOCKED_RECT lockedRect;
	(*surf)->LockRect(&lockedRect, NULL, NULL);
	DWORD *surfData = (DWORD*)lockedRect.pBits;

	//拷贝
	DWORD realcolor = 0;
	byte alpha = 0;
	int index = 0;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			index = i*lockedRect.Pitch / 4 * zoom + j*zoom;

			for (int m = 0; m < zoom; m++)
			{
				for (int n = 0; n < zoom; n++)
				{
					alpha = (byte)(((DWORD*)data[i])[j] >> 24);
					realcolor = 0;
					realcolor += ((((DWORD*)data[i])[j] & 0xFF)*alpha + (color & 0xFF)*(255 - alpha)) / 255;
					realcolor += (((((DWORD*)data[i])[j] >> 8 & 0xFF)*alpha + (color >> 8 & 0xFF)*(255 - alpha)) / 255)<<8;
					realcolor += (((((DWORD*)data[i])[j] >> 16 & 0xFF)*alpha + (color >> 16 & 0xFF)*(255 - alpha)) / 255)<<16;
					realcolor += ((DWORD*)data[i])[j] & 0xFF000000;

					//if (((DWORD*)data[i])[j] == 0)
					//	realcolor = 0;
					surfData[index + m*lockedRect.Pitch / 4 + n] = realcolor;
					//surfData[index + m*lockedRect.Pitch / 4 + n] = ((DWORD*)data[i])[j];
				}
			}
		}
	}
	(*surf)->UnlockRect();

	return true;
}

bool BMP::CreateSurf_Clip(LPDIRECT3DDEVICE9 device, LPDIRECT3DSURFACE9 * surf
	, POINT surfsrc, POINT clientrectsize, POINT &surfbase, bool &surfclipped, float zoom, bool clip
	, D3DCOLOR color, short rotate)
{
	//异常情况
	if (width <= 0 || height <= 0
		|| !data)
		return false;
	if (clientrectsize.x <= 0 || clientrectsize.y <= 0)
		return false;
	if (!device)
		return false;
	if (zoom <= 0)
		return false;

	HRESULT hr;

	//清除原表面
	if (*surf)
	{
		(*surf)->Release();
		(*surf) = NULL;
	}

	//放大后拟定surface的尺寸
	UINT32 zoomw = (UINT32)(width*zoom);
	UINT32 zoomh = (UINT32)(height*zoom);

	//要创建的surface实际大小
	POINT surfsize = { (LONG)zoomw, (LONG)zoomh };
	//实际surface起点（左上角）在拟定surface的坐标
	POINT surfbias = { 0, 0 };

	surfclipped = false;
	if (clip)
	{
		//surface超出窗口区域判断(与窗口客户区无交集)
		if (surfsrc.x > (LONG)zoomw || surfsrc.y > (LONG)zoomh//窗口区域超出放大surface右或下
			|| surfsrc.x + clientrectsize.x <= 0 || surfsrc.y + clientrectsize.y <= 0//区域右下不够surface左或上
			)
		{
			//创建空surface返回
			/*hr = device->CreateOffscreenPlainSurface(
				1, 1
				, D3DFMT_A8R8G8B8
				, D3DPOOL_SYSTEMMEM
				, surf
				, NULL);*/
			return false;
		}

		//clip后的surface尺寸,clip后的surface相对于clip前偏移（偏移用于根据bmp设置surface像素）
		//并修改clip后的surface拷贝到backbuffer的起始点
		if (surfsrc.x > 0)//surface左侧需clip
		{
			surfsize.x -= surfsrc.x;
			surfbias.x += surfsrc.x;
			surfbase.x = 0;

			surfclipped = true;
		}
		else
		{
			surfbase.x = -surfsrc.x;
		}
		if (surfsrc.y > 0)//surface上侧需clip
		{
			surfsize.y -= surfsrc.y;
			surfbias.y += surfsrc.y;
			surfbase.y = 0;

			surfclipped = true;
		}
		else
		{
			surfbase.y = -surfsrc.y;
		}
		if ((LONG)zoomw - surfsrc.x > clientrectsize.x)//surface右侧需clip
		{
			surfsize.x -= zoomw - surfsrc.x - clientrectsize.x;

			surfclipped = true;
		}
		if ((LONG)zoomh - surfsrc.y > clientrectsize.y)//surface下侧需clip
		{
			surfsize.y -= zoomh - surfsrc.y - clientrectsize.y;

			surfclipped = true;
		}
	}

	//新建surface
	hr = device->CreateOffscreenPlainSurface(
		surfsize.x, surfsize.y
		, D3DFMT_A8R8G8B8
		, D3DPOOL_SYSTEMMEM
		, surf
		, NULL);

	if (FAILED(hr))//创建surface失败
	{
		//MessageBoxW(NULL, L"CreateOffscreenPlainSurface in BMP FAILED!", L"", 0);
		return false;
	}

	//锁定surface区域
	D3DLOCKED_RECT lockedRect;
	(*surf)->LockRect(&lockedRect, NULL, NULL);
	DWORD *surfData = (DWORD*)lockedRect.pBits;

	//拷贝
	DWORD realcolor = 0;
	byte alpha = 0;
	int index = 0;
	int zoomint = (int)zoom;

	if (zoom < 1)
	{
		COLORTYPE xpixspan = (COLORTYPE)width / zoomw;//新像素x方向跨度
		COLORTYPE ypixspan = (COLORTYPE)height / zoomh;//新像素y方向跨度
		COLORTYPE spanarea = xpixspan*ypixspan;//新像素面积 x*y
		COLORTYPE *xbase = new COLORTYPE[zoomw + 1];//新像素左上起始点，x方向
		COLORTYPE *ybase = new COLORTYPE[zoomh + 1];//新像素左上起始点，y方向
		for (UINT16 i = 0; i < zoomw + 1; i++)
			xbase[i] = (COLORTYPE)i*width / zoomw;
		for (UINT16 i = 0; i < zoomh + 1; i++)
			ybase[i] = (COLORTYPE)i*height / zoomh;

		COLOR_F3 rcolor = 0;//颜色累加器
		for (int i = 0; i < surfsize.y; i++)
		{
			int starty = (int)ybase[i + surfbias.y];//新像素左上起始点，y方向
			int endy = ceilf2i(ybase[i + surfbias.y + 1]);//新像素左上结束点，y方向
			for (int j = 0; j < surfsize.x; j++)
			{
				index = i*lockedRect.Pitch / 4 + j;

				int startx = (int)xbase[j + surfbias.x];//新像素左上起始点，x方向
				int endx = ceilf2i(xbase[j + surfbias.x + 1]);//新像素左上结束点，x方向

				rcolor = 0;
				COLORTYPE floatsy, floatey;//精确起点终点

				//范围内像素颜色累加
				for (int m = starty; m < endy; m++)
				{
					if (m == (int)ybase[i + surfbias.y])
						floatsy = ybase[i + surfbias.y];
					else
						floatsy = (float)m;
					if (ybase[i + surfbias.y + 1] - m < 1)
						floatey = ybase[i + surfbias.y]+ ypixspan;
					else
						floatey = (float)m + 1;

					for (int n = startx; n < endx; n++)
					{
						COLORTYPE floatsx, floatex;
						if (n == (int)xbase[j + surfbias.x])
							floatsx = xbase[j + surfbias.x];
						else
							floatsx = (float)n;
						if (xbase[j + surfbias.x + 1] - n < 1)
							floatex = xbase[j + surfbias.x]+ xpixspan;
						else
							floatex = (float)n + 1;

						DWORD bmppixel = ((DWORD*)data[m])[n];
						//if (endx - startx == 1 && endy - starty == 1)//防止放大后，大块像素颜色不均匀，但会产生误差
						//	rcolor.Add(bmppixel);
						//else
							rcolor.Add(bmppixel, (floatey - floatsy) * (floatex - floatsx), spanarea);
					}
				}

				/*DWORD bmppixel = rcolor.GetDW();
				alpha = (byte)(bmppixel >> 24);
				realcolor = ((bmppixel & 0xFF)*alpha + (color & 0xFF)*(255 - alpha)) / 255;
				realcolor += (((bmppixel >> 8 & 0xFF)*alpha + (color >> 8 & 0xFF)*(255 - alpha)) / 255) << 8;
				realcolor += (((bmppixel >> 16 & 0xFF)*alpha + (color >> 16 & 0xFF)*(255 - alpha)) / 255) << 16;
				realcolor += (bmppixel & 0xFF000000);*/

				//rcolor.Normalize();
				surfData[index] = rcolor.AlphaBlend(color);
			}
		}
		delete[] xbase;
		delete[] ybase;
	}
	else
	{
		for (int i = 0; i < surfsize.y; i++)
		{
			for (int j = 0; j < surfsize.x; j++)
			{
				index = i*lockedRect.Pitch / 4 + j;

				DWORD bmppixel = ((DWORD*)data[(i + surfbias.y) / zoomint])[(j + surfbias.x) / zoomint];//读取bmp信息时加上surface偏移
				alpha = (byte)(bmppixel >> 24);
				realcolor = ((bmppixel & 0xFF)*alpha + (color & 0xFF)*(255 - alpha)) / 255;
				realcolor += (((bmppixel >> 8 & 0xFF)*alpha + (color >> 8 & 0xFF)*(255 - alpha)) / 255) << 8;
				realcolor += (((bmppixel >> 16 & 0xFF)*alpha + (color >> 16 & 0xFF)*(255 - alpha)) / 255) << 16;
				realcolor += (bmppixel & 0xFF000000);
				//realcolor = bmppixel*alpha + color*(255 - alpha);

				surfData[index] = realcolor;
			}
		}
	}

	(*surf)->UnlockRect();

	return true;
}

bool BMP::GetPixel(int x, int y, DWORD *dest)
{
	if(!data)
		return false;

	if (!data[y])
		return false;

	if (x >= width || y >= height)
		return false;

	*dest = ((DWORD*)data[y])[x];

	return true;
}

void BMP::Clear()
{
	if (data)
	{
		for (int i = 0; i < height; i++)
		{
			if (data[i])
			{
				delete[] data[i];
				data[i] = NULL;
			}
		}

		delete[] data;
		data = NULL;
	}
	width = 0;
	height = 0;
}

bool BMP::Empty()
{
	if(!data)
		return true;

	return false;
}

UINT16 roundf2ui16(float f)
{
	if (f > 0)
		return (UINT16)(f + 0.5);
	else
		return (UINT16)(f - 0.5);
}

DWORD roundf2d(float f)
{
	if (f > 0)
		return (DWORD)(f + 0.5);
	else
		return (DWORD)(f - 0.5);
}

LONG roundf2l(float f)
{
	if (f > 0)
		return (LONG)(f + 0.5);
	else
		return (LONG)(f - 0.5);
}

float roundbf2f(float f)
{
	if (f > 0)
		return (float)(byte)(f + 0.5);
	else
		return (float)(byte)(f - 0.5);
}

int ceilf2i(float f)
{
	if ((int)f != f)
		return (int)(f + 1);
	else
		return (int)f;
}
