#pragma once 

#include <new>
#include <iostream>
#include <stdlib.h>
#include <winnt.h>
#include <strsafe.h>

#include "picfile.h"
#include "generalgeo.h"

#include <d3d9.h>
#include <d3dx9tex.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

// 透明背景效果
//#define TRANSPARENTBACK_SQUAREWIDTH		8
#define TRANSPARENTBACK_MASK			0b1000
#define TRANSPARENTBACK_HOLLOWCOLOR		D3DCOLOR_XRGB(255, 255, 255)
#define TRANSPARENTBACK_FILLCOLOR		D3DCOLOR_XRGB(204, 203, 204)
#define TRANSPARENTBACK_HOLLOWDENSITY	255
#define TRANSPARENTBACK_FILLDENSITY		230

#define RGB2GRAY(R, G, B)				(((R)*38 + (G)*75 + (B)*15) >> 7)

// 内存
#define SAFE_RELEASE(p)					{ if (p) { (p)->Release(); (p) = NULL; } }
#define SAFE_DELETE(p)					{ if (p) { delete (p); (p) = NULL;} }
#define SAFE_DELETE_LIST(p)				{ if (p) { delete[] (p); (p) = NULL;} }
#define DELETE_LIST(p)					{ delete[] (p); (p) = NULL; }
#define FAILED_RETURN(hr)				{ if (FAILED(hr)) return hr; }
#define FAILED_RETURN_FALSE(hr)			{ if (FAILED(hr)) return false; }
#define FALSE_RETURN(r)					{ if (!(r)) return false;}

#define MALLOC(type, size)				(type *)malloc(size*sizeof(type))
#define REALLOC(P, type, size)			P = (type *)realloc(P, size*sizeof(type));

#define _CDECL							__cdecl
#define _STDCALL						__stdcall
#define _FASTCALL						__fastcall
#define MYCALL1							_CDECL

// 图片缩放
#define ZOOM_MAX						FLT_MAX
#define ZOOM_MIN						0.0f
#define ZOOM_MINDELTA					0.0001f

#define ZOOMFACTOR_DRAG					0.015f //缩放灵敏度
#define ZOOMFACTOR_WHEEL				0.025f
#define ZOOMFACTOR_ZOOMOUTSHRINK		0.92f

#define ZOOM_MAXCONTROLED				0.5f

// 采样策略
#define SAMPLE_SCHEMA_UNKNOWN			0
#define SAMPLE_SCHEMA_NN				1
#define SAMPLE_SCHEMA_BILINEAR			2
#define SAMPLE_SCHEMA_CUBE				3
#define SAMPLE_SCHEMA_SINGLE			4
#define SAMPLE_SCHEMA_MAX				5

#define NN_MIN							3.0f	//大于这个放大倍率使用NN采样
#define MAXSAMPLE_WIDTH_UPPERLIMIT		20000	//大于这个尺寸图片不进行最大采样MAXSAMPLE
#define MAXSAMPLE_HEIGHT_UPPERLIMIT		MAXSAMPLE_WIDTH_UPPERLIMIT

#define MIN_NORMALRENEW_WIDTH			300		//图片小于这个尺寸，在加速时才允许使用SingleSample(太大缩放过程可能有卡顿)
#define MIN_NORMALRENEW_HEIGHT			MIN_NORMALRENEW_WIDTH

#define MIN_FORCECLIP_WIDTH				1500	//图片大于这个尺寸，则设置bClip。增加缩放速度
#define MIN_FORCECLIP_HEIGHT			MIN_FORCECLIP_WIDTH

#define MAX_NOCLIP_WIDTH_DRAG			3000	//图片小于这个尺寸，拖动图片时取消bClip。增加拖动速度
#define MAX_NOCLIP_HEIGHT_DRAG			MAX_NOCLIP_WIDTH_DRAG

// 图片列表
#define PICPACKLIST_SIZE_INIT			3
#define PICPACKLIST_SIZE_INCREMENT		2

// 鼠标位置
#define CURSORPOS_PIC					0
#define CURSORPOS_BLANK					1
#define CURSORPOS_OUTWINDOW				2

// 颜色
#define BACKCOLOR_INIT					0xFF888888
#ifndef SETALPHA
#define SETALPHA(C, A)					((C & 0xFFFFFF) | (A << 24))
#endif

using std::string;
using std::bad_alloc;
using std::endl;
using std::cerr;

typedef float	COLORTYPE;

inline int ceilf2i(float f)
{
	if ((int)f != f)
		return (int)(f + 1);
	else
		return (int)f;
}

struct COLOR_F3 {
	COLORTYPE a;

	COLORTYPE r;
	COLORTYPE g;
	COLORTYPE b;

	COLOR_F3();
	COLOR_F3(COLORTYPE f);

	inline COLOR_F3& operator=(COLOR_F3& cf) {
		a = cf.a;

		r = cf.r;
		g = cf.g;
		b = cf.b;
	}
	inline COLOR_F3& operator=(const COLORTYPE &f) {
		a = r = g = b = f;
		return *this;
	}
	inline void Add(DWORD dw, const COLORTYPE &mul = 1.0, const COLORTYPE &div = 1.0) {
		a += ((dw >> 24) & 0xFF) *mul / div;
		r += (((dw >> 16) & 0xFF)) *mul / div;
		g += ((dw >> 8) & 0xFF) *mul / div;
		b += (dw & 0xFF) *mul / div;
	}
	void Normalize();

	inline DWORD GetDW()
	{
		DWORD res = 0;
		res += (byte)(a + 0.5f) << 24;
		res += (byte)(r + 0.5f) << 16;
		res += (byte)(g + 0.5f) << 8;
		res += (byte)(b + 0.5f);

		return res;
	}
	//inline DWORD AlphaBlend(DWORD blender) {
	//	byte alpha = (byte)a;
	//	DWORD result = 0;
	//	//result += (byte)round(a) << 24;
	//	result += (DWORD)((r*alpha + ((blender >> 16) & 0xFF)*(255 - alpha)) / 255) << 16;
	//	result += (DWORD)((g*alpha + ((blender >> 8) & 0xFF)*(255 - alpha)) / 255) << 8;
	//	result += (DWORD)((b*alpha + (blender & 0xFF)*(255 - alpha)) / 255);
	//	return result;
	//}
};

class ALPHABLEND {
public:
	//对单一（或2种）destcolor进行任意srccolor和alpha的alphablend的查询表
	static DWORD lastcolor1, lastresult1;
	static DWORD lastcolor2, lastresult2;
	static byte **alphablendtable1, **alphablendtable2;

	//对任意destcolor、任意srccolor进行alphablend的查询表
	static byte ***alphablendcube1;

	ALPHABLEND() {}

	//内联示例
	/*static inline DWORD AlphaBlend1(COLOR_F3 src) {
		byte alpha = roundf2b(src.a);
		DWORD result = 0;
		result += (DWORD)alphablendtable1[(byte)(src.r)][alpha] << 16;
		result += (DWORD)alphablendtable1[(byte)(src.g)][alpha] << 8;
		result += (DWORD)alphablendtable1[(byte)(src.b)][alpha];

		return result;
	}*/
	//内联示例
	/*static inline DWORD AlphaBlend1(DWORD srccolor) {
		if (srccolor == ALPHABLEND::lastcolor1)
		{
			return ALPHABLEND::lastresult1;
		}
		else
		{
			byte *src = (byte*)&srccolor;
			byte *dest = (byte*)&lastresult1;

			dest[2] = alphablendtable1[src[2]][src[3]];
			dest[1] = alphablendtable1[src[1]][src[3]];
			dest[0] = alphablendtable1[src[0]][src[3]];

			ALPHABLEND::lastcolor1 = srccolor;
			ALPHABLEND::lastresult1 = *(DWORD*)dest;

			return lastresult1;
		}
	}*/
	static void InitAlphBlendTable(byte color1, byte color2);

