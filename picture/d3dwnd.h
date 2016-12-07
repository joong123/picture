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

//窗口信息
#define WINDOWPOSX_INIT				180
#define WINDOWPOSY_INIT				100
#define WINDOWWIDTH_INIT			800
#define WINDOWHEIGHT_INIT			520

using std::string;

#define FVF_1PD						(D3DFVF_XYZ | D3DFVF_DIFFUSE)
struct FVF1
{
	D3DXVECTOR3 pos;		//顶点位置
	DWORD color;
};

#define FVF_2CPD					(D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
struct FVF2
{
	//float x, y, z, rhw;
	D3DXVECTOR4 pos;     //顶点位置
	DWORD color;
};

class D3DWnd {
private:
	RECT clientrect;
	RECT windowrect;

	HWND hwnd;
	HINSTANCE hinst;
private:
	LPDIRECT3D9	lpD3D;
	D3DPRESENT_PARAMETERS d3dpp;
	LPDIRECT3DDEVICE9 device;
	D3DDISPLAYMODE displaymode;				//显示模式
	D3DCAPS9 caps;							//设备能力
	int vertexprocessing;					//vertexprocessing方式
public:
	D3DWnd() {
		hwnd = NULL;
		hinst = NULL;

		lpD3D = NULL;
		memset(&d3dpp, 0, sizeof(d3dpp));
		device = NULL;
		memset(&displaymode, 0, sizeof(displaymode));
		memset(&caps, 0, sizeof(caps));
	}
//set callback

//window
	bool D3DCreateWindow(WCHAR *lpWindowName
		, WNDPROC wndproc, HINSTANCE hInstance = NULL
		, DWORD ClassStyle = 0, DWORD Style = 0, DWORD ExStyle = 0
		, int x = WINDOWPOSX_INIT, int y = WINDOWPOSY_INIT
		, int width = WINDOWWIDTH_INIT, int height = WINDOWHEIGHT_INIT
		, HICON hIcon = NULL, HICON hIconsm = NULL//unimportant param
		, WCHAR *lpMenuName = NULL
		, WCHAR *szWindowClass = NULL
		, D3DCOLOR backcolor = D3DCOLOR_XRGB(250, 250, 250)
	);
	void SetHWND(HWND hwnd);
	HWND GetHWND();
	inline void Get2WndRect();
//D3D
	bool CreateDevice(D3DFORMAT format = D3DFMT_A8R8G8B8, UINT backbuffercount = 1);
	bool D3DCreateFont(LPD3DXFONT *lpf
		, WCHAR *fontName
		, INT Height = 12, INT Width = 0, INT Weight = 0
		, DWORD Quality = DEFAULT_QUALITY//交换了次序
		, UINT MipLevels = 1, bool Italics = false
		, DWORD Charset = DEFAULT_CHARSET, DWORD OutputPrecision = OUT_DEFAULT_PRECIS
		, DWORD PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE
	);
	bool CreateMeshFVF(LPD3DXMESH *ppmesh, void *vertice, void *index, UINT16 sizev, UINT16 sizei, DWORD numfaces, DWORD numvertices
		,DWORD options);
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

		d3dpp.BackBufferWidth = WIDTHOF(clientrect);
		d3dpp.BackBufferHeight = HEIGHTOF(clientrect);

		if (FAILED(device->Reset(&d3dpp)))
		{
			return false;
		}

		return true;
	}
	//inline void Render();
//信息
	void DisplayAdapter();
//辅助
	string GetFMTStr(D3DFORMAT);				//获得图片格式字符串
};

static char * print_guid(GUID id) {
	char * str = (char *)malloc(39);
	sprintf_s(str, 39, "{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		id.Data1, id.Data2, id.Data3,
		id.Data4[0], id.Data4[1], id.Data4[2], id.Data4[3],
		id.Data4[4], id.Data4[5], id.Data4[6], id.Data4[7]);
	return str;
}