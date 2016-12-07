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

//透明背景效果
#define TRANSPARENTBACK_SQUAREWIDTH		7
#define TRANSPARENTBACK_HOLLOWCOLOR		D3DCOLOR_XRGB(255, 255, 255)
#define TRANSPARENTBACK_FILLCOLOR		D3DCOLOR_XRGB(204, 203, 204)
#define TRANSPARENTBACK_HOLLOWDENSITY	255
#define TRANSPARENTBACK_FILLDENSITY		230

//最大采样适用图片尺寸上限
#define MAXSAMPLE_WIDTH_UPPERLIMIT		2000
#define MAXSAMPLE_HEIGHT_UPPERLIMIT		MAXSAMPLE_WIDTH_UPPERLIMIT

#define RGB2GRAY(R, G, B)				((R)*38 + (G)*75 + (B)*15) >> 7

#define SAFE_RELEASE(p)					{ if (p) { (p)->Release(); (p)=NULL; } }
#define SAFE_DELETE_LIST(p)				{ if (p) { delete[] (p); (p)=NULL;} }
#define DELETE_LIST(p)					{ delete[] (p); (p)=NULL; }
#define FAILED_RETURN(hr)				{ if (FAILED(hr)) return false; }
#define FALSE_RETURN(r)					{ if (!(r)) return false;}

#define _CDECL							__cdecl
#define _STDCALL						__stdcall
#define _FASTCALL						__fastcall
#define MYCALL1							_CDECL

//图像缩放
#define MAX_ZOOM						200
#define MAX_CONTROLEDZOOM				0.1f
#define MIN_ZOOM						0.001f
#define MIN_DELTAZOOM					0.001f

#define DELTAZOOMNORMAL					0.2f
#define DELTAZOOM1						0.002f
#define DELTAZOOM2						0.002f

#define ZOOMOUT_SHRINK					0.82f

//图像控制
#define MIN_FORCECLIP_WIDTH				2000
#define MIN_FORCECLIP_HEIGHT			MIN_FORCECLIP_WIDTH

#define MIN_FORCECLIP_WIDTH_DRAG		3000
#define MIN_FORCECLIP_HEIGHT_DRAG		MIN_FORCECLIP_WIDTH_DRAG

//鼠标位置
#define CURSORPOS_PIC					0
#define CURSORPOS_BLANK					1
#define CURSORPOS_OUTWINDOW				2

#define BACKCOLOR_INIT					0xFF888888

using std::string;
using std::bad_alloc;
using std::endl;
using std::cerr;

typedef float	COLORTYPE;