	static void InitAlphBlendTCube();
};

inline COLOR_F3 RGB2HSV_F3(DWORD c1)
{
	byte *ppix = (byte*)&c1;
	COLOR_F3 res = 0;
	res.a = ppix[3];

	byte r = ppix[2];
	byte g = ppix[1];
	byte b = ppix[0];
	byte max = 0;
	byte min = 1;
	if (ppix[1] > ppix[0])
	{
		max = 1;
		min = 0;
	}
	if (ppix[2] > ppix[max])
		max = 2;
	else if (ppix[2] < ppix[min])
		min = 2;
	byte minvalue = ppix[min];
	byte maxvalue = ppix[max];

	res.b = maxvalue;//v
	res.g = (maxvalue == 0 ? 0 : 1 - minvalue / (float)maxvalue);//s

	if (maxvalue == minvalue)
		res.r = 0;
	else
	{
		res.r = (maxvalue - r + g - minvalue + b - minvalue) / (float)(maxvalue - minvalue)*60.0f;
		/*if (max == 2)
		{
			result.r = (60.0f * (g - b) / (maxvalue - minvalue));
		}
		else if (max == 1)
		{
			result.r = (120.0f + 60.0f * (b - r) / (maxvalue - minvalue));
		}
		else
		{
			result.r = (240.0f + 60.0f * (r - g) / (maxvalue - minvalue));
		}*/
	}
	if (g < b)
		res.r = 360 - res.r;
	
	return res;
}

inline DWORD RGB2HSV(DWORD color)
{
	byte *pSrc = (byte*)&color;
	DWORD result = 0;
	BYTE* pResult = (byte*)&result;

	byte r = pSrc[2];
	byte g = pSrc[1];
	byte b = pSrc[0];
	byte max = 0;
	byte min = 1;
	if (pSrc[1] > pSrc[0])
	{
		max = 1;
		min = 0;
	}
	if (pSrc[2] > pSrc[max])
		max = 2;
	else if (pSrc[2] < pSrc[min])
		min = 2;
	byte minvalue = pSrc[min];
	byte maxvalue = pSrc[max];

	pResult[0] = maxvalue;//v
	pResult[1] = (maxvalue == 0 ? 0 : ROUNDF_BYTE(255.0f*(maxvalue - minvalue) / maxvalue));//s

	if (maxvalue == minvalue)
		pResult[2] = 0;
	else
	{
		if (max == 2)
		{
			pResult[2] = ROUNDF_BYTE(255.0f * (000.0f + 60.0f * (g - b) / (maxvalue - minvalue)) / 360.0f);
		}
		else if (max == 1)
		{
			pResult[2] = ROUNDF_BYTE(255.0f * (120.0f + 60.0f * (b - r) / (maxvalue - minvalue)) / 360.0f);
		}
		else
		{
			pResult[2] = ROUNDF_BYTE(255.0f * (240.0f + 60.0f * (r - g) / (maxvalue - minvalue)) / 360.0f);
		}
	}
	pResult[3] = pSrc[3];

	return result;
}

inline DWORD HSV2RGB_F3(COLOR_F3 color)
{
	DWORD result = 0;
	byte* pResult = (byte*)&result;
	
	float maxvalue = color.b;
	if (color.g == 0)
	{
		pResult[0] = ROUNDF_BYTE(maxvalue);
		pResult[1] = ROUNDF_BYTE(maxvalue);
		pResult[2] = ROUNDF_BYTE(maxvalue);
	}
	else
	{
		float ρ = color.g * color.b;
		float minvalue = maxvalue - ρ;
		byte nh = ROUNDF_BYTE(color.r);
		if (color.r > 180)
			nh = 360 - nh;
		float λ = nh / 60.0f * ρ;
		float r, g, b;
		r = 2 * ρ - λ;
		if (r < 0)
			r = 0;
		if (r > ρ)
			r = ρ;
		g = λ;
		if (g > ρ)
			g = ρ;

		b = λ - 2 * ρ;
		if (b < 0)
			b = 0;

		if (color.r > 180)
		{
			float temp = b;
			b = g;
			g = temp;
		}

		pResult[0] = ROUNDF_BYTE(b + minvalue);
		pResult[1] = ROUNDF_BYTE(g + minvalue);
		pResult[2] = ROUNDF_BYTE(r + minvalue);

	}
	pResult[3] = ROUNDF_BYTE(color.a);

	return result;
}

inline DWORD HSV2RGB(DWORD color)
{
	byte *pSrc = (byte*)&color;
	DWORD result = 0;
	byte* pResult = (byte*)&result;

	byte h = pSrc[2];
	byte s = pSrc[1];
	byte v = pSrc[0];

	byte maxvalue = v;
	if (s == 0)
	{
		pResult[0] = maxvalue;
		pResult[1] = maxvalue;
		pResult[2] = maxvalue;
	}
	else
	{
		float ρ = s * v / 255.0f;
		float minvalue = maxvalue - ρ;
		byte nh = h;
		if (h >= 128)
			nh = 255 - h;
		float λ = nh * 6 / 255.0f * ρ;
		float r, g, b;
		r = 2 * ρ - λ;
		if (r < 0)
			r = 0;
		if (r > ρ)
			r = ρ;
		g = λ;
		if (g > ρ)
			g = ρ;

		b = λ - 2 * ρ;
		if (b < 0)
			b = 0;

		if (h >= 128)
		{
			float temp = b;
			b = g;
			g = temp;
		}

		pResult[0] = ROUNDF_BYTE(b + minvalue);
		pResult[1] = ROUNDF_BYTE(g + minvalue);
		pResult[2] = ROUNDF_BYTE(r + minvalue);

	}
	pResult[3] = pSrc[3];

	return result;
}

class BMP {
private:
	/*static void __cdecl newhandlerfunc()
	{
		cerr << "Allocate failed in class BMP" << endl;
		abort();
	}*/
public:
	int width, height;
	byte **data;
	bool bAlpha;		//是否有透明通道

	BMP();
	~BMP();

	// 操作
	void SetAlpha(bool alpha);
	bool Load(LPDIRECT3DSURFACE9 surf);			//从surface生成对应大小图片
	bool UpLoad(LPDIRECT3DSURFACE9 surf);		//拷贝到surface，从{0,0}对齐开始，尽可能多的拷贝

