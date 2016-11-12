#pragma once 

#include<new>
#include<iostream>
#include<stdlib.h>

#include <d3d9.h>
#include <d3dx9tex.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

#define TRANSPARENTBACK_SQUAREWIDTH		7
#define TRANSPARENTBACK_HOLLOWCOLOR		D3DCOLOR_XRGB(255, 255, 255)
#define TRANSPARENTBACK_FILLCOLOR		D3DCOLOR_XRGB(204, 203, 204)
#define TRANSPARENTBACK_HOLLOWDENSITY	255
#define TRANSPARENTBACK_FILLDENSITY		230

using namespace std;

typedef float COLORTYPE;

//各式四舍五入 & 取整
UINT16 roundf2ui16(float f);
int roundpf2i(float f);
DWORD roundf2d(float f);
LONG roundf2l(float f);
float roundbf2f(float f);
byte roundf2b(float f);
byte roundpf2b(float f);
char roundpf2c(float f);
int ceilf2i(float f);

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
		res += (byte)round(a) << 24;
		res += (byte)round(r) << 16;
		res += (byte)round(g) << 8;
		res += (byte)round(b);

		return res;
	}
	inline DWORD AlphaBlend(DWORD blender) {
		byte alpha = (byte)a;
		DWORD res = 0;
		//res += (byte)round(a) << 24;
		res += roundf2d((r*alpha + ((blender >> 16) & 0xFF)*(255 - alpha)) / 255) << 16;
		res += roundf2d((g*alpha + ((blender >> 8) & 0xFF)*(255 - alpha)) / 255) << 8;
		res += roundf2d((b*alpha + (blender & 0xFF)*(255 - alpha)) / 255);

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
	//示例
	/*static inline DWORD AlphaBlend1(DWORD srccolor) {
		if (srccolor == lastcolor1)
		{
			return lastresult1;
		}
		else
		{
			byte *src = (byte*)&srccolor;
			byte *dest = (byte*)&lastresult1;

			dest[2] = alphablendtable1[src[2]][src[3]];
			dest[1] = alphablendtable1[src[1]][src[3]];
			dest[0] = alphablendtable1[src[0]][src[3]];

			lastcolor1 = srccolor;
		}

		return lastresult1;
	}*/
	static inline void InitAlphBlendTable(byte color1, byte color2) {
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
	static inline void InitAlphBlendTCube() {
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
					alphablendcube1[i][j][k] = (byte)((i*j + (float)(alphalevels - 1 - i)*k) / (alphalevels - 1));
				}
			}
		}
	}

};


class BMP {
private:
	static void __cdecl newhandlerfunc()
	{
		cerr << "Allocate failed in class BMP" << endl;
		abort();
	}

public:
	int width, height;
	byte **data;
	bool alphaon;

	BMP();
	bool Load(LPDIRECT3DSURFACE9 surf);
	bool CreateSurf(LPDIRECT3DDEVICE9 device, LPDIRECT3DSURFACE9 *surf, D3DCOLOR color, int zoom = 1);
	bool CreateSurf_Clip(LPDIRECT3DDEVICE9 device, LPDIRECT3DSURFACE9 *surf
		, POINT surfsrc, POINT clientrectsize, POINT &surfbase, bool &surfclipped
		, float zoom = 1, bool clip = true, D3DCOLOR color = 0xFF000000
		, float rotate = 0
		);

	//目标地址空间，每行pitch，目标地址尺寸
	//surfbias:surf相对原图偏移（用于原图选区）
	//backbase:surf相对client窗口（用于辅助透明背景绘制）

	//最大无损失采样（缩小效果较好，缩小速度最慢）
	inline bool Copy_MaxSample(DWORD *surfData, int pitch, const POINT &surfsize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 });
	//整数放大倍数的马赛克式（无采样，无差值，无失真，仅用于正整数放大倍数）
	inline bool Copy_NoSample(DWORD *surfData, int pitch, const POINT &surfsize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 });
	//NN和bilinear结合，跨交界处像素使用bilinear（优化的马赛克版本放大，缩小近似于bilinear）
	inline bool Copy_Single(DWORD *surfData, int pitch, const POINT &surfsize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 });
	//最近邻近（放大缩小都可以，效果不会太好）
	inline bool Copy_NN(DWORD *surfData, int pitch, const POINT &surfsize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 });
	//双线性（缩小过多或放大过多效果差）
	inline bool Copy_BiLinear(DWORD *surfData, int pitch, const POINT &surfsize
		, float zoom, float rotate = 0, const POINT surfbias = { 0,0 }, const POINT backbase = { 0,0 });


	inline bool GetPixel(int x, int y, DWORD *dest)
	{
		if (!data)
			return false;

		if (!data[y])
			return false;

		if (x >= width || y >= height)
			return false;

		*dest = ((DWORD*)data[y])[x];

		return true;
	}
	void Clear();
	inline bool Empty()
	{
		return !data;
	}
};