//各式四舍五入 & 取整
inline float MYCALL1 roundbf2f(float f)
{
	if (f > 0)
		return (float)(byte)(f + 0.5f);
	else
		return (float)(byte)(f - 0.5);
}
inline byte MYCALL1 roundf2b(float f)
{
	if (f > 0)
		return (byte)(f + 0.5f);
	else
		return (byte)(f - 0.5f);
}
inline LONG MYCALL1 roundf2l(float f)
{
	if (f > 0)
		return (LONG)(f + 0.5f);
	else
		return (LONG)(f - 0.5f);
}
inline int MYCALL1 ceilf2i(float f)
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

	COLOR_F3() { r = g = b = 0; }
	COLOR_F3(COLORTYPE f) { r = g = b = f; }

	COLOR_F3& operator=(COLOR_F3& cf) {
		a = cf.a;

		r = cf.r;
		g = cf.g;
		b = cf.b;
	}
	COLOR_F3& operator=(const COLORTYPE &f) {
		a = r = g = b = f;
		return *this;
	}
	inline void Add(DWORD dw, const COLORTYPE &mul = 1.0, const COLORTYPE &div = 1.0) {
		//byte alpha = (dw >> 24) & 0xFF;
		a += ((dw >> 24) & 0xFF) *mul / div;
		r += (((dw >> 16) & 0xFF)) *mul / div;
		g += ((dw >> 8) & 0xFF) *mul / div;
		b += (dw & 0xFF) *mul / div;
	}
	void Normalize()
	{
		if (a > 255.0)
			a = 255.0;
		if (r > 255.0)
			r = 255.0;
		if (g > 255.0)
			g = 255.0;
		if (b > 255.0)
			b = 255.0;

		a = roundbf2f(a);
		r = roundbf2f(r);
		g = roundbf2f(g);
		b = roundbf2f(b);
	}

	inline DWORD GetDW()
	{
		DWORD res = 0;
		res += roundf2b(a) << 24;
		res += roundf2b(r) << 16;
		res += roundf2b(g) << 8;
		res += roundf2b(b);

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

	//操作
	void SetAlpha(bool alpha);
	bool Load(LPDIRECT3DSURFACE9 surf);			//从surface生成对应大小图片
	bool UpLoad(LPDIRECT3DSURFACE9 surf);		//拷贝到surface，从{0,0}对齐开始，尽可能多的拷贝

	//
	//对于已定制的surface的拷贝
	//
	/***************************************************
	目标地址空间，每行pitch，目标地址尺寸
	surfbias:surf相对原图偏移（用于原图选区）
	backbase:surf相对client窗口（用于辅助透明背景绘制）

	双线性插值的两种策略（有色差，无色差）在Copy_Single中详细定义
	****************************************************/
	//最大无损失采样（缩小效果较好，缩小速度最慢）
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

		//设置新像素起点值数组,限定需要的区间
		float actualzx = (float)width / zoomw;
		//float actualzy = (float)height / zoomh;
		for (int i = 0; i < surfsize.x + 1; i++)
		{
			xbase[i] = (COLORTYPE)(i + surfbias.x)*actualzx;
			xincolor1[i] = ((backbase.x + i) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01;
		}
		for (int i = 0; i < surfsize.y + 1; i++)
			ybase[i] = (COLORTYPE)(i + surfbias.y)*height / zoomh;//height / zoomh

		int index;
		COLOR_F3 rcolor = 0;//颜色累加器
		for (int i = 0; i < surfsize.y; i++)
		{
			int starty = (int)ybase[i];//新像素左上起始点，y方向
			int endy = ceilf2i(ybase[i + 1]);//新像素左上结束点，y方向

			bool yincolor1 = ((backbase.y + i) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01;

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

				//条件alphablend方法
				if (rcolor.a < 255.0f)
				{
					if (yincolor1 ^ xincolor1[j])
					{//两种背景色
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
	//NN和bilinear结合，跨交界处像素使用bilinear（优化的马赛克版本放大，缩小近似于bilinear）
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
			xincolor1[j] = ((backbase.x + j) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01;
		}
		if (alphaon)//有alpha通道
		{
			for (int i = 0; i < surfsize.y; i++)
			{
				//y方向计算
				float fronty = (i + surfbias.y + 0.5f) / zoom - 0.5f;
				float frontybias = fronty - (int)fronty;
				if (frontybias < auxfront)
					frontybias = 0;
				else if (frontybias > auxrear)
					frontybias = 1;
				else
					frontybias = (frontybias - auxfront) * zoom;
				float rearybias = 1 - frontybias;

				int fy = (int)fronty;
				int ry = fy + 1;
				if (ry > height - 1)
					ry -= 1;
				/*if (fronty < 0)
				frontybias = 0;*/
				DWORD *datafy = (DWORD*)data[fy];
				DWORD *datary = (DWORD*)data[ry];

				bool yincolor1 = ((backbase.y + i) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01;

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
						memcpy_s(pixel, 4
							, ((byte*)&((DWORD*)data[(int)(fronty + frontybias)])[(int)(xsto[j] + frontxbias)]), 4);
					}
					else
					{
						//对应原图周围4像素x下标
						int fx = (int)xsto[j];
						int rx = fx + 1;
						if (rx > width - 1)//修正，防越界
							rx -= 1;
						//对应原图周围4像素
						byte *lt, *rt, *lb, *rb;
						lt = (byte*)&datafy[fx];
						rt = (byte*)&datafy[rx];
						lb = (byte*)&datary[fx];
						rb = (byte*)&datary[rx];

						//2次插值
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
						//--#3--加速尝试，略快
						float auxrxry = rxb*ryb, auxfxry = fxb*ryb, auxrxfy = rxb2*fyb, auxfxfy = fxb2*fyb;
						pixel[0] = (byte)(lt[0] * auxrxry + rt[0] * auxfxry
							+ lb[0] * auxrxfy + rb[0] * auxfxfy);
						pixel[1] = (byte)(lt[1] * auxrxry + rt[1] * auxfxry
							+ lb[1] * auxrxfy + rb[1] * auxfxfy);
						pixel[2] = (byte)(lt[2] * auxrxry + rt[2] * auxfxry
							+ lb[2] * auxrxfy + rb[2] * auxfxfy);

						//alpha只根据位置比例分配
						pixel[3] = (byte)(((lt[3] * rearxbias + rt[3] * frontxbias) * rearybias
							+ (lb[3] * rearxbias + rb[3] * frontxbias) * frontybias));
					}
					//bmppixel = *((DWORD*)&pixel);//可能效率降低

					//条件alphablend方法
					if (pixel[3] < (byte)255)
					{
						if (yincolor1 ^ xincolor1[j])
						{//两种背景色
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
				//y方向运算
				float fronty = (i + surfbias.y + 0.5f) / zoom - 0.5f;
				float frontybias = fronty - (int)fronty;
				/*if (fronty < 0)
				frontybias = 0;*/
				if (frontybias <  auxfront)
					frontybias = 0;
				else if (frontybias > auxrear)
					frontybias = 1;
				else
					frontybias = (frontybias - auxfront) * zoom;
				float rearybias = 1 - frontybias;

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
						memcpy_s(pixel, 4
							, ((byte*)&((DWORD*)data[(int)(fronty + frontybias)])[(int)(frontx + frontxbias)]), 4);
					}
					else
					{
						byte *lt, *rt, *lb, *rb;
						//对应原图周围4像素下标
						int fx = (int)frontx;
						int rx = fx + 1;
						if (rx > width - 1)//修正，防越界
							rx -= 1;
						//对应原图周围4像素
						lt = (byte*)&datafy[fx];
						rt = (byte*)&datafy[rx];
						lb = (byte*)&datary[fx];
						rb = (byte*)&datary[rx];

						//2次插值-不去色差法
						//--#1--不去色差法，rgb值仅根据位置比例分配
						pixel[0] = (byte)(((lt[0] * rearxbias + rt[0] * frontxbias) * rearybias
							+ (lb[0] * rearxbias + rb[0] * frontxbias) * frontybias));
						pixel[1] = (byte)(((lt[1] * rearxbias + rt[1] * frontxbias) * rearybias
							+ (lb[1] * rearxbias + rb[1] * frontxbias) * frontybias));
						pixel[2] = (byte)(((lt[2] * rearxbias + rt[2] * frontxbias) * rearybias
							+ (lb[2] * rearxbias + rb[2] * frontxbias) * frontybias));
						pixel[3] = (byte)(((lt[3] * rearxbias + rt[3] * frontxbias) * rearybias
							+ (lb[3] * rearxbias + rb[3] * frontxbias) * frontybias));
					}

					surfData[i*pitch + j] = *((DWORD*)&pixel);
				}
			}
		}
		delete[] xsto;
		delete[] xincolor1;

		return true;
	}
	//最近邻近（放大缩小都可以，效果不会太好）
	inline bool MYCALL1 Copy_NN(DWORD *surfData, int pitch, const POINT &surfsize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 }) 
	{
		int index;

		int *bxarray = new int[surfsize.x];
		bool *xincolor1 = new bool[surfsize.x];
		for (int j = 0; j < surfsize.x; j++)//加速
		{
			bxarray[j] = (int)(min((j + surfbias.x + 0.5f) / zoom, width - 1));
			xincolor1[j] = ((backbase.x + j) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01;
		}
		if (alphaon)
		{
			for (int i = 0; i < surfsize.y; i++)
			{
				int by = (int)(min((i + surfbias.y + 0.5f) / zoom, height - 1));
				DWORD *databy = (DWORD*)data[by];

				bool yincolor1 = ((backbase.y + i) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01;

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
	//双线性（缩小过多或放大过多效果差）
	inline bool MYCALL1 Copy_BiLinear(DWORD *surfData, int pitch, const POINT &surfsize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 }) 
	{
		int index;

		float *xsto = new float[surfsize.x];//加速,防止处于二层循环的x重复计算
		bool *xincolor1 = new bool[surfsize.x];
		for (int j = 0; j < surfsize.x; j++)//提前运算x方向的变量，终极加速
		{
			xsto[j] = (j + surfbias.x + 0.5f) / zoom - 0.5f;
			xincolor1[j] = ((backbase.x + j) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01;
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

				bool yincolor1 = ((backbase.y + i) / TRANSPARENTBACK_SQUAREWIDTH) & 0x01;

				for (int j = 0; j < surfsize.x; j++)
				{
					index = i*pitch + j;

					//对应原图像素位置
					float frontx = xsto[j];
					float frontxbias = frontx - (int)frontx;
					/*if (frontx < 0)
					frontxbias = 0;*/
					float rearxbias = 1 - frontxbias;

					byte *lt, *rt, *lb, *rb;
					//对应原图周围4像素下标
					int fx = (int)frontx;
					int rx = fx + 1;
					if (rx > width - 1)//修正，防越界
						rx -= 1;
					//对应原图周围4像素
					lt = (byte*)&datafy[fx];
					rt = (byte*)&datafy[rx];
					lb = (byte*)&datary[fx];
					rb = (byte*)&datary[rx];

					//2次插值-无色差法
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
					//--#3--加速尝试，略快
					byte pixel[4];
					float auxrxry = rxb*ryb, auxfxry = fxb*ryb, auxrxfy = rxb2*fyb, auxfxfy = fxb2*fyb;
					pixel[0] = (byte)(lt[0] * auxrxry + rt[0] * auxfxry
						+ lb[0] * auxrxfy + rb[0] * auxfxfy);
					pixel[1] = (byte)(lt[1] * auxrxry + rt[1] * auxfxry
						+ lb[1] * auxrxfy + rb[1] * auxfxfy);
					pixel[2] = (byte)(lt[2] * auxrxry + rt[2] * auxfxry
						+ lb[2] * auxrxfy + rb[2] * auxfxfy);

					//alpha只根据位置比例分配
					pixel[3] = (byte)(((lt[3] * rearxbias + rt[3] * frontxbias) * rearybias
						+ (lb[3] * rearxbias + rb[3] * frontxbias) * frontybias));

					//条件alphablend方法
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
				/*if (fronty < 0)
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
					//对应原图像素位置
					float frontx = xsto[j];
					float frontxbias = frontx - (int)frontx;
					/*if (frontx < 0)
					frontxbias = 0;*/
					float rearxbias = 1 - frontxbias;

					byte *lt, *rt, *lb, *rb;
					//对应原图周围4像素下标
					int fx = (int)frontx;
					int rx = fx + 1;
					if (rx > width - 1)//修正，防越界
						rx -= 1;
					//对应原图周围4像素
					lt = (byte*)&datafy[fx];
					rt = (byte*)&datafy[rx];
					lb = (byte*)&datary[fx];
					rb = (byte*)&datary[rx];

					//2次插值-不去色差法
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
					surfData[i*pitch + j] = *((DWORD*)&pixel);
				}
			}
		}
		delete[] xsto;
		delete[] xincolor1;

		return true;
	}

	//
	//图片处理
	//
	bool MYCALL1 Gray();

	bool MYCALL1 Inverse();
	bool MYCALL1 InverseAlpha();
	bool MYCALL1 InverseAll();

	bool MYCALL1 LOSE_R();
	bool MYCALL1 LOSE_G();
	bool MYCALL1 LOSE_B();

	//信息
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

	//状态
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

struct Surfer {
	LPDIRECT3DSURFACE9 surf;				//surface
	LPDIRECT3DDEVICE9 pdev;					//设备指针
	BMP *pbmp;								//BMP指针
	int *pbmpw, *pbmph;						//BMP尺寸
	UINT *pbufw, *pbufh;					//目标surface尺寸

	int surfrefreshcount;					//surface更新计数
	float renewtime;

	LARGE_INTEGER wheeltick, lastwheeltick;	//滚轮计时
	LARGE_INTEGER freq;						//频率

	/******************************************************************************
	surfsrc是自由变量，是BMP显示到目标SURFACE的主键，和"选项"共同作用。逻辑上可分离，作为属于的BMP信息。
	其他变量内部维护
	surfsrc和bmp（或clip,zoom）迁移过来后，在SurfRenew之前，需更新:【zoomw，zoomh或oldsurfzoom】&信息
	*******************************************************************************/

	//"BMP" 映射 "SURFACE" 选项
	bool clip;								//surface是否clip（截去客户区外区域）
	D3DCOLOR backcolor;						//背景色（暂时没用）
	//"BMP" 映射 "SURFACE" 参数
	float surfzoom;							//直接操控的放大倍数
	float oldsurfzoom;						//上一个放大倍数
	int zoomw, zoomh;						//放大surfzoom倍后假想surface的尺寸，【减少临时运算量】

	//"BMP" 映射 "SURFACE" 状态
	float actualzoomx, actualzoomy;			//实际x,y方向上的放大倍数【这个标志暂时只用来查看】
	bool surffailed;						//surface是否创建失败，在有mainbmp情况下【这个标志暂时只用来查看】
	bool surfclipped;						//当前surface是否clip

	//"源SURFACE" 映射 "目标SURFACE" 参数
	POINT surfsrc;							//放大surfzoom倍的假想surface的左上角到窗口客户区的向量
	POINT basepoint;						//用于dragzoom的中心点
	//"源SURFACE" 映射 "目标SURFACE" 直接参数
	POINT surfbase;							//surface拷贝到backbuffer, backbuffer的起始点
	RECT surfrect;							//surface要拷贝到backbuffer的区域

	//"源SURFACE" 映射 "目标SURFACE" 状态
	POINT surfsize;							//surface尺寸
	bool picclipped;						//当前图片是否clip
	bool outsideclient;						//surface和窗口客户区是否无交集（在客户区外）
//信息
	POINT picpixel;							//鼠标当前位置对应原图的像素
	DWORD picpixelcolor;					//鼠标当前位置对应原图的颜色
	UINT8 cursorpos;						//当前鼠标位置（种类，处于哪个区域：图片、空白、窗口外）

public:
	Surfer();

	bool BindDevice(LPDIRECT3DDEVICE9 device);//绑定设备
	bool BindBuf(UINT * bufw, UINT *bufh);	//绑定目标surface尺寸
	void SetBackcolor(D3DCOLOR bc);			

	bool BindBMP(BMP *bmp);
	void Refresh();

	//WCHAR *GetPicInfoStrW();											//更新图像信息字符串并返回
	//bool LoadFile(WCHAR []);
	//bool SaveFile(WCHAR []);
	
//surface定位操作
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
	//设置surface缩放倍率	
	inline void SurfAdjustZoom_normal1(int wParam)
	{
		float adds = 0;
		QueryPerformanceCounter(&wheeltick);

		adds = (float)(/*freq.QuadPart / (wheeltick.QuadPart - lastwheeltick.QuadPart)
			**/(DELTAZOOMNORMAL*surfzoom));//增长速度与时间和放大倍率有关
		lastwheeltick.QuadPart = wheeltick.QuadPart;
		//adds限制
		if (adds < MIN_DELTAZOOM)
			adds = MIN_DELTAZOOM;
		adds *= wParam / 120.0f;
		if (adds < 0)
			adds *= ZOOMOUT_SHRINK;

		surfzoom += adds;
		//zoom上下限
		if (surfzoom > MAX_ZOOM)
			surfzoom = MAX_ZOOM;
		if (surfzoom*min(*pbmpw, *pbmph) < 1)
			surfzoom = 1.00001f / min(*pbmpw, *pbmph);
	}
	inline void SurfAdjustZoom_wheel(int wParam)
	{
		float adds = 0;
		QueryPerformanceCounter(&wheeltick);

		if (surfzoom < 1)
			//adds = (float)(frequency.QuadPart / (zoomtick.QuadPart - lastwheeltick.QuadPart)
			//	*(DELTAZOOM1*zoom));//增长速度与时间和放大倍率有关
			adds = (float)(freq.QuadPart / (wheeltick.QuadPart - lastwheeltick.QuadPart)
				*(DELTAZOOM1*surfzoom));//增长速度与时间和放大倍率有关
		else
			adds = (float)(freq.QuadPart / (wheeltick.QuadPart - lastwheeltick.QuadPart)
				*(DELTAZOOM2*surfzoom));//增长速度与时间和放大倍率有关
		lastwheeltick.QuadPart = wheeltick.QuadPart;
		//adds限制
		if (adds < MIN_DELTAZOOM)
			adds = MIN_DELTAZOOM;
		//if ((short)HIWORD(wParam) < 0)
		//	adds = -adds;//adds取反
		adds *= wParam / 120.0f;
		if (adds < 0)
			adds *= 0.8f;//放大缩小一致化

		surfzoom += adds;
		//zoom上下限
		if (surfzoom > MAX_ZOOM)
			surfzoom = MAX_ZOOM;
		if (surfzoom*min(*pbmpw, *pbmph) < 1)
			surfzoom = 1.00001f / min(*pbmpw, *pbmph);
	}
	//根据调整值调整缩放倍率	
	inline void SurfZoomRenew(POINT client2cursor, bool stable = false, bool acce = false)
	{
		int oldzoomw = zoomw, oldzoomh = zoomh;
		zoomw = (int)(surfzoom**pbmpw);
		zoomh = (int)(surfzoom**pbmph);

		actualzoomx = (float)zoomw / *pbmpw;
		actualzoomy = (float)zoomh / *pbmph;

		if (oldsurfzoom == surfzoom)
			return;

		UINT oldswidth = (UINT)(*pbmpw*oldsurfzoom)
			, oldsheight = (UINT)(*pbmph*oldsurfzoom);

		//调整起始点#1
		short altstate = GetAsyncKeyState(VK_MENU);
		if ((altstate & 0x8000) || stable)
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

		//设置clip标志
		clip = (surfzoom >= 1)
			|| (surfzoom < 1 && (zoomw > MIN_FORCECLIP_WIDTH || zoomh > MIN_FORCECLIP_HEIGHT));

		SurfRenew(acce);

		oldsurfzoom = surfzoom;
	}

	inline bool SurfRenew(bool acce = false)
	{
		LARGE_INTEGER start, end;
		QueryPerformanceCounter(&start);

		surfrefreshcount++;

		outsideclient = (
			surfsrc.x > zoomw || surfsrc.y > zoomh	//区域超出surface右或下
			|| surfsrc.x + (int)*pbufw <= 0			//区域右下不够surface左或上
			|| surfsrc.y + (int)*pbufh <= 0
			);

		surfsize.x = (LONG)zoomw;
		surfsize.y = (LONG)zoomh;
		POINT surfbias = { 0, 0 };	//实际surface起点（左上角）相对假想surface的位置（偏移用于定位图片区域来设置surface像素）
		surfclipped = false;
		if (clip)
		{
			//计算surfsize	 ：clip后的surface尺寸,
			//计算surfbias	 ：clip后的surface相对于假想surface的偏移
			//计算surfclipped ：实际surface是否clip
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
			if ((LONG)zoomw - surfsrc.x > (int)*pbufw)
			{
				surfsize.x -= zoomw - surfsrc.x - (int)*pbufw;

				surfclipped = true;
			}
			//surface下侧需clip
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

		//生成surface
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


			//清除原表面
			SAFE_RELEASE(surf);
			//新建surface
			if (FAILED(pdev->CreateOffscreenPlainSurface(
				surfsize.x, surfsize.y
				, D3DFMT_A8R8G8B8
				, D3DPOOL_SYSTEMMEM
				, &surf
				, NULL)))
				return false;

			//锁定surface区域
			D3DLOCKED_RECT lockedRect;
			surf->LockRect(&lockedRect, NULL, NULL);
			DWORD *surfData = (DWORD*)lockedRect.pBits;

			//拷贝
			if (acce)
			{
				pbmp->Copy_NN(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
				/*if(surfzoom>2)
					pbmp->Copy_NN(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
				else
					pbmp->Copy_Single(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);*/
			}
			else
			{
				if (surfzoom > 2)
				{
					pbmp->Copy_NN(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
				}
				else if (surfzoom >= 1)
				{
					//小平滑马赛克
					pbmp->Copy_Single(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
				}
				else if (surfzoom > 0.5f/* || pbmp->width > MAXSAMPLE_WIDTH_UPPERLIMIT || pbmp->height > MAXSAMPLE_HEIGHT_UPPERLIMIT*/)
				{
					//双线性
					//pbmp->Copy_BiLinear(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
					pbmp->Copy_BiLinear(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
				}
				else
				{
					//最大重采样
					pbmp->Copy_MaxSample(surfData, lockedRect.Pitch / 4, surfsize, surfzoom, 0, surfbias, backbase);
				}
			}
			surf->UnlockRect();
		}


		if (surf)
		{
			surffailed = false;

			//renew后续信息获取
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
	inline void SurfCalcMapInfo()
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
	inline bool OnDrag_Custom(POINT cursorbias)
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
			if (zoomw <= MIN_FORCECLIP_WIDTH_DRAG && zoomh <= MIN_FORCECLIP_HEIGHT_DRAG)
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
	inline void OnDrag_Custom2(POINT cursorbias)
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
			if (zoomw <= MIN_FORCECLIP_WIDTH_DRAG && zoomh <= MIN_FORCECLIP_HEIGHT_DRAG)
			{
				clip = false;
				if (surfclipped)
				{
					SurfRenew(false);
					renew = true;
				}
			}
			//如果图片始终全部在窗口客户区范围内（前后surfclipped状态都是0）
			//或者始终在客户区外（前后outsideclient状态都是1），或者不设置clip标志，就不更新
			else if ((lastpicclipped || picclipped)
				&& !outsideclient)
			{
				SurfRenew(false);
				renew = true;
			}
		}
		//本次没更新surface，则更新surface映射信息，否则surface不动
		if (!renew)
		{
			SurfCalcMapInfo();//更新surface拷贝到backbuffer参数
		}
	}
	inline bool OnWinsize_Custom()//和ondrag差不多
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
			if (zoomw <= MIN_FORCECLIP_WIDTH_DRAG && zoomh <= MIN_FORCECLIP_HEIGHT_DRAG)
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

	//渲染
	inline void Render()
	{
		if (surf && !outsideclient)
		{
			//图片存在且surface与窗口客户区有交集才渲染
			//获得backbuffer
			LPDIRECT3DSURFACE9 backbuffer = NULL;
			HRESULT hr = pdev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
			if (FAILED(hr))
				return;
			//拷贝到backbuffer
			hr = pdev->UpdateSurface(surf, &surfrect, backbuffer, &surfbase);//统一的拷贝

			SAFE_RELEASE(backbuffer);
		}
	}

	//清除
	void Clear();

//信息
	inline void GetCurInfo(POINT cursor, RECT clientrect)
	{
		//获得当前鼠标位置（种类）
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

		//所处像素
		//如果除以actualzoom，和surface不一定匹配，因为surface按照realzoom生成，未计算actualzoom
		picpixel.x = (LONG)((surf2cursor.x + 0.5f) / surfzoom);
		picpixel.y = (LONG)((surf2cursor.y + 0.5f) / surfzoom);

		//获取所处像素颜色
		if (cursorpos != CURSORPOS_PIC)//所处像素超出图片范围获鼠标超出窗口范围
		{
			picpixelcolor = 0;
		}
		else
			pbmp->GetPixel(picpixel.x, picpixel.y, &picpixelcolor);
	}//获取当前鼠标位置对应图片的像素
};

string GetFMTStr(D3DFORMAT);

class PicPack {
public:
	BMP bmp;
	PicInfo myimginfo;				//图片信息
	D3DXIMAGE_INFO d3dimginfo;		//自定图片信息

	POINT src;

	PicPack() {
		ZeroMemory(&myimginfo, sizeof(myimginfo));
		ZeroMemory(&d3dimginfo, sizeof(d3dimginfo));
		src = { 0,0 };
	}

	inline bool isEmpty()
	{
		return bmp.isEmpty();
	}

	bool LoadFile(LPDIRECT3DDEVICE9 pdev, WCHAR[]);
	bool SaveFile(LPDIRECT3DDEVICE9 pdev, WCHAR[]);

	WCHAR *GetPicInfoStrW();
};