	/*
	 * 对于已定制的surface的拷贝
	 */
	/***************************************************
	目标地址空间，每行pitch，目标地址尺寸
	surfBias:surface相对原图偏移（用于原图选区）
	destBase:surface相对client窗口（用于辅助透明背景绘制）

	双线性插值的两种策略（有色差，无色差）在Sample_Single中详细定义
	****************************************************/
	// 最大无损失采样（缩小效果较好，缩小速度最慢）
	inline bool MYCALL1 Sample_Area(DWORD *surfData, int pitch, const POINT &surfSize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 }) 
	{
		UINT32 zoomW = (UINT32)(width*zoom);
		UINT32 zoomH = (UINT32)(height*zoom);

		COLORTYPE xpixspan = (COLORTYPE)width / zoomW;//新像素x方向跨度
		COLORTYPE ypixspan = (COLORTYPE)height / zoomH;//新像素y方向跨度
		COLORTYPE spanarea = xpixspan*ypixspan;//新像素面积 x*y
		float baseweight = 1.0f / spanarea;
		COLORTYPE *xbase = new COLORTYPE[surfSize.x + 1];//新像素左上起始点数组，x方向
		COLORTYPE *ybase = new COLORTYPE[surfSize.y + 1];//新像素左上起始点数组，y方向
		bool *xincolor1 = new bool[surfSize.x + 1];

		// 设置新像素起点值数组,限定需要的区间
		float actualzx = (float)width / zoomW;
		//float actualzy = (float)height / zoomH;
		for (int i = 0; i < surfSize.x + 1; i++)
		{
			xbase[i] = (COLORTYPE)(i + surfbias.x)*actualzx;
			xincolor1[i] = ((backbase.x + i) & TRANSPARENTBACK_MASK) > 0;
		}
		for (int i = 0; i < surfSize.y + 1; i++)
			ybase[i] = (COLORTYPE)(i + surfbias.y)*height / zoomH;//height / zoomH

		int index;
		COLOR_F3 rcolor = 0;//颜色累加器
		for (int i = 0; i < surfSize.y; i++)
		{
			int starty = (int)ybase[i];//新像素左上起始点，y方向
			int endy = ceilf2i(ybase[i + 1]);//新像素左上结束点，y方向
			bool yincolor1 = ((backbase.y + i) & TRANSPARENTBACK_MASK) > 0;

			for (int j = 0; j < surfSize.x; j++)
			{
				index = i*pitch + j;

				int startx = (int)xbase[j];//新像素左上起始点，x方向
				int endx = ceilf2i(xbase[j + 1]);//新像素左上结束点，x方向

				rcolor = 0;
				COLORTYPE floatsy, floatey;//精确起点终点

				bool isxbase, isybase;
				// 范围内像素颜色累加
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

				// 条件alphablend方法
				if (rcolor.a < 255.0f)
				{
					if (yincolor1 ^ xincolor1[j])
					{// 两种背景色
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
					}
					else
					{
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
	// NN和bilinear结合，跨交界处像素使用bilinear（优化的马赛克版本放大，缩小近似于bilinear）
	inline bool MYCALL1 Sample_Single(DWORD *surfData, int pitch, const POINT &surfSize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 }) 
	{
		int index;

		const float auxz = 0.3f / zoom;//加速
		float auxfront = 0.5f - auxz, auxrear = 0.5f + auxz;//加速

		float *xsto = new float[surfSize.x];//加速,防止处于二层循环的x重复计算
		bool *xincolor1 = new bool[surfSize.x];
		for (int j = 0; j < surfSize.x; j++)//提前运算x方向的变量
		{
			xsto[j] = (j + surfbias.x + 0.5f) / zoom - 0.5f;
			xincolor1[j] = ((backbase.x + j) & TRANSPARENTBACK_MASK) > 0;
		}
		if (bAlpha)// 有alpha通道
		{
			for (int i = 0; i < surfSize.y; i++)
			{
				// y方向计算
				float fronty = (i + surfbias.y + 0.5f) / zoom - 0.5f;
				float frontybias = fronty - (int)fronty;
				if (frontybias < auxfront)
					frontybias = 0;
				else if (frontybias > auxrear)
					frontybias = 1;
				else
					frontybias = (frontybias - auxfront) * zoom;
				float rearybias = 1 - frontybias;

				int fy, ry;
				fy = (int)fronty;
				ry = fy + 1;
				
				if (ry > height - 1)
					ry -= 1;
				/*if (fronty < 0)
				frontybias = 0;*/
				DWORD *datafy = (DWORD*)data[fy];
				DWORD *datary = (DWORD*)data[ry];

				bool yincolor1 = ((backbase.y + i) & TRANSPARENTBACK_MASK) > 0;

				for (int j = 0; j < surfSize.x; j++)
				{
					index = i*pitch + j;

					float frontx = xsto[j];
					float frontxbias = frontx - (int)frontx;
					/*if (frontx < 0)//不加不会报错
					frontxbias = 0;*/
					if (frontxbias < auxfront)
						frontxbias = 0;
					else if (frontxbias > auxrear)
						frontxbias = 1;
					else
						frontxbias = (frontxbias - auxfront) * zoom;

					byte pixel[4];
					if ((frontxbias == 0 || frontxbias == 1)
						&& (frontybias == 0 || frontybias == 1))
					{
						//memcpy_s(pixel, 4
						//	, ((byte*)&((DWORD*)data[(int)(fronty + frontybias)])[(int)(frontx + frontxbias)]), 4);
						*(DWORD*)pixel = ((DWORD*)data[(int)(fronty + frontybias)])[(int)(frontx + frontxbias)];
					}
					else
					{
						// 对应原图周围4像素x下标
						int fx, rx;
						fx = (int)xsto[j];
						rx = fx + 1;
						if (rx > width - 1)//修正，防越界
							rx -= 1;
						// 对应原图周围4像素
						byte *lt, *rt, *lb, *rb;
						lt = (byte*)&datafy[fx];
						rt = (byte*)&datafy[rx];
						lb = (byte*)&datary[fx];
						rb = (byte*)&datary[rx];

						// 2次插值
						float rearxbias = 1 - frontxbias;

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
						ryb = rearybias * norm1;
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
						// --#3--加速尝试，略快
						float auxrxry = rxb*ryb, auxfxry = fxb*ryb
							, auxrxfy = rxb2*fyb, auxfxfy = fxb2*fyb;
						pixel[0] = (byte)(lt[0] * auxrxry + rt[0] * auxfxry
							+ lb[0] * auxrxfy + rb[0] * auxfxfy);
						pixel[1] = (byte)(lt[1] * auxrxry + rt[1] * auxfxry
							+ lb[1] * auxrxfy + rb[1] * auxfxfy);
						pixel[2] = (byte)(lt[2] * auxrxry + rt[2] * auxfxry
							+ lb[2] * auxrxfy + rb[2] * auxfxfy);

						// alpha只根据位置比例分配
						pixel[3] = (byte)(((lt[3] * rearxbias + rt[3] * frontxbias) * rearybias
							+ (lb[3] * rearxbias + rb[3] * frontxbias) * frontybias));
					}
					//bmppixel = *((DWORD*)&pixel);//可能效率降低

					// 条件alphablend方法
					if (pixel[3] < (byte)255)
					{
						if (yincolor1 ^ xincolor1[j])
						{// 两种背景色
							if (*((DWORD*)&pixel) == ALPHABLEND::lastcolor2)//缓存加速
							{
								surfData[index] = ALPHABLEND::lastresult2;
							}
							else
							{
								byte *dest = ((byte*)&surfData[index]);

								dest[2] = ALPHABLEND::alphablendtable2[pixel[2]][pixel[3]];
								dest[1] = ALPHABLEND::alphablendtable2[pixel[1]][pixel[3]];
								dest[0] = ALPHABLEND::alphablendtable2[pixel[0]][pixel[3]];

								ALPHABLEND::lastcolor2 = *((DWORD*)&pixel);
								ALPHABLEND::lastresult2 = surfData[index];
							}
						}
						else
						{
							if (*((DWORD*)&pixel) == ALPHABLEND::lastcolor1)//缓存加速
							{
								surfData[index] = ALPHABLEND::lastresult1;
							}
							else
							{
								byte *dest = ((byte*)&surfData[index]);

								dest[2] = ALPHABLEND::alphablendtable1[pixel[2]][pixel[3]];
								dest[1] = ALPHABLEND::alphablendtable1[pixel[1]][pixel[3]];
								dest[0] = ALPHABLEND::alphablendtable1[pixel[0]][pixel[3]];

								ALPHABLEND::lastcolor1 = *((DWORD*)&pixel);
								ALPHABLEND::lastresult1 = surfData[index];
							}
						}
					}
					else
						surfData[index] = *((DWORD*)&pixel);
				}
			}
		}
		else//无alpha通道
		{
			for (int i = 0; i < surfSize.y; i++)
			{
				// y方向运算
				float fronty = (i + surfbias.y + 0.5f) / zoom - 0.5f;
				float frontybias = fronty - (int)fronty;
				/*if (fronty < 0)
				frontybias = 0;*/
				if (frontybias < auxfront)
					frontybias = 0;
				else if (frontybias > auxrear)
					frontybias = 1;
				else
					frontybias = (frontybias - auxfront) * zoom;
				float rearybias = 1 - frontybias;

				// 原图周围4像素下标
				int fy = (int)fronty;
				int ry = fy + 1;
				if (ry > height - 1)
					ry -= 1;
				DWORD* datafy = (DWORD*)data[fy];
				DWORD* datary = (DWORD*)data[ry];

				for (int j = 0; j < surfSize.x; j++)
				{
					float frontx = xsto[j];

					float frontxbias = frontx - (int)frontx;
					/*if (frontx < 0)//不加不会报错
					frontxbias = 0;*/
					if (frontxbias < auxfront)
						frontxbias = 0;
					else if (frontxbias > auxrear)
						frontxbias = 1;
					else
						frontxbias = (frontxbias - auxfront) * zoom;
					float rearxbias = 1 - frontxbias;

					//DWORD bmppixel;
					byte pixel[4];
					if ((frontxbias == 0 || frontxbias == 1)
						&& (frontybias == 0 || frontybias == 1))
					{
						//memcpy_s(pixel, 4
						//	, ((byte*)&((DWORD*)data[(int)(fronty + frontybias)])[(int)(frontx + frontxbias)]), 4);
						*(DWORD*)pixel = ((DWORD*)data[(int)(fronty + frontybias)])[(int)(frontx + frontxbias)];
					}
					else
					{
						byte *lt, *rt, *lb, *rb;
						// 原图周围4像素下标
						int fx = (int)frontx;
						int rx = fx + 1;
						if (rx > width - 1)//修正，防越界
							rx -= 1;
						// 原图周围4像素
						lt = (byte*)&datafy[fx];
						rt = (byte*)&datafy[rx];
						lb = (byte*)&datary[fx];
						rb = (byte*)&datary[rx];

						// 2次插值-不去色差法
						// --#1--不去色差法，rgb值仅根据位置比例分配
						float auxrxry = rearxbias*rearybias, auxfxry = frontxbias*rearybias
							, auxrxfy = rearxbias*frontybias, auxfxfy = frontxbias*frontybias;
						pixel[0] = (byte)(lt[0] * auxrxry + rt[0] * auxfxry
							+ lb[0] * auxrxfy + rb[0] * auxfxfy);
						pixel[1] = (byte)(lt[1] * auxrxry + rt[1] * auxfxry
							+ lb[1] * auxrxfy + rb[1] * auxfxfy);
						pixel[2] = (byte)(lt[2] * auxrxry + rt[2] * auxfxry
							+ lb[2] * auxrxfy + rb[2] * auxfxfy);
						pixel[3] = (byte)(lt[3] * auxrxry + rt[3] * auxfxry
							+ lb[3] * auxrxfy + rb[3] * auxfxfy);
					}

					surfData[i*pitch + j] = *((DWORD*)&pixel);
				}
			}
		}
		delete[] xsto;
		delete[] xincolor1;

		return true;
	}
	// 最近邻近（放大缩小都可以，效果不会太好）
	inline bool MYCALL1 Sample_NN(DWORD *surfData, int pitch, const POINT &surfSize
		, float zoom, float rotate = 0, const POINT surfBias = { 0,0 }, const POINT backbase = { 0,0 }) 
	{
		int index;

		int *bxarray = new int[surfSize.x];
		bool *xincolor1 = new bool[surfSize.x];
		for (int j = 0; j < surfSize.x; j++)//加速
		{
			bxarray[j] = (int)(min((j + surfBias.x + 0.5f) / zoom, width - 1));
			xincolor1[j] = ((backbase.x + j) & TRANSPARENTBACK_MASK) > 0;
		}
		if (bAlpha)
		{
			for (int i = 0; i < surfSize.y; i++)
			{
				int by = (int)(min((i + surfBias.y + 0.5f) / zoom, height - 1));
				DWORD *databy = (DWORD*)data[by];

				bool yincolor1 = ((backbase.y + i) & TRANSPARENTBACK_MASK) > 0;

				for (int j = 0; j < surfSize.x; j++)
				{
					index = i*pitch + j;

					int bx = bxarray[j];
					DWORD bmppixel = databy[bx];//读取pBmp信息时加上surface偏移

												//条件alphablend方法
					if ((byte)(bmppixel >> 24) < 255)
					{
						if (yincolor1 ^ xincolor1[j])
						{//两种背景色
							if (bmppixel == ALPHABLEND::lastcolor2)//缓存加速
							{
								surfData[index] = ALPHABLEND::lastresult2;
							}
							else
							{
								byte *src = (byte*)&bmppixel;
								byte *dest = (byte*)&(surfData[index]);

								dest[2] = (DWORD)ALPHABLEND::alphablendtable2[src[2]][src[3]];
								dest[1] = (DWORD)ALPHABLEND::alphablendtable2[src[1]][src[3]];
								dest[0] = (DWORD)ALPHABLEND::alphablendtable2[src[0]][src[3]];

								ALPHABLEND::lastcolor2 = bmppixel;
								ALPHABLEND::lastresult2 = *(DWORD*)dest;
							}
						}
						else
						{
							if (bmppixel == ALPHABLEND::lastcolor1)//缓存加速
							{
								surfData[index] = ALPHABLEND::lastresult1;
							}
							else
							{
								byte *src = (byte*)&bmppixel;
								byte *dest = (byte*)&(surfData[index]);

								dest[2] = (DWORD)ALPHABLEND::alphablendtable1[src[2]][src[3]];
								dest[1] = (DWORD)ALPHABLEND::alphablendtable1[src[1]][src[3]];
								dest[0] = (DWORD)ALPHABLEND::alphablendtable1[src[0]][src[3]];

								ALPHABLEND::lastcolor1 = bmppixel;
								ALPHABLEND::lastresult1 = *(DWORD*)dest;
							}
						}
					}
					else
						surfData[index] = bmppixel;
				}
			}
		}
		else
		{
			for (int i = 0; i < surfSize.y; i++)
			{
				int by = (int)(min((i + surfBias.y + 0.5f) / zoom, height - 1));
				DWORD *databy = (DWORD*)data[by];
				for (int j = 0; j < surfSize.x; j++)
				{
					int bx = bxarray[j];

					//copypixel方法
					surfData[i*pitch + j] = databy[bx];
				}
			}
		}
		delete[] bxarray;
		delete[] xincolor1;

		return true;
	}
	// 双线性（缩小过多或放大过多效果差）
	inline bool MYCALL1 Sample_BiLinear(DWORD *surfData, int pitch, const POINT &surfSize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 }) 
	{
		int index;

		float *xsto = new float[surfSize.x];//加速,防止处于二层循环的x重复计算
		bool *xincolor1 = new bool[surfSize.x];
		for (int j = 0; j < surfSize.x; j++)//提前运算x方向的变量，终极加速
		{
			xsto[j] = (j + surfbias.x + 0.5f) / zoom - 0.5f;
			xincolor1[j] = ((backbase.x + j) & TRANSPARENTBACK_MASK) > 0;
		}
		if (bAlpha)
		{
			for (int i = 0; i < surfSize.y; i++)
			{
				float fronty = (i + surfbias.y + 0.5f) / zoom - 0.5f;
				float frontybias = fronty - (int)fronty;
				/*if (fronty < 0)
				frontybias = 0;*/
				float rearybias = 1 - frontybias;

				int fy = (int)fronty;
				int ry = fy + 1;
				if (ry > height - 1)//修正，防越界
					ry -= 1;
				DWORD* datafy = (DWORD*)data[fy];
				DWORD* datary = (DWORD*)data[ry];

				bool yincolor1 = ((backbase.y + i) & TRANSPARENTBACK_MASK) > 0;

				for (int j = 0; j < surfSize.x; j++)
				{
					index = i*pitch + j;

					// 对应原图片素位置
					float frontx = xsto[j];
					float frontxbias = frontx - (int)frontx;
					/*if (frontx < 0)
					frontxbias = 0;*/
					float rearxbias = 1 - frontxbias;

					byte *lt, *rt, *lb, *rb;
					// 对应原图周围4像素下标
					int fx = (int)frontx;
					int rx = fx + 1;
					if (rx > width - 1)//修正，防越界
						rx -= 1;
					// 对应原图周围4像素
					lt = (byte*)&datafy[fx];
					rt = (byte*)&datafy[rx];
					lb = (byte*)&datary[fx];
					rb = (byte*)&datary[rx];

					// 2次插值-无色差法
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
					ryb = rearybias * norm1;
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
					// --#3--加速尝试，略快
					byte pixel[4];
					float auxrxry = rxb*ryb, auxfxry = fxb*ryb, auxrxfy = rxb2*fyb, auxfxfy = fxb2*fyb;
					pixel[0] = (byte)(lt[0] * auxrxry + rt[0] * auxfxry
						+ lb[0] * auxrxfy + rb[0] * auxfxfy);
					pixel[1] = (byte)(lt[1] * auxrxry + rt[1] * auxfxry
						+ lb[1] * auxrxfy + rb[1] * auxfxfy);
					pixel[2] = (byte)(lt[2] * auxrxry + rt[2] * auxfxry
						+ lb[2] * auxrxfy + rb[2] * auxfxfy);

					// alpha只根据位置比例分配
					pixel[3] = (byte)(((lt[3] * rearxbias + rt[3] * frontxbias) * rearybias
						+ (lb[3] * rearxbias + rb[3] * frontxbias) * frontybias));

					// 条件alphablend方法
					if (pixel[3] < 255)
					{
						if (yincolor1 ^ xincolor1[j])
						{//两种背景色
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
						}
						else
						{
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
						}
					}
					else
						surfData[index] = *((DWORD*)&pixel);
				}
			}
		}
		else
		{
			for (int i = 0; i < surfSize.y; i++)
			{
				float fronty = (i + surfbias.y + 0.5f) / zoom - 0.5f;
				float frontybias = fronty - (int)fronty;
				/*if (fronty < 0)//不加不报错
				frontybias = 0;*/
				float rearybias = 1 - frontybias;

				int fy = (int)fronty;
				int ry = fy + 1;
				if (ry > height - 1)//修正，防越界
					ry -= 1;
				DWORD* datafy = (DWORD*)data[fy];
				DWORD* datary = (DWORD*)data[ry];

				for (int j = 0; j < surfSize.x; j++)
				{
					// 对应原图片素位置
					float frontx = xsto[j];
					float frontxbias = frontx - (int)frontx;
					/*if (frontx < 0)
					frontxbias = 0;*/
					float rearxbias = 1 - frontxbias;

					byte *lt, *rt, *lb, *rb;
					// 对应原图周围4像素下标
					int fx = (int)frontx;
					int rx = fx + 1;
					if (rx > width - 1)//修正，防越界
						rx -= 1;
					// 对应原图周围4像素
					lt = (byte*)&datafy[fx];
					rt = (byte*)&datafy[rx];
					lb = (byte*)&datary[fx];
					rb = (byte*)&datary[rx];

					// 2次插值-不去色差法  >90%的时间花费在4句这里
					byte pixel[4];
					float auxrxry = rearxbias*rearybias, auxfxry = frontxbias*rearybias
						, auxrxfy = rearxbias*frontybias, auxfxfy = frontxbias*frontybias;
					pixel[0] = (byte)((lt[0] * auxrxry + rt[0] * auxfxry
						+ lb[0] * auxrxfy + rb[0] * auxfxfy));
					pixel[1] = (byte)((lt[1] * auxrxry + rt[1] * auxfxry
						+ lb[1] * auxrxfy + rb[1] * auxfxfy));
					pixel[2] = (byte)((lt[2] * auxrxry + rt[2] * auxfxry
						+ lb[2] * auxrxfy + rb[2] * auxfxfy));
					pixel[3] = (byte)((lt[3] * auxrxry + rt[3] * auxfxry
						+ lb[3] * auxrxfy + rb[3] * auxfxfy));

					// copypixel方法
					surfData[i*pitch + j] = *((DWORD*)&pixel);
				}
			}
		}
		delete[] xsto;
		delete[] xincolor1;

		return true;
	}

	/*
	 * 图片处理
	 */
	inline void TestInc()
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				byte *ppix = (byte*)&((DWORD*)data[i])[j];
				ppix[0]++;
				ppix[1]++;
				ppix[2]++;
			}
		}
	}
	bool MYCALL1 Gray();

	bool MYCALL1 Inverse();
	bool MYCALL1 InverseAlpha();
	bool MYCALL1 InverseAll();

	bool MYCALL1 LOSE_R();
	bool MYCALL1 LOSE_G();
	bool MYCALL1 LOSE_B();

	bool MYCALL1 RGB2HSV();

	// 信息
	inline bool MYCALL1 GetPixel(int x, int y, DWORD *dest)
	{
		if (x < 0 || y < 0 || x >= width || y >= height)
			return false;

		if (!data)
			return false;

		if (!data[y])
			return false;

		*dest = ((DWORD*)data[y])[x];

		return true;
	}

	// 状态
	void MYCALL1 Clear();
	inline bool MYCALL1 isEmpty()
	{
		return (bool)!data;
	}
	inline bool MYCALL1 isNotEmpty()
	{
		return (data != NULL);
	}
};


