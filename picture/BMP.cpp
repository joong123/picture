#include "stdafx.h"
#include "BMP.h"

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
	, D3DCOLOR color, float rotate)
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

	//放大后假想surface的尺寸
	UINT32 zoomw = (UINT32)(width*zoom);
	UINT32 zoomh = (UINT32)(height*zoom);

	POINT surfsize = { (LONG)zoomw, (LONG)zoomh };	//要创建的surface实际大小
	POINT surfbias = { 0, 0 };						//实际surface起点（左上角）相对假想surface的位置
	surfclipped = false;							//surface是否clip
	if (clip)
	{
		//clip异常：surface超出窗口区域(与窗口客户区无交集)
		if (
			//假想surface超出窗口区域左上角（左或上）
			surfsrc.x > (LONG)zoomw || surfsrc.y > (LONG)zoomh
			//假想surface超出窗口区域右下角（右或下）
			|| surfsrc.x + clientrectsize.x <= 0 || surfsrc.y + clientrectsize.y <= 0
			)
		{
			/*hr = device->CreateOffscreenPlainSurface(1, 1
			, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, surf, NULL);//空surface*/
			return false;
		}

		//计算surfsize	 ：clip后的surface尺寸,
		//计算surfbias	 ：clip后的surface相对于假想surface的偏移
		//计算surfclipped ：实际surface是否clip
		//（偏移用于定位图片区域来设置surface像素）
		//surface左侧需clip
		if (surfsrc.x > 0)
		{
			surfsize.x -= surfsrc.x;
			surfbias.x = surfsrc.x;

			surfclipped = true;
		}
		//surface上侧需clip
		if (surfsrc.y > 0)
		{
			surfsize.y -= surfsrc.y;
			surfbias.y = surfsrc.y;

			surfclipped = true;
		}
		//surface右侧需clip
		if ((LONG)zoomw - surfsrc.x > clientrectsize.x)
		{
			surfsize.x -= zoomw - surfsrc.x - clientrectsize.x;

			surfclipped = true;
		}
		//surface下侧需clip
		if ((LONG)zoomh - surfsrc.y > clientrectsize.y)
		{
			surfsize.y -= zoomh - surfsrc.y - clientrectsize.y;

			surfclipped = true;
		}
	}
	if (surfsrc.x > 0)
		surfbase.x = 0;
	else
		surfbase.x = -surfsrc.x;
	if (surfsrc.y > 0)
		surfbase.y = 0;
	else
		surfbase.y = -surfsrc.y;

	POINT backbase;
	if (!clip)
	{
		backbase.x = surfbase.x - max(surfsrc.x, 0);
		backbase.y = surfbase.y - max(surfsrc.y, 0);
	}
	else
	{
		backbase.x = surfbase.x;
		backbase.y = surfbase.y;
	}


	//清除原表面
	if (*surf)
	{
		(*surf)->Release();
		(*surf) = NULL;
	}
	//新建surface
	if(FAILED(device->CreateOffscreenPlainSurface(
		surfsize.x, surfsize.y
		, D3DFMT_A8R8G8B8
		, D3DPOOL_SYSTEMMEM
		, surf
		, NULL)))
		return false;

	//锁定surface区域
	D3DLOCKED_RECT lockedRect;
	(*surf)->LockRect(&lockedRect, NULL, NULL);
	DWORD *surfData = (DWORD*)lockedRect.pBits;
	
	//拷贝
	byte alpha = 0;
	int index = 0;
	if (zoom < 0.5 && width < 2000 && height < 2000)
	{
		//最大重采样
		Copy_MaxSample(surfData, lockedRect.Pitch / 4, surfsize, zoom, rotate, surfbias, backbase);
	}
	else if (zoom < 1)
	{
		//双线性
		Copy_BiLinear(surfData, lockedRect.Pitch / 4, surfsize, zoom, rotate, surfbias, backbase);
	}
	else 
	{
		//小平滑马赛克
		Copy_Single(surfData, lockedRect.Pitch / 4, surfsize, zoom, rotate, surfbias, backbase);
		//Copy_BiLinear(surfData, lockedRect.Pitch / 4, surfsize, zoom, rotate, surfbias, backbase);
		//Copy_NN(surfData, lockedRect.Pitch / 4, surfsize, zoom, rotate, surfbias, backbase);
	}

	(*surf)->UnlockRect();

	return true;
}

