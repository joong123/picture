#pragma once

#include "stdio.h"
#include <string>

//local include
#include "generalgeo.h"

//D3D include
#include <d3d9.h>
#include <d3dx9tex.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

//命令
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif
#define FAILED_RETURN_FALSE(hr)			{ if (FAILED(hr)) return false; }

//窗口信息
#define D3DWINDOWPOSX_INIT				CW_USEDEFAULT
#define D3DWINDOWPOSY_INIT				100
#define D3DWINDOWWIDTH_INIT				656
#define D3DWINDOWHEIGHT_INIT			518

using std::string;

#define FVF_1PD						( D3DFVF_XYZ | D3DFVF_DIFFUSE )
struct FVF1
{
	D3DXVECTOR3 pos;		//顶点位置
	DWORD color;
};

#define FVF_2CPD					( D3DFVF_XYZRHW | D3DFVF_DIFFUSE )
struct FVF2
{
	float x, y, z, rhw;		//顶点位置
	DWORD color;
};

#define FVF_3PDN					( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE )
struct FVF3
{
	D3DXVECTOR3 pos;		//顶点位置
	D3DXVECTOR3 normal;
	DWORD color;
};

class D3DWnd {
private:
	RECT rcClient;
	RECT windowrect;

	HWND hWnd;
	HINSTANCE hinst;
private:
	LPDIRECT3D9	lpD3D;
	D3DPRESENT_PARAMETERS d3dpp;
	LPDIRECT3DDEVICE9 device;
	D3DDISPLAYMODE displaymode;				//显示模式
	D3DCAPS9 caps;							//设备能力
	D3DMULTISAMPLE_TYPE mst;
	int vertexprocessing;					//vertexprocessing方式
public:
	D3DWnd() {
		hWnd = NULL;
		hinst = NULL;

		lpD3D = NULL;
		memset(&d3dpp, 0, sizeof(d3dpp));
		device = NULL;
		memset(&displaymode, 0, sizeof(displaymode));
		memset(&caps, 0, sizeof(caps));
	}
//callback

//window
	bool D3DCreateWindow(WCHAR *lpWindowName
		, WNDPROC wndproc, HINSTANCE hInstance = NULL
		, DWORD ClassStyle = 0, DWORD Style = 0, DWORD ExStyle = 0
		, int x = D3DWINDOWPOSX_INIT, int y = D3DWINDOWPOSY_INIT
		, int width = D3DWINDOWWIDTH_INIT, int height = D3DWINDOWHEIGHT_INIT
		, HICON hIcon = NULL, HICON hIconsm = NULL//unimportant param
		, WCHAR *lpMenuName = NULL
		, WCHAR *szWindowClass = NULL
		, D3DCOLOR BackgroundColor = D3DCOLOR_XRGB(250, 250, 250)
	);
	void SetHWND(HWND hWnd);
	HWND GetHWND();
	inline void Get2WndRect();
//D3D
	bool CreateDevice(D3DFORMAT textFormat = D3DFMT_A8R8G8B8, UINT backbuffercount = 1);
	bool DXCreateFont(LPD3DXFONT *lpf
		, WCHAR *fontName
		, INT Height = 12, INT Width = 0, INT Weight = 0
		, DWORD Quality = DEFAULT_QUALITY//交换了次序
		, UINT MipLevels = 1, bool Italics = false
		, DWORD Charset = DEFAULT_CHARSET, DWORD OutputPrecision = OUT_DEFAULT_PRECIS
		, DWORD PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE
	);
	bool CreateMeshFVF(LPD3DXMESH *ppmesh, void *vertice, void *index, UINT16 sizev, UINT16 sizei, DWORD numfaces, DWORD numvertices
		, DWORD options);
	static bool ChangeMeshColor_tail(LPD3DXMESH *ppmesh, D3DCOLOR color);
	static bool RefreshVertexBuffer(LPDIRECT3DVERTEXBUFFER9 *vb, void *vertice);
	bool ChangeVBColor_tail(LPDIRECT3DVERTEXBUFFER9 *vb, D3DCOLOR color, UINT16 sizev, int nums = -1);
	bool CreateVertexBuffer_Custom1(LPDIRECT3DVERTEXBUFFER9 *vb, int x = 0, int y = 0, int r = 1, D3DCOLOR color = 0);
	bool CreateVertexBuffer_Custom2(LPDIRECT3DVERTEXBUFFER9 *vb, int x = 0, int y = 0, int w = 1, int h = 1, D3DCOLOR color = 0);

	bool CreateMesh_Custom1(LPD3DXMESH *ppmesh);
	void CreateSphere(ID3DXMesh ** obj, int finess, float radius, D3DCOLOR color, float height);


	LPDIRECT3DDEVICE9 GetDevice();
	inline UINT * GetPBufferWidth()
	{
		return &d3dpp.BackBufferWidth;
	}
	inline UINT * GetPBufferHeight()
	{
		return &d3dpp.BackBufferHeight;
	}
	inline bool OnLostDevice()
	{
		if (!device)
			return false;

		return true;
	}
	inline bool OnResetDevice(int clientw, int clienth)
	{
		if (!device || clientw < 1 || clienth < 1)
			return false;

		Get2WndRect();

		d3dpp.BackBufferWidth = clientw;
		d3dpp.BackBufferHeight = clienth;

		if (FAILED(device->Reset(&d3dpp)))
		{
			return false;
		}

		return true;
	}
	inline bool OnResetDevice()
	{
		if (!device)
			return false;

		Get2WndRect();

		d3dpp.BackBufferWidth = WIDTHOF(rcClient);
		d3dpp.BackBufferHeight = HEIGHTOF(rcClient);

		if (FAILED(device->Reset(&d3dpp)))
		{
			return false;
		}

		return true;
	}
	D3DMULTISAMPLE_TYPE GetMultiSample()
	{
		return mst;
	}
	void ChangeMultiSample();
	//inline void Render();
//信息
	void DisplayAdapter();
};

static char * print_guid(GUID ID) {
	char * str = (char *)malloc(39);
	sprintf_s(str, 39, "{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		ID.Data1, ID.Data2, ID.Data3,
		ID.Data4[0], ID.Data4[1], ID.Data4[2], ID.Data4[3],
		ID.Data4[4], ID.Data4[5], ID.Data4[6], ID.Data4[7]);
	return str;
}