string GetFMTStr(D3DFORMAT);

class PicPack {
private:
	BMP *pBmp;
	PicInfo myPicInfo;				//图片信息
	D3DXIMAGE_INFO D3DPicInfo;		//自定图片信息,TODO待完善
	WCHAR strFileName[MAX_PATH];	//文件名

	bool bSaved;
	bool bHasDirectory;

	// 与surface捆绑的信息
	POINT src;
	float zoom;

	// 信息
	WCHAR picInfoStr[256];
public:
	PicPack();
	~PicPack();

	inline bool isEmpty()
	{
		return pBmp->isEmpty();
	}
	inline bool isNotEmpty()
	{
		return pBmp->isNotEmpty();
	}
	inline float GetZoom()
	{
		return zoom;
	}
	inline void SetZoom(float z)
	{
		zoom = z;
	}
	inline POINT GetBase()
	{
		return src;
	}
	inline void SetSrc(POINT point)
	{
		src = point;
	}
	inline BMP* GetPBMP()
	{
		return pBmp;
	}
	WCHAR *GetFileName();

	HRESULT LoadFile(LPDIRECT3DDEVICE9 pDevice, WCHAR[]);
	bool SaveFile(LPDIRECT3DDEVICE9 pDevice, WCHAR[]);

	WCHAR *GetPicInfoStrW();
};

