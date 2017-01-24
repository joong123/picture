#pragma once 

#include<new>
#include<iostream>
#include<stdlib.h>
#include<winnt.h>

#include "Pic.h"
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

#define RGB2GRAY(R, G, B)				((R)*38 + (G)*75 + (B)*15) >> 7

// 内存
#define SAFE_RELEASE(p)					{ if (p) { (p)->Release(); (p) = NULL; } }
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

// 图像缩放
#define MAX_ZOOM						200
#define MAX_CONTROLEDZOOM				0.5f
#define MIN_ZOOM						0.0f
#define MIN_DELTAZOOM					0.000001f

#define DELTAZOOMNORMAL					0.25f
#define DELTAZOOM1						0.002f
#define DELTAZOOM2						0.002f

#define ZOOMOUT_SHRINK					0.80f

// 采样策略
#define SAMPLE_SCHEMA_UNKNOWN			0
#define SAMPLE_SCHEMA_NN				1
#define SAMPLE_SCHEMA_BILINEAR			2
#define SAMPLE_SCHEMA_CUBE				3
#define SAMPLE_SCHEMA_SINGLE			4
#define SAMPLE_SCHEMA_MAX				5

#define NN_MIN							3.0f	//大于这个放大倍率使用NN采样
#define MAXSAMPLE_WIDTH_UPPERLIMIT		20000	//大于这个尺寸图像默认不进行最大采样MAXSAMPLE
#define MAXSAMPLE_HEIGHT_UPPERLIMIT		MAXSAMPLE_WIDTH_UPPERLIMIT

#define MIN_NORMALRENEW_WIDTH			500		//图像小于这个尺寸，在加速renew时才允许使用SingleSample(太大缩放过程可能有卡顿)
#define MIN_NORMALRENEW_HEIGHT			MIN_NORMALRENEW_WIDTH

#define MIN_FORCECLIP_WIDTH				1500	//图像大于这个尺寸，则设置clip。增加缩放速度
#define MIN_FORCECLIP_HEIGHT			MIN_FORCECLIP_WIDTH

#define MAX_NOCLIP_WIDTH_DRAG			3000	//图像小于这个尺寸，拖动图像时取消clip。增加拖动速度
#define MAX_NOCLIP_HEIGHT_DRAG			MAX_NOCLIP_WIDTH_DRAG

// 图片列表
#define PICPACKLIST_SIZE_INIT			3
#define PICPACKLIST_SIZE_INCREMENT		2

// 按键状态
#define KEYSTATEMASK_DOWN				0x8000
#define ISKEYDOWN(KEY)					(GetAsyncKeyState(KEY) & KEYSTATEMASK_DOWN)

// 鼠标位置
#define CURSORPOS_PIC					0
#define CURSORPOS_BLANK					1
#define CURSORPOS_OUTWINDOW				2

