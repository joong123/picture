#pragma once 

#include<new>
#include<iostream>
#include<stdlib.h>

#include <d3d9.h>
#include <d3dx9tex.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")


using namespace std;

typedef float COLORTYPE;

//各式四舍五入
UINT16 roundf2ui16(float f);
DWORD roundf2d(float f);
LONG roundf2l(float f);
float roundbf2f(float f);
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
	void Add(DWORD dw, const COLORTYPE &mul = 1.0, const COLORTYPE &div = 1.0) {
		a += ((dw >> 24) & 0xFF) *mul / div;
		r += ((dw >> 16) & 0xFF) *mul / div;
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
		, short rotate = 0
		);
	bool GetPixel(int x, int y, DWORD *dest);
	void Clear();
	bool Empty();
};