class PicPackList {
private:
	PicPack **data;
	int size;
	int count;

	int cur;
public:
	PicPack *pLivePicPack;
	BMP *pLiveBMP;

	PicPackList();

	bool isEmpty()
	{
		return count == 0;
	}
	int GetCurPos();
	int GetCount();
	PicPack *SetPicPack(int idx);
	PicPack *SetTailPicPack();
	PicPack *SetPrev();
	PicPack *SetNext();
	PicPack *Drop();
	bool Add(PicPack *newpp);
};

class SurferBase {
protected:
	BMP *pBmp;								//BMP指针·捆绑
	UINT *pBufferW, *pBufferH;				//[INT64]目标surface尺寸·捆绑
	bool bHasPic;
	bool bNeedRenew;

	// 计时
	LARGE_INTEGER wheeltick, lastwheeltick;	//滚轮计时
	LARGE_INTEGER freq;						//频率

	// 参数
	POINT ptBase;							//[INT64]用于dragzoom的中心点
	double ptScaleX;						//用于缩放的比例点
	double ptScaleY;
	
public:
	// 映射参数
	float			zoom;					//★放大倍数
	float			lastZoom;				//★上一个放大倍数
	INT64			zoomW, zoomH;			//放大zoom倍后surface的尺寸（不clip的情况），【减少临时运算量，需要手动更新】(需要手动更新场合：缩放，捆绑图片)
	// 映射状态
	float			actualZoomX, actualZoomY;//实际x,y方向上的放大倍数【暂时只用来查看】