inline bool BMP::Copy_MaxSample(DWORD *surfData, int pitch, const POINT &surfsize, float zoom, float rotate, const POINT surfbias, const POINT backbias)
{
	UINT32 zoomw = (UINT32)(width*zoom);
	UINT32 zoomh = (UINT32)(height*zoom);

	COLORTYPE xpixspan = (COLORTYPE)width / zoomw;//新像素x方向跨度
	COLORTYPE ypixspan = (COLORTYPE)height / zoomh;//新像素y方向跨度
	COLORTYPE spanarea = xpixspan*ypixspan;//新像素面积 x*y
	float baseweight = 1.0f / spanarea;
	COLORTYPE *xbase = new COLORTYPE[zoomw + 1];//新像素左上起始点数组，x方向
	COLORTYPE *ybase = new COLORTYPE[zoomh + 1];//新像素左上起始点数组，y方向

	//设置新像素起点值数组,限定需要的区间
	float actualzx = (float)width / zoomw;
	//float actualzy = (float)height / zoomh;
	for (UINT16 i = 0; i < surfsize.x + 1; i++)
		xbase[i] = (COLORTYPE)(i + surfbias.x)*actualzx;
	for (UINT16 i = 0; i < surfsize.y + 1; i++)
		ybase[i] = (COLORTYPE)(i + surfbias.y)*height / zoomh;//height / zoomh

	int index;
	COLOR_F3 rcolor = 0;//颜色累加器
	for (int i = 0; i < surfsize.y; i++)
	{
		int starty = (int)ybase[i];//新像素左上起始点，y方向
		int endy = ceilf2i(ybase[i + 1]);//新像素左上结束点，y方向
		for (int j = 0; j < surfsize.x; j++)
		{ 
			index = i*pitch + j;

			int startx = (int)xbase[j];//新像素左上起始点，x方向
			int endx = ceilf2i(xbase[j + 1]);//新像素左上结束点，x方向

			rcolor = 0;
			COLORTYPE floatsy, floatey;//精确起点终点

			bool isxbase, isybase;
			//范围内像素颜色累加
			for (int m = starty; m < endy; m++)
			{
				isybase = true;
				if (m == (int)ybase[i])//区间起点
				{
					floatsy = ybase[i];
					isybase = false;
				}
				else
				{
					floatsy = (float)m;
				}
				if (ybase[i + 1] - m < 1)//区间终点
				{
					floatey = ybase[i] + ypixspan;//ybase[srcy] + ypixspan，ybase[srcy + 1]
					isybase = false;
				}
				else
				{
					floatey = (float)m + 1;
				}

				COLORTYPE floatsx, floatex;
				for (int n = startx; n < endx; n++)
				{
					isxbase = true;
					if (n == (int)xbase[j])//区间起点
					{
						floatsx = xbase[j];
						isxbase = false;
					}
					else
					{
						floatsx = (float)n;
					}
					if (xbase[j + 1] - n < 1)//区间终点
					{
						floatex = xbase[j] + xpixspan;//xbase[srcx] + xpixspan，xbase[srcx + 1]
						isxbase = false;
					}
					else
					{
						floatex = (float)n + 1;
					}

					//DWORD bmppixel = ((DWORD*)data[m])[n];//原像素
					//if (endx - startx == 1 && endy - starty == 1)//防止放大后，大块像素颜色不均匀，但会产生误差
					//	rcolor.Add(bmppixel);
					//else

					//if (isxbase && isybase)//基本权值情况的加速运算
					//	rcolor.Add(((DWORD*)data[m])[n], baseweight);
					//else
						rcolor.Add(((DWORD*)data[m])[n], (floatey - floatsy) * (floatex - floatsx), spanarea);
				}
			}

			//rcolor.Normalize();

			//copypixel方法
			//surfData[index] = rcolor.GetDW();

			//条件alphablend方法
			if (rcolor.a < 255.0f)
			{
				if (!((((backbias.y + i) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01)
					^ (((backbias.x + j) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01)))
				{//两种背景色
					byte *dest = (byte*)&(surfData[index]);
					byte alpha;
					if (rcolor.a > 0)
						alpha = (byte)(rcolor.a + 0.5f);
					else
						alpha = (byte)(rcolor.a - 0.5f);
					//dest[3] = ALPHABLEND::alphablendtable1[(byte)(rcolor.a + 0.5f)][alpha];
					dest[2] = ALPHABLEND::alphablendtable1[(byte)(rcolor.r + 0.5f)][alpha];
					dest[1] = ALPHABLEND::alphablendtable1[(byte)(rcolor.g + 0.5f)][alpha];
					dest[0] = ALPHABLEND::alphablendtable1[(byte)(rcolor.b + 0.5f)][alpha];

					//surfData[index] = ALPHABLEND::AlphaBlend1(rcolor);
				}
				else
				{
					byte *dest = (byte*)&(surfData[index]);
					byte alpha;
					if (rcolor.a > 0)
						alpha = (byte)(rcolor.a + 0.5f);
					else
						alpha = (byte)(rcolor.a - 0.5f);
					//dest[3] = ALPHABLEND::alphablendtable2[(byte)(rcolor.a + 0.5f)][alpha];
					dest[2] = ALPHABLEND::alphablendtable2[(byte)(rcolor.r + 0.5f)][alpha];
					dest[1] = ALPHABLEND::alphablendtable2[(byte)(rcolor.g + 0.5f)][alpha];
					dest[0] = ALPHABLEND::alphablendtable2[(byte)(rcolor.b + 0.5f)][alpha];

					//surfData[index] = ALPHABLEND::AlphaBlend2(rcolor);
				}
			}
			else
				surfData[index] = rcolor.GetDW();
		}
	}
	delete[] xbase;
	delete[] ybase;

	return true;
}

inline bool BMP::Copy_NoSample(DWORD * surfData, int pitch, const POINT & surfsize, float zoom, float rotate, const POINT surfbias, const POINT backbase)
{
	int zoomint = (int)zoom;
	if (zoomint <= 0)
		return false;
	int index;

	for (int i = 0; i < surfsize.y; i++)
	{
		for (int j = 0; j < surfsize.x; j++)
		{
			index = i*pitch + j;

			DWORD bmppixel = ((DWORD*)data[(i + surfbias.y) / zoomint])[(j + surfbias.x) / zoomint];//读取bmp信息时加上surface偏移

			//copypixel方法
			//surfData[index] = bmppixel;

			//条件alphablend方法
			if ((byte)(bmppixel >> 24) < 255)
			{
				if (!((((backbase.y + i) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01)
						^ (((backbase.x + j) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01)))
				{//两种背景色
					if (bmppixel == ALPHABLEND::lastcolor1)//缓存加速
					{
						surfData[index] = ALPHABLEND::lastresult1;
					}
					else
					{
						byte *dest = (byte*)&(surfData[index]);
						byte *src = (byte*)&bmppixel;
						byte alpha = src[3];

						dest[2] = (DWORD)ALPHABLEND::alphablendtable1[src[2]][alpha];
						dest[1] = (DWORD)ALPHABLEND::alphablendtable1[src[1]][alpha];
						dest[0] = (DWORD)ALPHABLEND::alphablendtable1[src[0]][alpha];

						ALPHABLEND::lastcolor1 = bmppixel;
						ALPHABLEND::lastresult1 = *(DWORD*)dest;
					}

					//surfData[index] = ALPHABLEND::AlphaBlend1(bmppixel);
				}
				else
				{
					if (bmppixel == ALPHABLEND::lastcolor2)//缓存加速
					{
						surfData[index] = ALPHABLEND::lastresult2;
					}
					else
					{
						byte *dest = (byte*)&(surfData[index]);
						byte *src = (byte*)&bmppixel;
						byte alpha = src[3];

						dest[2] = (DWORD)ALPHABLEND::alphablendtable2[src[2]][alpha];
						dest[1] = (DWORD)ALPHABLEND::alphablendtable2[src[1]][alpha];
						dest[0] = (DWORD)ALPHABLEND::alphablendtable2[src[0]][alpha];

						ALPHABLEND::lastcolor2 = bmppixel;
						ALPHABLEND::lastresult2 = *(DWORD*)dest;
					}

					//surfData[index] = ALPHABLEND::AlphaBlend2(bmppixel);
				}
			}
			else
				surfData[index] = bmppixel;
		}
	}

	return true;
}

inline bool BMP::Copy_Single(DWORD * surfData, int pitch, const POINT & surfsize, float zoom, float rotate, const POINT surfbias, const POINT backbase)
{
	int index;

	float *xsto = new float[surfsize.x];//加速
	float *ysto = new float[surfsize.y];//加速
	for (int i = 0; i < surfsize.x; i++)
	{
		xsto[i] = (i + surfbias.x + 0.5f) / zoom - 0.5f;
	}
	for (int i = 0; i < surfsize.y; i++)
	{
		ysto[i] = (i + surfbias.y + 0.5f) / zoom - 0.5f;
	}
	const float auxz = 0.3f / zoom;//加速
	float auxfront = 0.5f - auxz, auxrear = 0.5f + auxz;//加速
	for (int i = 0; i < surfsize.y; i++)
	{
		for (int j = 0; j < surfsize.x; j++)
		{
			index = i*pitch + j;

			float frontx = xsto[j];
			float fronty = ysto[i];
			float frontxbias = frontx - (int)frontx;
			/*if (frontx < 0)//不加不会报错
				frontxbias = 0;*/
			if (frontxbias < auxfront)
				frontxbias = 0;
			else if (frontxbias > auxrear)
				frontxbias = 1;
			else
				frontxbias = (frontxbias - auxfront) * zoom;
			float frontybias = fronty - (int)fronty;
			/*if (fronty < 0)
				frontybias = 0;*/
			if (frontybias <  auxfront)
				frontybias = 0;
			else if (frontybias > auxrear)
				frontybias = 1;
			else
				frontybias = (frontybias - auxfront) * zoom;

			//DWORD bmppixel;
			byte pixel[4];
			if ((frontxbias == 0 || frontxbias == 1)
				&& (frontybias == 0 || frontybias == 1))
			{
				memcpy_s(pixel, 4
					, ((byte*)&((DWORD*)data[(int)(fronty + frontybias)])[(int)(frontx + frontxbias)]), 4);
			}
			else
			{
				byte *lt, *rt, *lb, *rb;
				//对应原图周围4像素下标
				int fx = (int)frontx, fy = (int)fronty;
				int rx = fx + 1, ry = fy + 1;
				if (ry > height - 1)//修正，防越界
					ry -= 1;
				if (rx > width - 1)
					rx -= 1;
				//对应原图周围4像素
				lt = (byte*)&((DWORD*)data[fy])[fx];
				rt = (byte*)&((DWORD*)data[fy])[rx];
				lb = (byte*)&((DWORD*)data[ry])[fx];
				rb = (byte*)&((DWORD*)data[ry])[rx];
				//2次插值
				float rearxbias = 1 - frontxbias;
				float rearybias = 1 - frontybias;

				float fxb, rxb, fxb2, rxb2, fyb, ryb, norm1, norm2, norm3;
				fxb = frontxbias * rt[3];
				rxb = rearxbias * lt[3];
				norm1 = fxb + rxb;
				if (norm1 == 0.0f)
				{
					fxb = rxb = 0.5;
				}
				else
				{
					fxb /= norm1;
					rxb /= norm1;
				}

				fxb2 = frontxbias * rb[3];
				rxb2 = rearxbias * lb[3];
				norm2 = fxb2 + rxb2;
				if (norm2 == 0.0f)
				{
					fxb2 = rxb2 = 0.5f;
				}
				else
				{
					fxb2 /= norm2;
					rxb2 /= norm2;
				}

				fyb = frontybias * norm2;
				ryb = rearybias *norm1;
				norm3 = fyb + ryb;
				if (norm3 == 0.0f)
				{
					fyb = ryb = 0.5f;
				}
				else
				{
					fyb /= norm3;
					ryb /= norm3;
				}
				//有色差法，rgb值仅根据位置比例分配
				/*pixel[0] = (byte)(((lt[0] * rearxbias + rt[0] * frontxbias) * rearybias
					+ (lb[0] * rearxbias + rb[0] * frontxbias) * frontybias));
				pixel[1] = (byte)(((lt[1] * rearxbias + rt[1] * frontxbias) * rearybias
					+ (lb[1] * rearxbias + rb[1] * frontxbias) * frontybias));
				pixel[2] = (byte)(((lt[2] * rearxbias + rt[2] * frontxbias) * rearybias
					+ (lb[2] * rearxbias + rb[2] * frontxbias) * frontybias));*/

				//减小色差法，rgb值根据位置比例、alpha比例分配
				pixel[0] = (byte)((lt[0] * rxb + rt[0] * fxb) * ryb
					+ (lb[0] * rxb2 + rb[0] * fxb2) * fyb);
				pixel[1] = (byte)((lt[1] * rxb + rt[1] * fxb) * ryb
					+ (lb[1] * rxb2 + rb[1] * fxb2) * fyb);
				pixel[2] = (byte)((lt[2] * rxb + rt[2] * fxb) * ryb
					+ (lb[2] * rxb2 + rb[2] * fxb2) * fyb);
				//加速尝试，结果不明显
				//float auxrxry = rxb*ryb, auxfxry = fxb*ryb, auxrxfy = rxb2*fyb, auxfxfy = fxb2*fyb;
				/*pixel[0] = (byte)(lt[0] * auxrxry + rt[0] * auxfxry
					+ lb[0] * auxrxfy + rb[0] * auxfxfy);
				pixel[1] = (byte)(lt[1] * auxrxry + rt[1] * auxfxry
					+ lb[1] * auxrxfy + rb[1] * auxfxfy);
				pixel[2] = (byte)(lt[2] * auxrxry + rt[2] * auxfxry
					+ lb[2] * auxrxfy + rb[2] * auxfxfy);*/
				//加速尝试，失败
				/*byte alphax1 = (byte)(rxb * 19 + 0.5f), alphax2 = (byte)(rxb2 * 19 + 0.5f);
				byte alphay = (byte)(ryb * 19 + 0.5f);
				pixel[0] = (byte)(ALPHABLEND::alphablendcube1[alphay][ALPHABLEND::alphablendcube1[alphax1][lt[0]][rt[0]]]
					[ALPHABLEND::alphablendcube1[alphax2][lb[0]][rb[0]]]);
				pixel[1] = (byte)(ALPHABLEND::alphablendcube1[alphay][ALPHABLEND::alphablendcube1[alphax1][lt[1]][rt[1]]]
					[ALPHABLEND::alphablendcube1[alphax2][lb[1]][rb[1]]]);
				pixel[2] = (byte)(ALPHABLEND::alphablendcube1[alphay][ALPHABLEND::alphablendcube1[alphax1][lt[2]][rt[2]]]
					[ALPHABLEND::alphablendcube1[alphax2][lb[2]][rb[2]]]);*/

				//alpha只根据位置比例分配
				pixel[3] = (byte)(((lt[3] * rearxbias + rt[3] * frontxbias) * rearybias
					+ (lb[3] * rearxbias + rb[3] * frontxbias) * frontybias));
			}
			//bmppixel = *((DWORD*)&pixel);//可能效率降低

			//copypixel方法
			//surfData[index] = bmppixel;

			//条件alphablend方法
			if (pixel[3] < (byte)255)
			{
				if (!((((backbase.y + i) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01)
					^ (((backbase.x + j) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01)))
				{//两种背景色
					if (*((DWORD*)&pixel) == ALPHABLEND::lastcolor1)//缓存加速
					{
						surfData[index] = ALPHABLEND::lastresult1;
					}
					else
					{
						byte *dest = (byte*)&(surfData[index]);

						dest[2] = ALPHABLEND::alphablendtable1[pixel[2]][pixel[3]];
						dest[1] = ALPHABLEND::alphablendtable1[pixel[1]][pixel[3]];
						dest[0] = ALPHABLEND::alphablendtable1[pixel[0]][pixel[3]];

						ALPHABLEND::lastcolor1 = *((DWORD*)&pixel);
						ALPHABLEND::lastresult1 = surfData[index];
					}

					//surfData[index] = ALPHABLEND::AlphaBlend1(bmppixel);
				}
				else
				{
					if (*((DWORD*)&pixel) == ALPHABLEND::lastcolor2)//缓存加速
					{
						surfData[index] = ALPHABLEND::lastresult2;
					}
					else
					{
						byte *dest = (byte*)&(surfData[index]);

						dest[2] = ALPHABLEND::alphablendtable2[pixel[2]][pixel[3]];
						dest[1] = ALPHABLEND::alphablendtable2[pixel[1]][pixel[3]];
						dest[0] = ALPHABLEND::alphablendtable2[pixel[0]][pixel[3]];

						ALPHABLEND::lastcolor2 = *((DWORD*)&pixel);
						ALPHABLEND::lastresult2 = surfData[index];
					}

					//surfData[index] = ALPHABLEND::AlphaBlend2(bmppixel);
				}
			}
			else
				surfData[index] = *((DWORD*)&pixel);
		}
	}
	delete[] xsto;
	delete[] ysto;

	return true;
}


inline bool BMP::Copy_NN(DWORD * surfData, int pitch, const POINT & surfsize, float zoom, float rotate, const POINT surfbias, const POINT backbase)
{
	int index;

	for (int i = 0; i < surfsize.y; i++)
	{
		for (int j = 0; j < surfsize.x; j++)
		{
			index = i*pitch + j;

			int bx = (int)(min((j + surfbias.x + 0.5f) / zoom, width - 1))
				, by = (int)(min((i + surfbias.y + 0.5f) / zoom, height - 1));
			DWORD bmppixel = ((DWORD*)data[by])[bx];//读取bmp信息时加上surface偏移

			//copypixel方法
			//surfData[index] = bmppixel;

			//条件alphablend方法
			if ((byte)(bmppixel >> 24) < 255)
			{
				if (!((((backbase.y + i) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01)
					^ (((backbase.x + j) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01)))
				{//两种背景色
					if (bmppixel == ALPHABLEND::lastcolor1)//缓存加速
					{
						surfData[index] = ALPHABLEND::lastresult1;
					}
					else
					{
						byte *dest = (byte*)&(surfData[index]);
						byte *src = (byte*)&bmppixel;
						//byte alpha = src[3];

						dest[2] = (DWORD)ALPHABLEND::alphablendtable1[src[2]][src[3]];
						dest[1] = (DWORD)ALPHABLEND::alphablendtable1[src[1]][src[3]];
						dest[0] = (DWORD)ALPHABLEND::alphablendtable1[src[0]][src[3]];

						ALPHABLEND::lastcolor1 = bmppixel;
						ALPHABLEND::lastresult1 = *(DWORD*)dest;
					}

					//surfData[index] = ALPHABLEND::AlphaBlend1(bmppixel);
				}
				else
				{
					if (bmppixel == ALPHABLEND::lastcolor2)//缓存加速
					{
						surfData[index] = ALPHABLEND::lastresult2;
					}
					else
					{
						byte *dest = (byte*)&(surfData[index]);
						byte *src = (byte*)&bmppixel;
						//byte alpha = src[3];

						dest[2] = (DWORD)ALPHABLEND::alphablendtable2[src[2]][src[3]];
						dest[1] = (DWORD)ALPHABLEND::alphablendtable2[src[1]][src[3]];
						dest[0] = (DWORD)ALPHABLEND::alphablendtable2[src[0]][src[3]];

						ALPHABLEND::lastcolor2 = bmppixel;
						ALPHABLEND::lastresult2 = *(DWORD*)dest;
					}

					//surfData[index] = ALPHABLEND::AlphaBlend2(bmppixel);
				}
			}
			else
				surfData[index] = bmppixel;
		}
	}

	return true;
}

inline bool BMP::Copy_BiLinear(DWORD * surfData, int pitch, const POINT & surfsize, float zoom, float rotate, const POINT surfbias, const POINT backbase)
{
	int index;

	for (int i = 0; i < surfsize.y; i++)
	{
		for (int j = 0; j < surfsize.x; j++)
		{
			index = i*pitch + j;
			
			float frontx = (j + surfbias.x + 0.5f) / zoom - 0.5f
				, fronty = (i + surfbias.y + 0.5f) / zoom - 0.5f;
			float frontxbias = frontx - (int)frontx;
			/*if (frontx < 0)
				frontxbias = 0;*/
			float rearxbias = 1 - frontxbias;
			float frontybias = fronty - (int)fronty;
			/*if (fronty < 0)
				frontybias = 0;*/
			float rearybias = 1 - frontybias;

			byte *lt, *rt, *lb, *rb;
			//对应原图周围4像素下标
			int fx = (int)frontx, fy = (int)fronty;
			int rx = fx + 1, ry = fy + 1;
			if (ry > height - 1)//修正，防越界
				ry -= 1;
			if (rx > width - 1)
				rx -= 1;
			//对应原图周围4像素
			lt = (byte*)&((DWORD*)data[fy])[fx];
			rt = (byte*)&((DWORD*)data[fy])[rx];
			lb = (byte*)&((DWORD*)data[ry])[fx];
			rb = (byte*)&((DWORD*)data[ry])[rx];
			//2次插值
			byte pixel[4];
			pixel[0] = (byte)(((lt[0] * rearxbias + rt[0] * frontxbias) * rearybias
				+ (lb[0] * rearxbias + rb[0] * frontxbias) * frontybias));
			pixel[1] = (byte)(((lt[1] * rearxbias + rt[1] * frontxbias) * rearybias
				+ (lb[1] * rearxbias + rb[1] * frontxbias) * frontybias));
			pixel[2] = (byte)(((lt[2] * rearxbias + rt[2] * frontxbias) * rearybias
				+ (lb[2] * rearxbias + rb[2] * frontxbias) * frontybias));
			pixel[3] = (byte)(((lt[3] * rearxbias + rt[3] * frontxbias) * rearybias
				+ (lb[3] * rearxbias + rb[3] * frontxbias) * frontybias));

			//copypixel方法
			//surfData[index] = bmppixel;

			//条件alphablend方法
			if (pixel[3] < 255)
			{
				//DWORD bmppixel = *((DWORD*)&pixel);

				if (!((((backbase.y + i) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01)
					^ (((backbase.x + j) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01)))
				{//两种背景色
					if (*((DWORD*)&pixel) == ALPHABLEND::lastcolor1)//缓存加速
					{
						surfData[index] = ALPHABLEND::lastresult1;
					}
					else
					{
						byte *dest = (byte*)&(surfData[index]);
						//byte *src = pixel;

						dest[2] = ALPHABLEND::alphablendtable1[pixel[2]][pixel[3]];
						dest[1] = ALPHABLEND::alphablendtable1[pixel[1]][pixel[3]];
						dest[0] = ALPHABLEND::alphablendtable1[pixel[0]][pixel[3]];

						ALPHABLEND::lastcolor1 = *((DWORD*)&pixel);
						ALPHABLEND::lastresult1 = surfData[index];
					}

					//surfData[index] = ALPHABLEND::AlphaBlend1(bmppixel);
				}
				else
				{
					if (*((DWORD*)&pixel) == ALPHABLEND::lastcolor2)//缓存加速
					{
						surfData[index] = ALPHABLEND::lastresult2;
					}
					else
					{
						byte *dest = (byte*)&(surfData[index]);
						//byte *src = pixel;

						dest[2] = ALPHABLEND::alphablendtable2[pixel[2]][pixel[3]];
						dest[1] = ALPHABLEND::alphablendtable2[pixel[1]][pixel[3]];
						dest[0] = ALPHABLEND::alphablendtable2[pixel[0]][pixel[3]];

						ALPHABLEND::lastcolor2 = *((DWORD*)&pixel);
						ALPHABLEND::lastresult2 = surfData[index];
					}

					//surfData[index] = ALPHABLEND::AlphaBlend2(bmppixel);
				}
			}
			else
				surfData[index] = *((DWORD*)&pixel);
		}
	}

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


UINT16 roundf2ui16(float f)
{
	if (f > 0)
		return (UINT16)(f + 0.5f);
	else
		return (UINT16)(f - 0.5f);
}

int roundpf2i(float f)
{
	return (int)(f + 0.5f);
}

DWORD roundf2d(float f)
{
	if (f > 0)
		return (DWORD)(f + 0.5f);
	else
		return (DWORD)(f - 0.5f);
}

LONG roundf2l(float f)
{
	if (f > 0)
		return (LONG)(f + 0.5f);
	else
		return (LONG)(f - 0.5f);
}

float roundbf2f(float f)
{
	if (f > 0)
		return (float)(byte)(f + 0.5f);
	else
		return (float)(byte)(f - 0.5);
}

byte roundf2b(float f)
{
	if (f > 0)
		return (byte)(f + 0.5f);
	else
		return (byte)(f - 0.5f);
}

byte roundpf2b(float f)
{
	return (byte)(f + 0.5f);
}

char roundpf2c(float f)
{
	return (char)(f + 0.5f);
}

int ceilf2i(float f)
{
	if ((int)f != f)
		return (int)(f + 1);
	else
		return (int)f;
}
