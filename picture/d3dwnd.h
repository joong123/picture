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
	RECT rcWindow;

	HWND hWnd;
	HINSTANCE hInst;
private:
	LPDIRECT3D9	lpD3D;
	D3DPRESENT_PARAMETERS d3dParam;
	LPDIRECT3DDEVICE9 device;
	D3DDISPLAYMODE displayMode;				//显示模式
	D3DCAPS9 caps;							//设备能力
	D3DMULTISAMPLE_TYPE mst;
	int vertexprocessing;					//vertexprocessing方式
public:
	D3DWnd() {
		hWnd = NULL;
		hInst = NULL;

		lpD3D = NULL;
		memset(&d3dParam, 0, sizeof(d3dParam));
		device = NULL;
		memset(&displayMode, 0, sizeof(displayMode));
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
		, HWND hWndParent = NULL
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
	static bool RefreshVertexBuffer(LPDIRECT3DVERTEXBUFFER9 *vb, void *vertice);
	
	bool CreateMesh_Custom1(LPD3DXMESH *ppmesh);
	void CreateSphere(ID3DXMesh ** obj, int finess, float radius, D3DCOLOR color, float height);


	LPDIRECT3DDEVICE9 GetDevice();
	inline UINT * GetPBufferWidth()
	{
		return &d3dParam.BackBufferWidth;
	}
	inline UINT * GetPBufferHeight()
	{
		return &d3dParam.BackBufferHeight;
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

		d3dParam.BackBufferWidth = clientw;
		d3dParam.BackBufferHeight = clienth;

		if (FAILED(device->Reset(&d3dParam)))
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

		d3dParam.BackBufferWidth = WIDTHOF(rcClient);
		d3dParam.BackBufferHeight = HEIGHTOF(rcClient);

		if (FAILED(device->Reset(&d3dParam)))
		{
			return false;
		}

		return true;
	}
	inline D3DMULTISAMPLE_TYPE GetMultiSample()
	{
		return mst;
	}
	void ChangeMultiSample();
	//inline void Render();
//信息
	void DisplayAdapter();
};

char* print_guid(GUID ID);