	// "源SURFACE" 映射 "目标SURFACE" 参数
	POINT			surfBase;				//[INT64]★放大zoom倍的假想surface相对窗口客户区的位置（左上角）
	// 映射状态
	POINT			oldSurfSize;			//[INT64]存储最后一次surfRenew时的尺寸
	POINT			surfSize;				//[INT64]★surface尺寸
	bool			bPicClipped;			//当前图片是否clip(bPicClipped和bOutClient只在特定函数中计算，需要确保被更新)
	bool			bOutClient;				//surface和窗口客户区是否无交集（在客户区外）

	// "BMP" 映射 "SURFACE" 选项
	bool			bClip;					//★surface是否clip（截去客户区外区域）

	SurferBase();

	inline POINT	GetBase() const;
	inline float	GetZoom() const;
	inline INT64	GetZoomWidth() const;
	inline INT64	GetZoomHeight() const;
	inline void		SetClip(bool clip);
	// surface刷新前操作，定位缩放操作的参数准备
	inline void		SetBasePoint(POINT ptB);
	inline void		SurfHomePR();		//surface回原点
	inline void		SurfLocatePR(LONG x, LONG y);	//surface定位
	inline void		SurfLocatePR(POINT base);
	inline void		SurfCenterPR(int bufferW, int bufferH);
	inline void		SurfMovePR(int dx, int dy);
	//这3个仅用于以一定规则调整缩放倍率的值，并更新zoomW,zoomH等（实际缩放需要另一个函数，在另一个函数中使用调整后的倍率）
	inline void		SurfSetZoomPR(float zoom);// 设置surface缩放倍率
	//这两个仅仅调整zoom，其他不做修改
	void			SurfAdjustZoom_DragPR(int wParam);
	void			SurfAdjustZoom_WheelPR(int wParam);
protected:
	inline void		CalcBindState();
	inline void		CalcSurfSize();
	inline void		PostSurfPosChange();
	inline void		PostZoomChange();
};

inline POINT SurferBase::GetBase() const
{
	return surfBase;
}

inline float SurferBase::GetZoom() const
{
	return zoom;
}

inline INT64 SurferBase::GetZoomWidth() const
{
	return zoomW;
}

inline INT64 SurferBase::GetZoomHeight() const
{
	return zoomH;
}

inline void SurferBase::SetClip(bool clip)
{
	if (bClip != clip)
	{
		bClip = clip;

		CalcSurfSize();
	}
}

inline void SurferBase::SetBasePoint(POINT ptB)
{
	ptBase = ptB;

	POINT surf2cursor;
	surf2cursor.x = -surfBase.x + ptBase.x;
	surf2cursor.y = -surfBase.y + ptBase.y;

	if (zoomW != 0)
		ptScaleX = surf2cursor.x / (double)zoomW;
	else
		ptScaleX = 0;
	if (zoomH != 0)
		ptScaleY = surf2cursor.y / (double)zoomH;
	else
		ptScaleY = 0;
}

inline void SurferBase::SurfHomePR()
{
	surfBase.x = 0;
	surfBase.y = 0;

	PostSurfPosChange();
}

inline void SurferBase::SurfLocatePR(LONG x, LONG y)
{
	surfBase.x = x;
	surfBase.y = y;

	PostSurfPosChange();
}

inline void SurferBase::SurfLocatePR(POINT base)
{
	surfBase = base;

	PostSurfPosChange();
}

inline void SurferBase::SurfCenterPR(int bufferW, int bufferH)
{
	surfBase.x = (LONG)(bufferW / 2 - zoomW / 2);
	surfBase.y = (LONG)(bufferH / 2 - zoomH / 2);

	PostSurfPosChange();
}

inline void SurferBase::SurfMovePR(int dx, int dy)
{
	surfBase.x += dx;
	surfBase.y += dy;

	PostSurfPosChange();
}

inline void SurferBase::SurfSetZoomPR(float zoom)
{
	this->zoom = zoom;
	PostZoomChange();
}

inline void SurferBase::CalcBindState()
{
	bHasPic = (pBmp != NULL)/* && pBmpW && pBmpH*/;
}

inline void SurferBase::CalcSurfSize()
{
	if (pBufferW != NULL && pBufferH != NULL)
	{
		SETPOINT(surfSize, (LONG)zoomW, (LONG)zoomH);
		if (bClip)
		{
			// surface左侧clip
			if (surfBase.x < 0)
				surfSize.x += surfBase.x;
			// surface上侧clip
			if (surfBase.y < 0)
				surfSize.y += surfBase.y;
			// surface右侧clip
			if (surfBase.x + (LONG)zoomW > (INT64)*pBufferW)
				surfSize.x -= surfBase.x + zoomW - (INT64)*pBufferW;
			// surface下侧clip
			if (surfBase.y + (LONG)zoomH > (INT64)*pBufferH)
				surfSize.y -= surfBase.y + zoomH - (INT64)*pBufferH;
		}
	}
}

inline void SurferBase::PostSurfPosChange()
{
	bNeedRenew = true;
	if (pBufferW != NULL && pBufferH != NULL)
	{
		bPicClipped = (
			surfBase.x < 0 || surfBase.y < 0			//surface左上上侧需bClip
			|| surfBase.x + zoomW  >(INT64)*pBufferW		//surface右侧需bClip
			|| surfBase.y + zoomH  >(INT64)*pBufferH		//surface下侧需bClip
			);
		bOutClient = (
			surfBase.x < -zoomW || surfBase.y < -zoomH	//区域超出surface右或下
			|| surfBase.x >= (INT64)*pBufferW				//区域右下不够surface左或上
			|| surfBase.y >= (INT64)*pBufferH
			);

		CalcSurfSize();
	}
}