// 颜色
#define BACKCOLOR_INIT					0xFF888888
#define SETALPHA(C, A)					((C & 0xFFFFFF) | (A << 24))

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
	inline DWORD AlphaBlend(DWORD blender) {
		byte alpha = (byte)a;
		DWORD res = 0;
		//res += (byte)round(a) << 24;
		res += (DWORD)((r*alpha + ((blender >> 16) & 0xFF)*(255 - alpha)) / 255) << 16;
		res += (DWORD)((g*alpha + ((blender >> 8) & 0xFF)*(255 - alpha)) / 255) << 8;
		res += (DWORD)((b*alpha + (blender & 0xFF)*(255 - alpha)) / 255);

		return res;
	}
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

	//示例
	/*static inline DWORD AlphaBlend1(COLOR_F3 src) {
		byte alpha = roundf2b(src.a);
		DWORD res = 0;
		res += (DWORD)alphablendtable1[(byte)(src.r)][alpha] << 16;
		res += (DWORD)alphablendtable1[(byte)(src.g)][alpha] << 8;
		res += (DWORD)alphablendtable1[(byte)(src.b)][alpha];

		return res;
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
	bool alphaon;

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
	surfbias:surf相对原图偏移（用于原图选区）
	backbase:surf相对client窗口（用于辅助透明背景绘制）

	双线性插值的两种策略（有色差，无色差）在Copy_Single中详细定义
	****************************************************/
	// 最大无损失采样（缩小效果较好，缩小速度最慢）
	inline bool MYCALL1 Copy_MaxSample(DWORD *surfData, int pitch, const POINT &surfsize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 }) 
	{
		UINT32 zoomw = (UINT32)(width*zoom);
		UINT32 zoomh = (UINT32)(height*zoom);

		COLORTYPE xpixspan = (COLORTYPE)width / zoomw;//新像素x方向跨度
		COLORTYPE ypixspan = (COLORTYPE)height / zoomh;//新像素y方向跨度
		COLORTYPE spanarea = xpixspan*ypixspan;//新像素面积 x*y
		float baseweight = 1.0f / spanarea;
		COLORTYPE *xbase = new COLORTYPE[surfsize.x + 1];//新像素左上起始点数组，x方向
		COLORTYPE *ybase = new COLORTYPE[surfsize.y + 1];//新像素左上起始点数组，y方向
		bool *xincolor1 = new bool[surfsize.x + 1];

		// 设置新像素起点值数组,限定需要的区间
		float actualzx = (float)width / zoomw;
		//float actualzy = (float)height / zoomh;
		for (int i = 0; i < surfsize.x + 1; i++)
		{
			xbase[i] = (COLORTYPE)(i + surfbias.x)*actualzx;
			xincolor1[i] = ((backbase.x + i) & TRANSPARENTBACK_MASK) > 0;
		}
		for (int i = 0; i < surfsize.y + 1; i++)
			ybase[i] = (COLORTYPE)(i + surfbias.y)*height / zoomh;//height / zoomh

		int index;
		COLOR_F3 rcolor = 0;//颜色累加器
		for (int i = 0; i < surfsize.y; i++)
		{
			int starty = (int)ybase[i];//新像素左上起始点，y方向
			int endy = ceilf2i(ybase[i + 1]);//新像素左上结束点，y方向
			bool yincolor1 = ((backbase.y + i) & TRANSPARENTBACK_MASK) > 0;

			for (int j = 0; j < surfsize.x; j++)
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
	inline bool MYCALL1 Copy_Single(DWORD *surfData, int pitch, const POINT &surfsize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 }) 
	{
		int index;

		const float auxz = 0.3f / zoom;//加速
		float auxfront = 0.5f - auxz, auxrear = 0.5f + auxz;//加速

		float *xsto = new float[surfsize.x];//加速,防止处于二层循环的x重复计算
		bool *xincolor1 = new bool[surfsize.x];
		for (int j = 0; j < surfsize.x; j++)//提前运算x方向的变量
		{
			xsto[j] = (j + surfbias.x + 0.5f) / zoom - 0.5f;
			xincolor1[j] = ((backbase.x + j) & TRANSPARENTBACK_MASK) > 0;
		}
		if (alphaon)// 有alpha通道
		{
			for (int i = 0; i < surfsize.y; i++)
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

				for (int j = 0; j < surfsize.x; j++)
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
			for (int i = 0; i < surfsize.y; i++)
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

				for (int j = 0; j < surfsize.x; j++)
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
	inline bool MYCALL1 Copy_NN(DWORD *surfData, int pitch, const POINT &surfsize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 }) 
	{
		int index;

		int *bxarray = new int[surfsize.x];
		bool *xincolor1 = new bool[surfsize.x];
		for (int j = 0; j < surfsize.x; j++)//加速
		{
			bxarray[j] = (int)(min((j + surfbias.x + 0.5f) / zoom, width - 1));
			xincolor1[j] = ((backbase.x + j) & TRANSPARENTBACK_MASK) > 0;
		}
		if (alphaon)
		{
			for (int i = 0; i < surfsize.y; i++)
			{
				int by = (int)(min((i + surfbias.y + 0.5f) / zoom, height - 1));
				DWORD *databy = (DWORD*)data[by];

				bool yincolor1 = ((backbase.y + i) & TRANSPARENTBACK_MASK) > 0;

				for (int j = 0; j < surfsize.x; j++)
				{
					index = i*pitch + j;

					int bx = bxarray[j];
					DWORD bmppixel = databy[bx];//读取bmp信息时加上surface偏移

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
			for (int i = 0; i < surfsize.y; i++)
			{
				int by = (int)(min((i + surfbias.y + 0.5f) / zoom, height - 1));
				DWORD *databy = (DWORD*)data[by];
				for (int j = 0; j < surfsize.x; j++)
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
	inline bool MYCALL1 Copy_BiLinear(DWORD *surfData, int pitch, const POINT &surfsize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 }) 
	{
		int index;

		float *xsto = new float[surfsize.x];//加速,防止处于二层循环的x重复计算
		bool *xincolor1 = new bool[surfsize.x];
		for (int j = 0; j < surfsize.x; j++)//提前运算x方向的变量，终极加速
		{
			xsto[j] = (j + surfbias.x + 0.5f) / zoom - 0.5f;
			xincolor1[j] = ((backbase.x + j) & TRANSPARENTBACK_MASK) > 0;
		}
		if (alphaon)
		{
			for (int i = 0; i < surfsize.y; i++)
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

				for (int j = 0; j < surfsize.x; j++)
				{
					index = i*pitch + j;

					// 对应原图像素位置
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
			for (int i = 0; i < surfsize.y; i++)
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

				for (int j = 0; j < surfsize.x; j++)
				{
					// 对应原图像素位置
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
	BMP bmp;
	PicInfo myimginfo;				//图片信息
	D3DXIMAGE_INFO d3dimginfo;		//自定图片信息
	WCHAR strFileName[MAX_PATH];	//文件名

	// 与surface捆绑的信息
	POINT src;
	float zoom;

	// 信息
	WCHAR picinfostr[MAX_PATH];
public:
	PicPack();
	~PicPack();

	inline bool isEmpty()
	{
		return bmp.isEmpty();
	}
	inline bool isNotEmpty()
	{
		return bmp.isNotEmpty();
	}
	inline float GetZoom()
	{
		return zoom;
	}
	inline void SetZoom(float z)
	{
		zoom = z;
	}
	inline POINT GetSrc()
	{
		return src;
	}
	inline void SetSrc(POINT point)
	{
		src = point;
	}
	inline BMP* GetPBMP()
	{
		return &bmp;
	}
	WCHAR *GetFileName();

	HRESULT LoadFile(LPDIRECT3DDEVICE9 pdev, WCHAR[]);
	bool SaveFile(LPDIRECT3DDEVICE9 pdev, WCHAR[]);

	WCHAR *GetPicInfoStrW();
};

class PicPackList {
private:
	PicPack **plist;
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
	PicPack *SetPicPack();
	PicPack *GetLast();
	PicPack *GetNext();
	PicPack *Drop();
	bool Add(PicPack *newpp);
};

/*********************************************
*********************************************/
struct Surfer {
public:
	LPDIRECT3DSURFACE9 surf;				//surface
	LPDIRECT3DDEVICE9 pdev;					//设备指针·捆绑
	BMP *pbmp;								//BMP指针·加速·捆绑
	int *pbmpw, *pbmph;						//BMP尺寸·加速
	UINT *pbufw, *pbufh;					//目标surface尺寸·加速
	bool haspic, hasdevice, hasboth;

	//int surfrenewcount;					//surface更新计数
	float renewtime;						//surface更新耗时
	UINT8 sampleschema;						//采样策略
	WCHAR surferinfostr[512];				//信息字符串

	LARGE_INTEGER wheeltick, lastwheeltick;	//滚轮计时
	LARGE_INTEGER freq;						//频率

	/******************************************************************************
	surfsrc是自由变量，是BMP显示到目标SURFACE的主键，和"选项"共同作用。逻辑上可分离，作为属于的BMP信息。
	其他变量内部维护
	surfsrc和bmp（或clip,zoom）迁移过来后，在SurfRenew之前，需更新:【zoomw，zoomh或oldsurfzoom】&信息
	*******************************************************************************/

	// "BMP" 映射 "SURFACE" 选项
	bool clip;								//surface是否clip（截去客户区外区域）
	D3DCOLOR backcolor;						//背景色（暂时没用）
	// "BMP" 映射 "SURFACE" 参数
	float surfzoom;							//直接操控的放大倍数
	float oldsurfzoom;						//上一个放大倍数
	int zoomw, zoomh;						//放大surfzoom倍后假想surface的尺寸，【减少临时运算量】

	// "BMP" 映射 "SURFACE" 状态
	float actualzoomx, actualzoomy;			//实际x,y方向上的放大倍数【这个标志暂时只用来查看】
	bool surffailed;						//surface是否创建失败，在有mainbmp情况下【这个标志暂时只用来查看】
	bool surfclipped;						//当前surface是否clip

	// "源SURFACE" 映射 "目标SURFACE" 参数
	POINT surfsrc;							//放大surfzoom倍的假想surface的左上角到窗口客户区的向量
	POINT basepoint;						//用于dragzoom的中心点
	// "源SURFACE" 映射 "目标SURFACE" 直接参数
	POINT surfbase;							//surface拷贝到backbuffer, backbuffer的起始点
	RECT surfrect;							//surface要拷贝到backbuffer的区域

	// "源SURFACE" 映射 "目标SURFACE" 状态
	POINT surfsize;							//surface尺寸
	bool picclipped;						//当前图片是否clip
	bool outsideclient;						//surface和窗口客户区是否无交集（在客户区外）
// 信息
	POINT picpixel;							//鼠标当前位置对应原图的像素
	bool pixelcolorvalid;
	DWORD picpixelcolor;					//鼠标当前位置对应原图的颜色
	UINT8 cursorpos;						//当前鼠标位置（种类，处于哪个区域：图片、空白、窗口外）

	void CalcBindSituation();
public:
	Surfer();
	~Surfer();

	inline POINT GetSrc()
	{
		return surfsrc;
	}
	inline float GetZoom()
	{
		return surfzoom;
	}
// 设备
	bool BindDevice(LPDIRECT3DDEVICE9 device);//捆绑设备
	bool BindBuf(UINT * bufw, UINT *bufh);	//捆绑目标surface尺寸
	void SetBackcolor(D3DCOLOR bc);			

// 图片
	bool BindPic(PicPack *ppic, bool renew = true, bool debind = true);//捆绑图片
	bool DeBindPic(PicPack *ppic);						//解绑图片
	void Refresh();//【haspic】更新已捆绑的图片关键信息(zoomw,zoomh)【图像尺寸变化时必须更新】
	void Clear();							// 清除,释放与图片的捆绑
	
// surface定位操作
	void SetBasePoint(POINT bp);
	inline void SurfHoming()
	{
		surfsrc.x = 0;
		surfsrc.y = 0;
	}											//surface回原点
	inline void SurfLocate(LONG x, LONG y)		//surface定位
	{
		surfsrc.x = x;
		surfsrc.y = y;
	}
	inline void SurfCenter(int bufferw, int bufferh)
	{
		if (zoomw <= bufferw
			&& zoomh <= bufferh)
		{
			surfsrc.x = (LONG)(zoomw / 2 - bufferw / 2);
			surfsrc.y = (LONG)(zoomh / 2 - bufferh / 2);
		}
		else
		{
			surfsrc.x = 0;
			surfsrc.y = 0;
		}
	}					//surface居中（相对目标surface）
	inline void SurfMove(int dx, int dy)
	{
		surfsrc.x -= dx;
		surfsrc.y -= dy;
	}								//surface移动，不带surface更新，外部条件更新
	inline void SurfSetZoom(float z)
	{
		surfzoom = z;
	}
	// 设置surface缩放倍率	
	inline void SurfAdjustZoom_normal1(int wParam)
	{
		float adds = 0;
		QueryPerformanceCounter(&wheeltick);

		adds = (float)((DELTAZOOMNORMAL*surfzoom));//增长速度与时间和放大倍率有关
		lastwheeltick.QuadPart = wheeltick.QuadPart;

		adds *= wParam / 120.0f;
		if (adds < 0)
			adds *= ZOOMOUT_SHRINK;//放大缩小一致化

		surfzoom += adds;
		//zoom上下限
		if (surfzoom > MAX_ZOOM)
			surfzoom = MAX_ZOOM;
		if (surfzoom < 0)
			surfzoom = 0;
	}
	inline void SurfAdjustZoom_wheel(int wParam)
	{
		float adds = 0;
		QueryPerformanceCounter(&wheeltick);

		if (surfzoom < 1)
			adds = (float)(freq.QuadPart / (wheeltick.QuadPart - lastwheeltick.QuadPart)
				*(DELTAZOOM1*surfzoom)) + MIN_DELTAZOOM;//增长速度与时间和放大倍率有关
		else
			adds = (float)(freq.QuadPart / (wheeltick.QuadPart - lastwheeltick.QuadPart)
				*(DELTAZOOM2*surfzoom)) + MIN_DELTAZOOM;//增长速度与时间和放大倍率有关
		lastwheeltick.QuadPart = wheeltick.QuadPart;

		adds *= wParam / 120.0f;
		if (adds < 0)
			adds *= ZOOMOUT_SHRINK;//放大缩小一致化

		surfzoom += adds;
		//zoom上下限
		if (surfzoom > MAX_ZOOM)
			surfzoom = MAX_ZOOM;
		if (surfzoom < 0)
			surfzoom = 0;
		/*if (surfzoom*min(*pbmpw, *pbmph) < 1)
			surfzoom = 1.00001f / min(*pbmpw, *pbmph);//控制surface尺寸>0*/
	}

	// 【hasboth】根据调整值调整缩放倍率，更新映射参数
	inline void SurfZoomRenew(float zoom = -1, int dx = 0, int dy = 0)
	{
		if (hasboth)
		{
			if (oldsurfzoom == zoom && dx == 0 && dy == 0)
				return;

			surfsrc.x += dx;
			surfsrc.y += dy;

			if (zoom >= 0)
				surfzoom = zoom;
			zoomw = (int)(surfzoom**pbmpw);
			zoomh = (int)(surfzoom**pbmph);
			actualzoomx = (float)zoomw / *pbmpw;
			actualzoomy = (float)zoomh / *pbmph;

			//更新clip标志
			clip = true;

			oldsurfzoom = surfzoom;

			SurfRenew(false);
		}
	}
	inline void SurfZoomRenew(POINT client2cursor, bool stable = false, bool acce = false)
	{
		if (hasboth)
		{
			if (oldsurfzoom == surfzoom)
				return;

			int oldzoomw = zoomw, oldzoomh = zoomh;
			zoomw = (int)(surfzoom**pbmpw);
			zoomh = (int)(surfzoom**pbmph);

			actualzoomx = (float)zoomw / *pbmpw;
			actualzoomy = (float)zoomh / *pbmph;

			UINT oldswidth = (UINT)(*pbmpw*oldsurfzoom)
				, oldsheight = (UINT)(*pbmph*oldsurfzoom);

			//调整起始点#1
			if (ISKEYDOWN(VK_MENU) || stable)
			{
				surfsrc.x = (LONG)(zoomw / 2) - oldzoomw / 2 + surfsrc.x;
				surfsrc.y = (LONG)(zoomh / 2) - oldzoomh / 2 + surfsrc.y;
			}
			else
			{
				POINT surf2cursor;
				surf2cursor.x = surfsrc.x + client2cursor.x;
				surf2cursor.y = surfsrc.y + client2cursor.y;

				float tempf = (float)surf2cursor.x*surfzoom / oldsurfzoom;//平滑方式
				if (tempf > 0)
					surfsrc.x = (LONG)(tempf + 0.5f) - client2cursor.x;
				else
					surfsrc.x = (LONG)(tempf - 0.5f) - client2cursor.x;
				tempf = (float)surf2cursor.y*surfzoom / oldsurfzoom;//平滑方式
				if (tempf > 0)
					surfsrc.y = (LONG)(tempf + 0.5f) - client2cursor.y;
				else
					surfsrc.y = (LONG)(tempf - 0.5f) - client2cursor.y;
			}

			//如果在放大阶段，窗口客户区可以完全容纳surface，则控制surface显示区域
			//防止小图片放大迅速偏离窗口客户区
			if (zoomw <= (int)*pbufw
				&& zoomh <= (int)*pbufh
				&& surfzoom > oldsurfzoom
				&& surfzoom < MAX_CONTROLEDZOOM)
			{
				if (surfsrc.x > 0)
					surfsrc.x = 0;
				if (surfsrc.y > 0)
					surfsrc.y = 0;
				if (surfsrc.x < (LONG)zoomw - (LONG)*pbufw)
					surfsrc.x = (LONG)zoomw - (LONG)*pbufw;
				if (surfsrc.y < (LONG)zoomh - (LONG)*pbufh)
					surfsrc.y = (LONG)zoomh - (LONG)*pbufh;
			}

			//更新clip标志
			//clip = (zoomw > MIN_FORCECLIP_WIDTH || zoomh > MIN_FORCECLIP_HEIGHT);
			clip = true;

			oldsurfzoom = surfzoom;

			SurfRenew(acce);
		}
	}

	// 【hasboth】surface更新
	inline bool SurfRenew(bool acce = false)
	{
		if (hasboth)
		{
			LARGE_INTEGER start, end;
			QueryPerformanceCounter(&start);

			outsideclient = (
				surfsrc.x > zoomw || surfsrc.y > zoomh	//区域超出surface右或下
				|| surfsrc.x + (int)*pbufw <= 0			//区域右下不够surface左或上
				|| surfsrc.y + (int)*pbufh <= 0
				);

			POINT oldsurfsize = surfsize;
			surfsize.x = (LONG)zoomw;
			surfsize.y = (LONG)zoomh;
			POINT surfbias = { 0, 0 };	//实际surface起点（左上角）相对假想surface的位置（偏移用于定位图片区域来设置surface像素）
			surfclipped = false;
			if (clip)
			{
				// 计算surfsize	 ：clip后的surface尺寸,
				// 计算surfbias	 ：clip后的surface相对于假想surface的偏移
				// 计算surfclipped ：实际surface是否clip
				// surface左侧需clip
				if (surfsrc.x > 0)
				{
					surfsize.x -= surfsrc.x;
					surfbias.x = surfsrc.x;

					surfclipped = true;
				}
				// surface上侧需clip
				if (surfsrc.y > 0)
				{
					surfsize.y -= surfsrc.y;
					surfbias.y = surfsrc.y;

					surfclipped = true;
				}
				// surface右侧需clip
				if ((LONG)zoomw - surfsrc.x > (int)*pbufw)
				{
					surfsize.x -= zoomw - surfsrc.x - (int)*pbufw;

					surfclipped = true;
				}
				// surface下侧需clip
				if ((LONG)zoomh - surfsrc.y > (int)*pbufh)
				{
					surfsize.y -= zoomh - surfsrc.y - (int)*pbufh;

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

			// 生成surface
			if (!(outsideclient && clip))
			{
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

				if (oldsurfsize.x != surfsize.x || oldsurfsize.y != surfsize.y || !surf)
				{
					// 清除原表面
					SAFE_RELEASE(surf);
					// 新建surface
					if (FAILED(pdev->CreateOffscreenPlainSurface(
						surfsize.x, surfsize.y
						, D3DFMT_A8R8G8B8
						, D3DPOOL_SYSTEMMEM
						, &surf
						, NULL)))
						return false;//D3DPOOL_DEFAULT, D3DPOOL_SYSTEMMEM
				}

				// 锁定surface区域
				D3DLOCKED_RECT lockedRect;
				surf->LockRect(&lockedRect, NULL, NULL);
				DWORD *surfData = (DWORD*)lockedRect.pBits;

				// 拷贝
				if (acce)
				{
					if (surfzoom > NN_MIN)
					{
						pbmp->Copy_NN(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
						sampleschema = SAMPLE_SCHEMA_NN;
					}
					else if (surfsize.x > MIN_NORMALRENEW_WIDTH || surfsize.y > MIN_NORMALRENEW_HEIGHT)
					{
						pbmp->Copy_NN(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
						sampleschema = SAMPLE_SCHEMA_NN;
					}
					else
					{
						pbmp->Copy_Single(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
						sampleschema = SAMPLE_SCHEMA_SINGLE;
					}
				}
				else
				{
					// 多种映射策略的选择
					if (surfzoom > NN_MIN)
					{
						pbmp->Copy_NN(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
						sampleschema = SAMPLE_SCHEMA_NN;
					}
					else if (surfzoom >= 1)
					{
						//小平滑马赛克
						pbmp->Copy_Single(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
						sampleschema = SAMPLE_SCHEMA_SINGLE;
					}
					else if (surfzoom > 0.5f
						|| pbmp->width > MAXSAMPLE_WIDTH_UPPERLIMIT || pbmp->height > MAXSAMPLE_HEIGHT_UPPERLIMIT)
					{
						//双线性
						pbmp->Copy_BiLinear(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
						sampleschema = SAMPLE_SCHEMA_BILINEAR;
					}
					else
					{
						//最大重采样
						pbmp->Copy_MaxSample(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
						sampleschema = SAMPLE_SCHEMA_MAX;
					}
				}
				surf->UnlockRect();
			}

			if (surf)
			{
				surffailed = false;

				// renew后续信息获取
				picclipped = (
					surfsrc.x > 0 || surfsrc.y > 0			//surface左上上侧需clip
					|| zoomw - surfsrc.x > (int)*pbufw		//surface右侧需clip
					|| zoomh - surfsrc.y > (int)*pbufh		//surface下侧需clip
					);
				SurfCalcMapInfo();//更新surface拷贝到backbuffer参数

				QueryPerformanceCounter(&end);
				renewtime = (float)(end.QuadPart - start.QuadPart) / freq.QuadPart;
				return true;
			}
			else
			{
				surffailed = true;

				return false;
			}
		}
		else
			return false;
	}
	// 【hasdevice】计算映射到目标surface参数
	inline void SurfCalcMapInfo()
	{
		if (hasdevice)
		{
			//计算surfrect和surfbase
			//surfbase：surface拷贝到backbuffer起点，创建surface时计算过，还需要实时计算
			if (surfsrc.x > 0)
				surfbase.x = 0;
			else
				surfbase.x = -surfsrc.x;
			if (surfsrc.y > 0)
				surfbase.y = 0;
			else
				surfbase.y = -surfsrc.y;

			if (clip)
			{
				surfrect.left = 0;
				surfrect.top = 0;
				surfrect.right = min((int)*pbufw - surfbase.x, surfsize.x);//如果起始点右下区域不够窗口客户区，则削减右侧和下侧
				surfrect.bottom = min((int)*pbufh - surfbase.y, surfsize.y);
			}
			else
			{
				surfrect.left = max(surfsrc.x, 0);//如果起始点surfsrc某一维小于0，则平移区域到0起点，并设置destpoint为非0
				surfrect.top = max(surfsrc.y, 0);
				surfrect.right = min(zoomw, surfrect.left + (int)*pbufw - surfbase.x);//如果起始点右下区域不够窗口客户区，则削减右侧和下侧
				surfrect.bottom = min(zoomh, surfrect.top + (int)*pbufh - surfbase.y);
			}
		}
	}
	// 【hasboth】自定义拖拽事件处理
	inline bool OnDrag_Custom(POINT cursorbias)
	{
		if (hasboth)
		{
			SurfMove(cursorbias.x, cursorbias.y);

			bool lastpicclipped = picclipped;
			picclipped = (
				surfsrc.x > 0 || surfsrc.y > 0				//surface左上上侧需clip
				|| zoomw - surfsrc.x > (int)*pbufw			//surface右侧需clip
				|| zoomh - surfsrc.y > (int)*pbufh			//surface下侧需clip
				);
			outsideclient = (
				surfsrc.x > zoomw || surfsrc.y > zoomh		//区域超出surface右或下
				|| surfsrc.x + (int)*pbufw <= 0				//区域右下不够surface左或上
				|| surfsrc.y + (int)*pbufh <= 0
				);
			bool renew = false;
			if (clip)
			{
				//移动时如果有clip标志，并且图片小于一定尺寸，则取消clip。
				//如果先前surface被clip，就重新生成surface，增加拖动速度。
				if (zoomw <= MAX_NOCLIP_WIDTH_DRAG && zoomh <= MAX_NOCLIP_HEIGHT_DRAG)
				{
					clip = false;
					if (surfclipped)
					{
						SurfRenew(true);
						renew = true;
					}
				}
				//如果图片始终全部在窗口客户区范围内（前后surfclipped状态都是0）
				//或者始终在客户区外（前后outsideclient状态都是1），或者不设置clip标志，就不更新
				else if ((lastpicclipped || picclipped)
					&& !outsideclient)
				{
					SurfRenew(true);
					renew = true;
				}
			}
			//本次没更新surface，则更新surface映射信息，否则surface不动
			if (!renew)
			{
				SurfCalcMapInfo();//更新surface拷贝到backbuffer参数
			}

			return renew;
		}
		else
			return false;
	}
	// 【hasboth】自定义窗口拉伸事件处理
	inline bool OnWinsize_Custom()//和ondrag差不多
	{
		if (hasboth)
		{
			bool lastpicclipped = picclipped;
			picclipped = (
				surfsrc.x > 0 || surfsrc.y > 0		//surface左上上侧需clip
				|| zoomw - surfsrc.x > (int)*pbufw	//surface右侧需clip
				|| zoomh - surfsrc.y > (int)*pbufh	//surface下侧需clip
				);
			outsideclient = (
				surfsrc.x > zoomw || surfsrc.y > zoomh	//区域超出surface右或下
				|| surfsrc.x + (int)*pbufw <= 0			//区域右下不够surface左或上
				|| surfsrc.y + (int)*pbufh <= 0
				);
			bool renew = false;
			if (clip)
			{
				//移动时如果有clip标志，并且图片小于一定尺寸，则取消clip。
				//如果先前surface被clip，就重新生成surface，增加拖动速度。
				if (zoomw <= MAX_NOCLIP_WIDTH_DRAG && zoomh <= MAX_NOCLIP_HEIGHT_DRAG)
				{
					clip = false;
					if (surfclipped)
					{
						SurfRenew(true);
						renew = true;
					}
				}
				//如果图片始终全部在窗口客户区范围内（前后surfclipped状态都是0）
				//或者始终在客户区外（前后outsideclient状态都是1），或者不设置clip标志，就不更新
				else if ((lastpicclipped || picclipped)
					&& !outsideclient)
				{
					SurfRenew(true);
					renew = true;
				}
			}
			//本次没更新surface，则更新surface映射信息
			if (!renew)
			{
				SurfCalcMapInfo();//更新surface拷贝到backbuffer参数
			}

			return renew;
		}
		else
			return false;
	}
	// 【hasboth】
	inline void SurfSuit(int w, int h)
	{
		if (hasboth)
		{
			if (w < 1 || h < 1)
				return;

			SurfZoomRenew(min((float)w / *pbmpw, (float)h / *pbmph), -surfsrc.x, -surfsrc.y);
		}
	}

// 【surf】渲染
	inline HRESULT Render()
	{
		if (surf && !outsideclient)
		{
			// 图片存在且surface与窗口客户区有交集才渲染
			// 获得backbuffer
			LPDIRECT3DSURFACE9 backbuffer = NULL;
			HRESULT hr = pdev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
			if (FAILED(hr))
				return hr;
			// 拷贝到backbuffer
			hr = pdev->UpdateSurface(surf, &surfrect, backbuffer, &surfbase);//统一的拷贝
			if (FAILED(hr))
				return hr;

			SAFE_RELEASE(backbuffer);

			return S_OK;
		}
		else
			return E_FAIL;
	}

// 信息
	inline WCHAR *GetInfoStr()
	{
		return surferinfostr;
	}
	inline void SetInfoStr()
	{
		WCHAR subinfo[256] = { 0 };
		surferinfostr[0] = L'\0';

		// buffer尺寸、surface尺寸
		swprintf_s(subinfo, _T("SURFACE: %d × %d     TIME: %.3fms (%S)\n\
		ZOOM: %.4f\n")
			, surfsize.x, surfsize.y, renewtime*1000.0f, GetSampleSchemaStr()
			, surfzoom);
		wcscat_s(surferinfostr, subinfo);

		// surface起始点、clipsurface起始点
		swprintf_s(subinfo, _T("BASE: %d, %d\n")
			, -surfsrc.x, -surfsrc.y);
		wcscat_s(surferinfostr, subinfo);

		// 鼠标像素位置
		if (pbmp)
		{
			if (pbmp->isNotEmpty())
				swprintf_s(subinfo, _T("PIXEL: %d, %d\n"), picpixel.x, picpixel.y);
			else
				swprintf_s(subinfo, _T("PIXEL:-, -\n"));
		}
		else
			swprintf_s(subinfo, _T("PIXEL:-, -\n"));
		wcscat_s(surferinfostr, subinfo);

		// 鼠标像素颜色、屏幕像素颜色、背景色
		swprintf_s(subinfo, _T("COLOR: %02X.%06X.ARGB\n")
			, (picpixelcolor >> 24), (picpixelcolor & 0xFFFFFF));
		wcscat_s(surferinfostr, subinfo);

		// 附加信息
		//"intended surface: %d× %d\n
		//	clipsurface base: %d, %d\n
		//	PZ: X %.4f Y %.4f\n"
		//	, zoomw, zoomh
		//	, surfbase.x, surfbase.y
		//	, actualzoomx, actualzoomy

	}
	inline void GetCurInfo(POINT cursor, RECT clientrect)
	{
		if (haspic)
		{
			// 获得当前鼠标位置（种类）
			POINT surf2cursor = { surfsrc.x + cursor.x - clientrect.left
				,surfsrc.y + cursor.y - clientrect.top };
			if (OUTSIDE(cursor, clientrect))
				cursorpos = CURSORPOS_OUTWINDOW;
			else if (!surf)
				cursorpos = CURSORPOS_BLANK;
			else if (OUTSIDE2(surf2cursor, zoomw - 1, zoomh - 1))
				cursorpos = CURSORPOS_BLANK;
			else
				cursorpos = CURSORPOS_PIC;

			// 所处像素
			//如果除以actualzoom，和surface不一定匹配，因为surface按照realzoom生成，未计算actualzoom
			picpixel.x = (LONG)((surf2cursor.x + 0.5f) / surfzoom);
			picpixel.y = (LONG)((surf2cursor.y + 0.5f) / surfzoom);

			// 获取所处像素颜色
			if (cursorpos != CURSORPOS_PIC)//所处像素超出图片范围获鼠标超出窗口范围
			{
				picpixelcolor = 0;
				pixelcolorvalid = false;
			}
			else
				pixelcolorvalid = pbmp->GetPixel(picpixel.x, picpixel.y, &picpixelcolor);

			// 更新信息字符串
			SetInfoStr();
		}
	}//获取当前鼠标位置对应图片的像素
	inline char *GetSampleSchemaStr()
	{
		switch (sampleschema)
		{
		case SAMPLE_SCHEMA_UNKNOWN:
			return "unknown";
			break;
		case SAMPLE_SCHEMA_NN:
			return "nn";
			break;
		case SAMPLE_SCHEMA_BILINEAR:
			return "bi linear";
			break;
		case SAMPLE_SCHEMA_CUBE:
			return "cube";
			break;
		case SAMPLE_SCHEMA_SINGLE:
			return "single";
			break;
		case SAMPLE_SCHEMA_MAX:
			return "max";
			break;
		default:
			return "unknown";
			break;
		}
	}
};