inline void	SurferBase::PostZoomChange()
{
	bNeedRenew = true;
	if (bHasPic)
	{
		zoomW = (int)(zoom*pBmp->width);
		zoomH = (int)(zoom*pBmp->height);
		actualZoomX = (float)zoomW / pBmp->width;
		actualZoomY = (float)zoomH / pBmp->height;
	}

	PostSurfPosChange();
}

class Surfer :public SurferBase {
protected:
	LPDIRECT3DSURFACE9 surf;				//surface，生成的图片
	LPDIRECT3DDEVICE9 pDevice;				//设备指针·捆绑
	bool bHasDevice, bHasBoth;				//是否具备捆绑值（捆绑对象）

public:
	// 信息
	//int surfrenewcount;					//surface更新计数
	float renewTime;						//surface更新耗时
	UINT8 sampleSchema;						//采样策略
	WCHAR strSurfInfo[512];					//信息字符串

	/******************************************************************************
	surfBase是自由变量，逻辑上可分离，作为属于BMP的信息。
	surfBase和pBmp（或bClip,zoom）迁移过来后，在SurfRenew之前，需更新:zoomW，zoomH或lastZoom&信息
	*******************************************************************************/

	// "源SURFACE" 映射 "目标SURFACE" 直接参数
	POINT			surfDest;				//★surface拷贝到backbuffer, backbuffer的起始点
	RECT			rcSurf;					//★surface要拷贝到backbuffer的区域

	// "BMP" 映射 "SURFACE" 状态
	bool			bSurfFailed;			//surface是否创建失败，在有mainbmp情况下【暂时只用来查看，需要手动更新】
	bool			bSurfClipped;			//当前surface是否bClip

	// 鼠标信息
	POINT			cursorPixel;			//鼠标当前位置对应原图的像素
	bool			bCursorOnPic;			//鼠标是否在图片上
	DWORD			cursorColor;			//鼠标当前位置对应原图的颜色
	UINT8			cursorPos;				//当前鼠标位置（种类，处于哪个区域：图片、空白、窗口外）

private:
	inline void		CalcBindState();		// 更新捆绑标志（捆绑的状态）
	inline void		CalcMapInfo();			// 【bHasDevice】计算映射到目标surface参数

public:
	Surfer();
	~Surfer();

	inline bool		IsSurfNull() const
	{
		return surf == NULL;
	}
	inline bool		IsSurfNotNull() const
	{
		return surf != NULL;
	}

// 设备
	bool			BindDevice(LPDIRECT3DDEVICE9 device);//捆绑设备
	bool			BindBuf(UINT * pBufW, UINT *pBufH);//捆绑目标surface尺寸

// 图片
	bool			BindPic(PicPack *ppic, bool renew = true);//捆绑图片（可以捆绑NULL来清除，或加载/切换图片）
	bool			DeBindPic(PicPack *ppic);			//解绑图片
	void			Clear();							// 清除,释放与图片的捆绑

//surface缩放（强制捆绑刷新操作）
	// 【bHasBoth】根据调整值缩放，并设定位移量，更新映射参数，返回surface是否刷新
	inline bool		SurfZoomRenew(float zoom = -1, int dx = 0, int dy = 0);
	// 【bHasBoth】根据已设定的缩放倍率值缩放，更新映射参数，返回surface是否刷新
	inline bool		SurfZoomRenew(POINT *client2cursor, bool stable = false, bool acce = false);

//surface刷新
	// 【bHasBoth】surface刷新，返回surface是否刷新
	inline bool		SurfRenew(bool acce = false);

	// 【surf】渲染
	inline HRESULT	Render() const;

//surface定制操作
	// 【bHasBoth】自定义拖拽事件处理(根据bClip，surface位置等状态自动处理surface是否更新)，返回surface是否刷新
	inline bool		OnMove_Custom(POINT cursorbias);
	// 【bHasBoth】自定义窗口拉伸事件处理(根据bClip，surface位置等状态自动处理surface是否更新)，返回surface是否刷新
	inline bool		OnWinsize_Custom();//和OnMove_Custom差不多
	bool			SurfSuit(int w, int h);

// 信息
	//记录当前状态信息（更新字符串）
	void			SetInfoStr();
	//更新当前信息（当前鼠标位置对应图片的像素）和信息字符串
	void			GetCurInfo(POINT *cursor, RECT *rcClient);
	//获得信息字符串
	inline const WCHAR *GetInfoStr() const
	{
		return strSurfInfo;
	}
	//获得上一次生成surface的采样算法
	char*			GetSampleSchemaStr();
};

inline bool Surfer::SurfZoomRenew(float zoom, int dx, int dy)
{
	if (bHasBoth)
	{
		if (lastZoom == zoom && dx == 0 && dy == 0)
			return false;
		if (zoom < 0)
			return false;

		//#1调整起始点
		SurfMovePR(dx, dy);

		//#2调整zoom值
		SurfSetZoomPR(zoom);

		//更新标志、变量
		SetClip(true);
		lastZoom = zoom;

		//#3更新surface
		return SurfRenew(false);
	}
	else
		return false;
}

inline bool Surfer::SurfZoomRenew(POINT * client2cursor, bool stable, bool acce)
{
	if (bHasBoth)
	{
		if (lastZoom == zoom)
			return false;

		//调整起始点#1
		if (client2cursor == NULL)//如果参数没提供基准点，就用ptBase作为基准点
			client2cursor = &ptBase;
		if (client2cursor != NULL)
		{
			if (stable)
			{
				surfBase.x = ROUND(client2cursor->x - zoomW*0.5);
				surfBase.y = ROUND(client2cursor->y - zoomH*0.5);
			}
			else
			{
				surfBase.x = ROUND(client2cursor->x - zoomW*ptScaleX);
				surfBase.y = ROUND(client2cursor->y - zoomH*ptScaleY);
			}
		}

		//如果在放大阶段，窗口客户区可以完全容纳surface，则控制surface显示区域
		//防止小图片放大迅速偏离窗口客户区
		if (zoomW <= (INT64)*pBufferW
			&& zoomH <= (INT64)*pBufferH
			&& zoom > lastZoom
			&& zoom < ZOOM_MAXCONTROLED)
		{
			if (surfBase.x < 0)
				surfBase.x = 0;
			if (surfBase.y < 0)
				surfBase.y = 0;
			if (surfBase.x + zoomW >(INT64)*pBufferW)
				surfBase.x = (INT64)*pBufferW - zoomW;
			if (surfBase.y + zoomH >(INT64)*pBufferH)
				surfBase.y = (INT64)*pBufferH - zoomH;
		}

		//#2更新标志、变量
		//bClip = (zoomW > MIN_FORCECLIP_WIDTH || zoomH > MIN_FORCECLIP_HEIGHT);
		SetClip(true);
		PostSurfPosChange();
		lastZoom = zoom;

		//#3更新surface
		return SurfRenew(acce);
	}
	else
		return false;
}

inline bool Surfer::SurfRenew(bool acce)
{
	bool bRenewed = false;
	if (bHasBoth)
	{
		LARGE_INTEGER start, end;
		QueryPerformanceCounter(&start);

		POINT surfBias = { 0, 0 };	//实际surface起点（左上角）相对假想surface的位置（偏移用于定位图片区域来设置surface像素）
		bSurfClipped = false;
		if (bClip)
		{
			// 计算surfBias		：bClip后的surface相对于假想surface的偏移
			// 计算bSurfClipped	：实际surface是否clip
			// surface左侧需clip
			if (surfBase.x < 0)
			{
				surfBias.x = -surfBase.x;

				bSurfClipped = true;
			}
			// surface上侧需clip
			if (surfBase.y < 0)
			{
				surfBias.y = -surfBase.y;

				bSurfClipped = true;
			}
			// surface右侧需clip
			if (surfBase.x + zoomW >(INT64)*pBufferW)
			{
				bSurfClipped = true;
			}
			// surface下侧需clip
			if (surfBase.y + zoomH > (INT64)*pBufferH)
			{
				bSurfClipped = true;
			}
		}


		// 生成surface
		if (!(bOutClient && bClip))//surface只在客户区外并clip时才不创建
		{
			POINT destBase;//实际surface相对于目标buffer的位置（用于透明色块定位）
			if (bClip)
			{
				destBase.x = surfBase.x < 0 ? 0 : surfBase.x;
				destBase.y = surfBase.y < 0 ? 0 : surfBase.y;
			}
			else
			{
				destBase.x = surfBase.x;
				destBase.y = surfBase.y;
			}

			// 如果surface和原来尺寸有变化，则重新创建
			if (oldSurfSize.x != surfSize.x || oldSurfSize.y != surfSize.y || !surf)
			{
				// 清除原表面
				SAFE_RELEASE(surf);
				// 新建surface
				if (FAILED(pDevice->CreateOffscreenPlainSurface(
					surfSize.x, surfSize.y
					, D3DFMT_A8R8G8B8
					, D3DPOOL_SYSTEMMEM
					, &surf
					, NULL)))
				{
					bSurfFailed = true;
					return false;//D3DPOOL_DEFAULT, D3DPOOL_SYSTEMMEM
				}
			}

			// 锁定surface区域
			D3DLOCKED_RECT lockedRect;
			FAILED_RETURN_FALSE(surf->LockRect(&lockedRect, NULL, NULL));
			DWORD *surfData = (DWORD*)lockedRect.pBits;

			// 采样
			if (acce)
			{
				if (zoom > NN_MIN || surfSize.x > MIN_NORMALRENEW_WIDTH || surfSize.y > MIN_NORMALRENEW_HEIGHT)
				{
					pBmp->Sample_NN(surfData, lockedRect.Pitch / 4, surfSize, zoom, 0, surfBias, destBase);
					sampleSchema = SAMPLE_SCHEMA_NN;
				}
				else
				{
					pBmp->Sample_Single(surfData, lockedRect.Pitch / 4, surfSize, zoom, 0, surfBias, destBase);
					sampleSchema = SAMPLE_SCHEMA_SINGLE;
				}
			}
			else
			{
				// 多种映射策略的选择
				if (zoom > NN_MIN)
				{
					pBmp->Sample_NN(surfData, lockedRect.Pitch / 4, surfSize, zoom, 0, surfBias, destBase);
					sampleSchema = SAMPLE_SCHEMA_NN;
				}
				else if (zoom >= 1)
				{
					//小平滑马赛克
					pBmp->Sample_Single(surfData, lockedRect.Pitch / 4, surfSize, zoom, 0, surfBias, destBase);
					sampleSchema = SAMPLE_SCHEMA_SINGLE;
				}
				else if (zoom > 0.5f
					|| pBmp->width > MAXSAMPLE_WIDTH_UPPERLIMIT || pBmp->height > MAXSAMPLE_HEIGHT_UPPERLIMIT)
				{
					//双线性
					pBmp->Sample_BiLinear(surfData, lockedRect.Pitch / 4, surfSize, zoom, 0, surfBias, destBase);
					sampleSchema = SAMPLE_SCHEMA_BILINEAR;
				}
				else
				{
					//最大重采样
					pBmp->Sample_Area(surfData, lockedRect.Pitch / 4, surfSize, zoom, 0, surfBias, destBase);
					sampleSchema = SAMPLE_SCHEMA_MAX;
				}
			}
			surf->UnlockRect();

			bRenewed = true;//设置更新标志
			bSurfFailed = false;
			oldSurfSize = surfSize;

			// bRenewed后续信息获取
			CalcMapInfo();//更新surface拷贝到backbuffer参数

			QueryPerformanceCounter(&end);
			renewTime = (float)(end.QuadPart - start.QuadPart) / freq.QuadPart;
		}
	}

	if (bRenewed)
		bNeedRenew = false;

	return bRenewed;
}

inline HRESULT Surfer::Render() const
{
	if (surf && !bOutClient && bHasDevice)
	{
		// 图片存在且surface与窗口客户区有交集才渲染
		// 获得backbuffer
		LPDIRECT3DSURFACE9 backbuffer = NULL;
		HRESULT hr = pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
		if (FAILED(hr))
			return hr;
		// 拷贝到backbuffer
		hr = pDevice->UpdateSurface(surf, &rcSurf, backbuffer, &surfDest);//统一的拷贝
		if (FAILED(hr))
			return hr;

		SAFE_RELEASE(backbuffer);

		return S_OK;
	}
	else
		return E_FAIL;
}

inline bool Surfer::OnMove_Custom(POINT cursorbias)
{
	if (bHasBoth)
	{
		bool oldPicClipped = bPicClipped;
		bool oldOutClient = bOutClient;

		SurfMovePR(cursorbias.x, cursorbias.y);

		bool bRenewed = false;
		if (bClip)
		{
			//移动时如果有bClip标志，并且图片小于一定尺寸，则取消clip。
			//如果先前surface被clip，就重新生成surface，增加拖动速度。
			if (zoomW <= MAX_NOCLIP_WIDTH_DRAG && zoomH <= MAX_NOCLIP_HEIGHT_DRAG)
			{
				SetClip(false);
				if (bSurfClipped)
				{
					SurfRenew(false);//取消clip后直接生成最终surface
					bRenewed = true;
				}
			}
			//如果图片始终全部在窗口客户区范围内（前后bSurfClipped状态都是0）
			//或者始终在客户区外（前后bOutClient状态都是1），就不更新
			else if ((oldPicClipped || bPicClipped) && (!oldOutClient || !bOutClient))
			{
				SurfRenew(true);
				bRenewed = true;
			}
		}
		//本次没更新surface，则更新surface映射信息，否则surface不动
		if (!bRenewed)
		{
			CalcMapInfo();//更新surface拷贝到backbuffer参数
		}

		return bRenewed;
	}
	else
		return false;
}

inline bool Surfer::OnWinsize_Custom()
{
	if (bHasBoth)
	{
		bool oldPicClipped = bPicClipped;
		bool oldOutClient = bOutClient;

		PostSurfPosChange();

		bool bRenewed = false;
		if (bClip)
		{
			//移动时如果有bClip标志，并且图片小于一定尺寸，则取消clip。
			//如果先前surface被clip，就重新生成surface，增加拖动速度。
			if (zoomW <= MAX_NOCLIP_WIDTH_DRAG && zoomH <= MAX_NOCLIP_HEIGHT_DRAG)
			{
				SetClip(false);
				if (bSurfClipped)
				{
					SurfRenew(false);
					bRenewed = true;
				}
			}
			//如果图片始终全部在窗口客户区范围内（前后bSurfClipped状态都是0）
			//或者始终在客户区外（前后bOutClient状态都是1），或者不设置bClip标志，就不更新
			else if ((oldPicClipped || bPicClipped) && (!oldOutClient || !bOutClient))
			{
				SurfRenew(true);
				bRenewed = true;
			}
		}
		//本次没更新surface，则更新surface映射信息
		if (!bRenewed)
		{
			CalcMapInfo();//更新surface拷贝到backbuffer参数
		}

		return bRenewed;
	}
	else
		return false;
}