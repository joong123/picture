#include "stdafx.h"
#include "d3dwnd.h"

//windows include
#include "shellapi.h"
#include "winerror.h"

bool D3DWnd::D3DCreateWindow(WCHAR * lpWindowName
	, WNDPROC wndproc, HINSTANCE hInstance
	, DWORD ClassStyle, DWORD Style, DWORD ExStyle
	, int x, int y, int width, int height
	, HICON hIcon, HICON hIconsm
	, WCHAR *lpMenuName
	, WCHAR *szWindowClass
	, D3DCOLOR BackgroundColor)
{
	if (lpWindowName == NULL)
		lpWindowName = L"UNNAME";

	if (lpMenuName == NULL)
		lpMenuName = L"NOMENU";

	if (szWindowClass == NULL)
		szWindowClass = L"D3DWindowClass";

	if (wndproc == NULL)
	{
		MessageBox(NULL, L"No WndProc in D3DWnd::D3DCreateWindow!", L"ERROR", MB_OK | MB_APPLMODAL);
		return false;
	}

	if (hInstance == NULL)
		hInstance = (HINSTANCE)GetModuleHandle(NULL);
	hinst = hInstance;

	WCHAR szExePath[MAX_PATH];
	GetModuleFileName(NULL, szExePath, MAX_PATH);
	if (hIcon == NULL) // If the icon is NULL, then use the first one found in the exe
		hIcon = ExtractIcon(hInstance, szExePath, 0);
	if (hIconsm == NULL)
		hIconsm = ExtractIcon(hInstance, szExePath, 1);

	HBRUSH hb = CreateSolidBrush(RGB(BackgroundColor>>16, (BackgroundColor>>8)&0xFF, BackgroundColor & 0xFF));

	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = ClassStyle;
	wcex.lpfnWndProc = wndproc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = hIcon;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = hb;/*(HBRUSH)(COLOR_WINDOW + 1)*//*(HBRUSH)GetStockObject(BLACK_BRUSH)*/
	wcex.lpszMenuName = lpMenuName;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = hIconsm;
	//return RegisterClassExW(&wcex);

	if (!RegisterClassExW(&wcex))
	{
		DWORD dwError = GetLastError();
		if (dwError != ERROR_CLASS_ALREADY_EXISTS)
		{
			MessageBox(NULL, L"RegisterClass Failed in D3DWnd::D3DCreateWindow!", L"ERROR", MB_OK | MB_APPLMODAL);
		}
		return false;
	}

	hWnd = CreateWindowExW(ExStyle, szWindowClass, lpWindowName, Style,
		x, y, width, height
		, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return true;
}

void D3DWnd::SetHWND(HWND hWnd)
{
	this->hWnd = hWnd;
}

HWND D3DWnd::GetHWND()
{
	return hWnd;
}

inline void D3DWnd::Get2WndRect()
{
	if (!hWnd)
		return;

	GetClientRect(hWnd, &rcClient);//得到client区域尺寸
	POINT clienttl = { 0, 0 };
	ClientToScreen(hWnd, &clienttl);//获得client区域左上角的屏幕坐标
									   //得到client真实屏幕区域
	rcClient.left = clienttl.x;
	rcClient.top = clienttl.y;
	rcClient.right += clienttl.x;
	rcClient.bottom += clienttl.y;

	GetWindowRect(hWnd, &windowrect);//得到窗口区域
}

bool D3DWnd::CreateDevice(D3DFORMAT textFormat, UINT backbuffercount)
{
	if (!hWnd)
		return false;

	Get2WndRect();

	HRESULT hr;

	//Direct3D 接口对象
	SAFE_RELEASE(lpD3D);
	lpD3D = Direct3DCreate9(D3D_SDK_VERSION);

	//caps
	if (FAILED(lpD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps)))
		MessageBoxW(hWnd, L"get caps FAILED!", L"ERROR", MB_OK | MB_APPLMODAL);

	//display mode
	hr = lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displaymode);
	if (FAILED(hr))
		MessageBoxW(hWnd, L"get displaymode FAILED!", L"ERROR", MB_OK | MB_APPLMODAL);

	mst = D3DMULTISAMPLE_NONE;

	//D3DPRESENT_PARAMETERS
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = WIDTHOF(rcClient);
	d3dpp.BackBufferHeight = HEIGHTOF(rcClient);
	d3dpp.BackBufferFormat = textFormat;//加速，displaymode.Format
	d3dpp.BackBufferCount = backbuffercount;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed = TRUE;
	d3dpp.EnableAutoDepthStencil = TRUE;//深度缓冲
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;//加速用D3DFMT_D24S8不用D3DFMT_D16
	d3dpp.Flags = 0;//允许backbuffer lockrect:D3DPRESENTFLAG_LOCKABLE_BACKBUFFER
	d3dpp.FullScreen_RefreshRateInHz = 0;
	//关闭垂直同步,(极大增加帧率，略增加内存占用，较大增加cpu占用率)
	//在主循环限制帧率情况下，开启默认垂直同步可能略增加cpu占用率
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.MultiSampleType = mst;
	d3dpp.MultiSampleQuality = 0;

	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vertexprocessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vertexprocessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	SAFE_RELEASE(device);
	hr = lpD3D->CreateDevice(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd
		, vertexprocessing, &d3dpp, &device
	);

	if (FAILED(hr))
	{
		//MessageBoxW(hWndMain, L"Device create FAILED!", L"ERROR", MB_OK | MB_APPLMODAL);
		return false;
	}

	return true;
}

bool D3DWnd::DXCreateFont(LPD3DXFONT * lpf, WCHAR *fontName
	, INT Height, INT Width, INT Weight
	, DWORD Quality
	, UINT MipLevels, bool Italics
	, DWORD Charset, DWORD OutputPrecision
	, DWORD PitchAndFamily
)
{
	if (FAILED(D3DXCreateFontW(
		device,
		Height, Width, Weight, MipLevels, Italics,
		Charset,
		OutputPrecision,
		Quality,	//NONANTIALIASED_QUALITY, DEFAULT_QUALITY
		PitchAndFamily,
		fontName,
		lpf
	)))
		return false;
	else
		return true;
}

bool D3DWnd::CreateMeshFVF(LPD3DXMESH *ppmesh, void * vertice, void * index, UINT16 sizev, UINT16 sizei, DWORD numfaces, DWORD numvertices, DWORD options)
{
	if(!vertice || !index || !numfaces || !numvertices || !ppmesh || !device)
		return false;

	void* pVertices = NULL;
	void* pIndex = NULL;

	HRESULT hr = D3DXCreateMeshFVF(numfaces, numvertices, D3DXMESH_MANAGED,
		options,
		device,
		ppmesh);
	if (FAILED(hr))
		return false;

	(*ppmesh)->LockVertexBuffer(0, &pVertices);
	memcpy(pVertices, vertice, numvertices * sizev);
	(*ppmesh)->UnlockVertexBuffer();

	(*ppmesh)->LockIndexBuffer(0, &pIndex);
	memcpy(pIndex, index, 3 * numfaces * sizei);
	(*ppmesh)->UnlockIndexBuffer();

	return true;
}

bool D3DWnd::ChangeMeshColor_tail(LPD3DXMESH * ppmesh, D3DCOLOR color)
{
	if (!ppmesh)
		return false;

	if(!(*ppmesh))
		return false;

	DWORD sizev = (*ppmesh)->GetNumBytesPerVertex();
	DWORD numvertices = (*ppmesh)->GetNumVertices();

	if (!numvertices)
		return false;

	LPDIRECT3DVERTEXBUFFER9 newbuf;
	(*ppmesh)->GetVertexBuffer(&newbuf);
	void* vbuf;
	if (FAILED(newbuf->Lock(0, 0, &vbuf, 0)))
		return false;
	for (int i = 0; i < (long long)numvertices; i++)
	{
		*(D3DCOLOR*)((byte*)vbuf + sizev*(i + 1) - sizeof(D3DCOLOR)) = color;//改颜色
	}
	newbuf->Unlock();

	return true;
}

bool D3DWnd::RefreshVertexBuffer(LPDIRECT3DVERTEXBUFFER9 * vb, void * vertice)
{
	if (!vertice || !vb || !(*vb))
		return false;

	D3DVERTEXBUFFER_DESC vbd;
	(*vb)->GetDesc(&vbd);

	void *ptr;
	if (FAILED((*vb)->Lock(0, vbd.Size, (void**)&ptr, 0)))
		return false;
	memcpy(ptr, vertice, vbd.Size);
	(*vb)->Unlock();

	return true;
}

bool D3DWnd::ChangeVBColor_tail(LPDIRECT3DVERTEXBUFFER9 * vb, D3DCOLOR color, UINT16 sizev, int nums)
{
	if (!device || !vb || !(*vb) || !sizev)
		return false;

	D3DVERTEXBUFFER_DESC vbd;
	(*vb)->GetDesc(&vbd);
	int vertexs = vbd.Size / sizev;

	void *ptr;
	if (FAILED((*vb)->Lock(0, vbd.Size, (void**)&ptr, 0)))
		return false;

	if (nums > 0)
		vertexs = nums;
	for (int i = 0; i < vertexs; i++)
	{
		*(D3DCOLOR*)((byte*)ptr + sizev*(i + 1) - sizeof(D3DCOLOR)) = color;//改颜色
	}
	(*vb)->Unlock();

	return true;
}

bool D3DWnd::CreateVertexBuffer_Custom1(LPDIRECT3DVERTEXBUFFER9 * vb, int x, int y, int r, D3DCOLOR color)
{
	if (!device || (r < 1))
		return false;

	const int fans = 20;
	FVF2 g_vertices[2 + fans];
	g_vertices[0] = { (float)x, (float)y, 0.0f, 1.0f, color };
	g_vertices[1] = { (float)x + r, (float)y, 0.0f, 1.0f, color };
	for (int i = 0; i < fans - 1; i++)
	{
		g_vertices[i + 2] = { (float)(x + r*cos(-(i + 1)*PI / fans))
			, (float)(y - r*sin(-(i + 1)*PI / fans)), 0.0f, 1.0f, color };
	}
	g_vertices[fans - 1] = { (float)(x - r), (float)(y), 0.0f, 1.0f, color };

	if (FAILED(device->CreateVertexBuffer(sizeof(g_vertices), 0,
		FVF_2CPD, D3DPOOL_SYSTEMMEM, vb, NULL)))
		return false;
	RefreshVertexBuffer(vb, g_vertices);

	return true;
}

bool D3DWnd::CreateVertexBuffer_Custom2(LPDIRECT3DVERTEXBUFFER9 * vb, int x, int y, int w, int h, D3DCOLOR color)
{
	if (!device || (w < 1) || (h < 1))
		return false;

	FVF2 g_vertices[4] = {
		{ (float)x, (float)y, 0.0f, 1.0f, color }
		,{ (float)x + w, (float)y, 0.0f, 1.0f, color }
		,{ (float)x, (float)y + h, 0.0f, 1.0f, color }
		,{ (float)x, (float)y + h, 0.0f, 1.0f, color }
	};

	if (FAILED(device->CreateVertexBuffer(sizeof(g_vertices), 0,
		FVF_2CPD, D3DPOOL_SYSTEMMEM, vb, NULL)))
		return false;
	RefreshVertexBuffer(vb, g_vertices);

	return true;
}

bool D3DWnd::CreateMesh_Custom1(LPD3DXMESH * ppmesh)
{
	if (!ppmesh || !device)
		return false;

	const int numvertices = 288;//288
	const int numfaces = 96;
	FAILED_RETURN_FALSE(D3DXCreateMeshFVF(numfaces, numvertices, D3DXMESH_MANAGED,
		FVF_3PDN,
		device,
		ppmesh));

	byte alpha = 0xD0;
	// 颜色
	D3DCOLOR COLOR1 = D3DCOLOR_ARGB(alpha, 0xFF, 0x30, 0x30);	//红
	D3DCOLOR COLOR2 = D3DCOLOR_ARGB(alpha, 0xF9, 0xBB, 0x42);	//黄
	D3DCOLOR COLOR3 = D3DCOLOR_ARGB(alpha, 0x7A, 0x3C, 0x92);	//紫
	D3DCOLOR COLOR4 = D3DCOLOR_ARGB(alpha, 0xED, 0xD4, 0xE7);	//粉
	D3DCOLOR COLOR5 = D3DCOLOR_ARGB(alpha, 0xFF, 0xFF, 0xFF);	//白
	// 法向量
	D3DXVECTOR3 NORMAL1 = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	D3DXVECTOR3 NORMAL2 = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	D3DXVECTOR3 NORMALSIDE1 = D3DXVECTOR3(0.343f, 0.939f, 0.0f);
	D3DXVECTOR3 NORMALSIDE2 = D3DXVECTOR3(0.939f, 0.343f, 0.0f);
	// 点
	float baselen = 2.0f;
	float thickness = baselen*0.6f;
	float SQRT2 = sqrt(2.0f);
	float SQRT3 = sqrt(3.0f);
	float P1Y_TO_P4Y = 25.0f / 23 + 1;
	D3DXVECTOR3 POINTLIST[9] = { D3DXVECTOR3(0.0f, 0.0f, -thickness / 2)
		, D3DXVECTOR3(0.0f, baselen*SQRT2, -thickness / 2)
		, D3DXVECTOR3(baselen*SQRT2 / 2, baselen*SQRT2 / 2, -thickness / 2)
		, D3DXVECTOR3(baselen*SQRT2, 0.0f, -thickness / 2)
		, D3DXVECTOR3(0.0f, baselen*SQRT2*P1Y_TO_P4Y, -thickness / 2)
		, D3DXVECTOR3(baselen*SQRT2*P1Y_TO_P4Y / 2 - baselen / 2 / SQRT2
			, baselen*SQRT2*P1Y_TO_P4Y / 2 + baselen / 2 / SQRT2, -thickness / 2)
		, D3DXVECTOR3(baselen*SQRT2*P1Y_TO_P4Y / 2 + baselen / 2 / SQRT2
			, baselen*SQRT2*P1Y_TO_P4Y / 2 - baselen / 2 / SQRT2, -thickness / 2)
		, D3DXVECTOR3(baselen*SQRT2*P1Y_TO_P4Y, 0.0f, -thickness / 2)
		, D3DXVECTOR3(baselen*SQRT2*P1Y_TO_P4Y - baselen*SQRT2 / 2
			, baselen*SQRT2*P1Y_TO_P4Y - baselen*SQRT2 / 2, -thickness / 2)
	};
	// 索引
	WORD indexlist[10 * 3] = {
		0,1,2	,3,4,5	,6,7,8	,9,10,11	,12,13,14	,15,16,17	
		,18,19,20	,21,22,23	,24,25,26	,27,28,29
	};

	WORD index[numfaces * 3];
	FVF3 vertice[numvertices];
	for (int i = 0; i < 4; i++)
	{
		int bias = i*numvertices / 4;

		// 正面点
		vertice[0 + bias] = { POINTLIST[0], NORMAL1, COLOR1 };
		vertice[1 + bias] = { POINTLIST[1], NORMAL1, COLOR1 };
		vertice[2 + bias] = { POINTLIST[2], NORMAL1, COLOR1 };\
		vertice[3 + bias] = { POINTLIST[0], NORMAL1, COLOR2 };
		vertice[4 + bias] = { POINTLIST[2], NORMAL1, COLOR2 };
		vertice[5 + bias] = { POINTLIST[3], NORMAL1, COLOR2 };\
		vertice[6 + bias] = { POINTLIST[1], NORMAL1, COLOR2 };
		vertice[7 + bias] = { POINTLIST[4], NORMAL1, COLOR2 };
		vertice[8 + bias] = { POINTLIST[5], NORMAL1, COLOR2 };\
		vertice[9 + bias] = { POINTLIST[1], NORMAL1, COLOR5 };
		vertice[10 + bias] = { POINTLIST[5], NORMAL1, COLOR5 };
		vertice[11 + bias] = { POINTLIST[2], NORMAL1, COLOR5 };\
		vertice[12 + bias] = { POINTLIST[2], NORMAL1, COLOR3 };
		vertice[13 + bias] = { POINTLIST[5], NORMAL1, COLOR3 };
		vertice[14 + bias] = { POINTLIST[6], NORMAL1, COLOR3 };\
		vertice[15 + bias] = { POINTLIST[2], NORMAL1, COLOR5 };
		vertice[16 + bias] = { POINTLIST[6], NORMAL1, COLOR5 };
		vertice[17 + bias] = { POINTLIST[3], NORMAL1, COLOR5 };\
		vertice[18 + bias] = { POINTLIST[3], NORMAL1, COLOR4 };
		vertice[19 + bias] = { POINTLIST[6], NORMAL1, COLOR4 };
		vertice[20 + bias] = { POINTLIST[7], NORMAL1, COLOR4 };\
		vertice[21 + bias] = { POINTLIST[4], NORMAL1, COLOR1 };
		vertice[22 + bias] = { POINTLIST[8], NORMAL1, COLOR1 };
		vertice[23 + bias] = { POINTLIST[5], NORMAL1, COLOR1 };\
		vertice[24 + bias] = { POINTLIST[5], NORMAL1, COLOR5 };
		vertice[25 + bias] = { POINTLIST[8], NORMAL1, COLOR5 };
		vertice[26 + bias] = { POINTLIST[6], NORMAL1, COLOR5 };\
		vertice[27 + bias] = { POINTLIST[6], NORMAL1, COLOR1 };
		vertice[28 + bias] = { POINTLIST[8], NORMAL1, COLOR1 };
		vertice[29 + bias] = { POINTLIST[7], NORMAL1, COLOR1 };
		// 正面点的索引
		for (int j = 0; j < 30; j++)
		{
			index[j + bias] = indexlist[j] + bias;
		}


		for (int j = 0; j < 10; j++)
		{
			POINTLIST[j].z += thickness;
		}
		// 反面点
		vertice[30 + bias] = { POINTLIST[0], NORMAL2, COLOR1 };
		vertice[31 + bias] = { POINTLIST[1], NORMAL2, COLOR1 };
		vertice[32 + bias] = { POINTLIST[2], NORMAL2, COLOR1 };\
		vertice[33 + bias] = { POINTLIST[0], NORMAL2, COLOR2 };
		vertice[34 + bias] = { POINTLIST[2], NORMAL2, COLOR2 };
		vertice[35 + bias] = { POINTLIST[3], NORMAL2, COLOR2 };\
		vertice[36 + bias] = { POINTLIST[1], NORMAL2, COLOR2 };
		vertice[37 + bias] = { POINTLIST[4], NORMAL2, COLOR2 };
		vertice[38 + bias] = { POINTLIST[5], NORMAL2, COLOR2 };\
		vertice[39 + bias] = { POINTLIST[1], NORMAL2, COLOR5 };
		vertice[40 + bias] = { POINTLIST[5], NORMAL2, COLOR5 };
		vertice[41 + bias] = { POINTLIST[2], NORMAL2, COLOR5 };\
		vertice[42 + bias] = { POINTLIST[2], NORMAL2, COLOR3 };
		vertice[43 + bias] = { POINTLIST[5], NORMAL2, COLOR3 };
		vertice[44 + bias] = { POINTLIST[6], NORMAL2, COLOR3 };\
		vertice[45 + bias] = { POINTLIST[2], NORMAL2, COLOR5 };
		vertice[46 + bias] = { POINTLIST[6], NORMAL2, COLOR5 };
		vertice[47 + bias] = { POINTLIST[3], NORMAL2, COLOR5 };\
		vertice[48 + bias] = { POINTLIST[3], NORMAL2, COLOR4 };
		vertice[49 + bias] = { POINTLIST[6], NORMAL2, COLOR4 };
		vertice[50 + bias] = { POINTLIST[7], NORMAL2, COLOR4 };\
		vertice[51 + bias] = { POINTLIST[4], NORMAL2, COLOR1 };
		vertice[52 + bias] = { POINTLIST[8], NORMAL2, COLOR1 };
		vertice[53 + bias] = { POINTLIST[5], NORMAL2, COLOR1 };\
		vertice[54 + bias] = { POINTLIST[5], NORMAL2, COLOR5 };
		vertice[55 + bias] = { POINTLIST[8], NORMAL2, COLOR5 };
		vertice[56 + bias] = { POINTLIST[6], NORMAL2, COLOR5 };\
		vertice[57 + bias] = { POINTLIST[6], NORMAL2, COLOR1 };
		vertice[58 + bias] = { POINTLIST[8], NORMAL2, COLOR1 };
		vertice[59 + bias] = { POINTLIST[7], NORMAL2, COLOR1 };
		//反面点的索引
		for (int j = 30; j < 60; j++)
		{
			index[j + bias] = indexlist[60 - j - 1] + 30 + bias;
		}

		D3DXVECTOR3 ANTIPOINT4 = POINTLIST[4];
		D3DXVECTOR3 ANTIPOINT7 = POINTLIST[7];
		D3DXVECTOR3 ANTIPOINT8 = POINTLIST[8];
		//回到正面点
		for (int j = 0; j < 10; j++)
		{
			POINTLIST[j].z -= thickness;
		}
		//侧面点
		vertice[60 + bias] = { POINTLIST[4], NORMALSIDE1, COLOR1 };
		vertice[61 + bias] = { ANTIPOINT4, NORMALSIDE1, COLOR1 };
		vertice[62 + bias] = { ANTIPOINT8, NORMALSIDE1, COLOR1 };
		vertice[63 + bias] = { POINTLIST[4], NORMALSIDE1, COLOR1 };
		vertice[64 + bias] = { ANTIPOINT8, NORMALSIDE1, COLOR1 };
		vertice[65 + bias] = { POINTLIST[8], NORMALSIDE1, COLOR1 };
		vertice[66 + bias] = { POINTLIST[8], NORMALSIDE2, COLOR1 };
		vertice[67 + bias] = { ANTIPOINT8, NORMALSIDE2, COLOR1 };
		vertice[68 + bias] = { ANTIPOINT7, NORMALSIDE2, COLOR1 };
		vertice[69 + bias] = { POINTLIST[8], NORMALSIDE2, COLOR1 };
		vertice[70 + bias] = { ANTIPOINT7, NORMALSIDE2, COLOR1 };
		vertice[71 + bias] = { POINTLIST[7], NORMALSIDE2, COLOR1 };
		//侧面点的索引
		for (int j = 0; j < 12; j++)
		{
			index[j + 60 + bias] = indexlist[j] + 60 + bias;
		}


		for (int j = 0; j < 9; j++)//点旋转90度
		{
			float temp = POINTLIST[j].x;
			POINTLIST[j].x = POINTLIST[j].y;
			POINTLIST[j].y = -temp;
		}
		//法向量旋转
		float temp = NORMALSIDE1.x;
		NORMALSIDE1.x = NORMALSIDE1.y;
		NORMALSIDE1.y = -temp;
		temp = NORMALSIDE2.x;
		NORMALSIDE2.x = NORMALSIDE2.y;
		NORMALSIDE2.y = -temp;
	}


	void* pVertices = NULL;
	void* pIndex = NULL;
	(*ppmesh)->LockVertexBuffer(0, &pVertices);
	memcpy(pVertices, vertice, numvertices * sizeof(FVF3));
	(*ppmesh)->UnlockVertexBuffer();

	(*ppmesh)->LockIndexBuffer(0, &pIndex);
	memcpy(pIndex, index, 3 * numfaces * sizeof(WORD));
	(*ppmesh)->UnlockIndexBuffer();

	D3DXMATERIAL material;
	ZeroMemory(&material, sizeof(D3DMATERIAL9));
	material.MatD3D.Ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
	material.MatD3D.Diffuse = { 1.0f,1.0f,1.0f,0.8f };
	material.MatD3D.Specular = { 0.6f,0.6f,0.6f,1.0f };
	material.MatD3D.Emissive = { 0.0f,0.0f,0.0f,0.0f };
	material.MatD3D.Power = 400.0f;
	material.pTextureFilename = NULL;
	//strcpy_s(material.pTextureFilename, "crystal.x");

	volatile static HRESULT hr;
	hr = D3DXSaveMeshToXW(L"crystal.x", *ppmesh, NULL, &material, NULL, 1, D3DXF_FILEFORMAT_TEXT);
	if (SUCCEEDED(hr))
		return true;
	else
		return false;
}

void D3DWnd::CreateSphere(ID3DXMesh ** obj, int finess, float radius, D3DCOLOR color, float height)
{
	//球形
	const int loops = 40;//球精细度（180度内的环数）
	float loopradian = (float)(3.14159f / loops);
	int point = 0;
	const int pointcount = 2 + (loops - 1)*loops * 2;
	//顶点序列
	if (radius <= 0)
		radius = 1;
	float top = height + radius;
	FVF3 g_vertices[pointcount] =
	{
		{ D3DXVECTOR3(0, 0, 0),
		D3DXVECTOR3(0.0f, 0.0f, -1.0f), color }
	};
	//三角索引序列
	const int indexcount = (2 * loops + 2 * (loops - 2)*loops) * 3 * 2;
	WORD index[indexcount] =
	{
		0
	};


	//顶点和索引列表赋值
	g_vertices[0] = { D3DXVECTOR3(0.0f, 0.0f, -top),//顶点
		D3DXVECTOR3(0.0f, 0.0f, -1.0f), color };
	float hradian = 0.0f;
	float vradian = loopradian;
	int loop = 1;
	int face = 1;
	for (face = 1, point = 1; point <= 2 * loops; face++, point++)//第一环
	{
		hradian = loopradian*(point - 1);
		g_vertices[point].pos = D3DXVECTOR3(
			(float)(radius*cos(hradian)*sin(vradian))
			, (float)(radius*sin(hradian)*sin(vradian))
			, (float)(-height - radius*cos(vradian))
		);
		g_vertices[point].normal
			= D3DXVECTOR3((float)(cos(hradian)*sin(vradian)), (float)(sin(hradian)*sin(vradian)), (float)(-cos(vradian)));
		g_vertices[point].color = color;

		index[(point - 1) * 3] = 0;
		if (point == loops * 2)
			index[(point - 1) * 3 + 1] = 1;
		else
			index[(point - 1) * 3 + 1] = point + 1;
		index[(point - 1) * 3 + 2] = point;
	}
	for (loop = 2; loop < loops; loop++)//第二环开始
	{
		vradian = loop*loopradian;
		int startpoint = (loop - 1)*loops * 2 + 1;
		for (point = startpoint; point <= loop*loops * 2; point++)
		{
			hradian = loopradian*(point - startpoint);
			g_vertices[point].pos = D3DXVECTOR3(
				(float)(radius*cos(hradian)*sin(vradian))
				, (float)(radius*sin(hradian)*sin(vradian))
				, (float)(-height - radius*cos(vradian))
			);
			g_vertices[point].normal = D3DXVECTOR3(
				(float)(cos(hradian)*sin(vradian))
				, (float)(sin(hradian)*sin(vradian))
				, (float)(-cos(vradian))
			);
			g_vertices[point].color = color;

			index[(face - 1) * 3] = point - loops * 2;
			if (point == loop*loops * 2)
				index[(face - 1) * 3 + 1] = startpoint;
			else
				index[(face - 1) * 3 + 1] = point + 1;
			index[(face - 1) * 3 + 2] = point;
			face++;

			index[(face - 1) * 3] = point - loops * 2;
			if (point == loop*loops * 2)
			{
				index[(face - 1) * 3 + 1] = point - 2 * loops * 2 + 1;
				index[(face - 1) * 3 + 2] = startpoint;
			}
			else
			{
				index[(face - 1) * 3 + 1] = point - loops * 2 + 1;
				index[(face - 1) * 3 + 2] = point + 1;
			}
			face++;
		}
	}
	//最后一点
	g_vertices[point].pos = D3DXVECTOR3(0.0f, 0.0f, -height + radius);
	g_vertices[point].normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	int lastpoint = point - loops * 2;
	for (; face <= indexcount / 3; face++, lastpoint++)
	{
		index[(face - 1) * 3] = lastpoint;
		if (face == indexcount / 3)
		{
			index[(face - 1) * 3 + 1] = point - loops * 2;
		}
		else
		{
			index[(face - 1) * 3 + 1] = lastpoint + 1;
		}
		index[(face - 1) * 3 + 2] = point;
	}
	g_vertices[point].color = color;

	void* pVertices = NULL;
	void* pIndex = NULL;
	HRESULT hr;

	if (*obj)
		(*obj)->Release();
	//mesh方式
	hr = D3DXCreateMeshFVF(indexcount / 3, pointcount, D3DXMESH_MANAGED,
		FVF_3PDN,
		device,
		obj);
	(*obj)->LockVertexBuffer(0, &pVertices);
	memcpy(pVertices, g_vertices, sizeof(g_vertices));
	(*obj)->UnlockVertexBuffer();

	(*obj)->LockIndexBuffer(0, &pIndex);
	memcpy(pIndex, index, sizeof(index));
	(*obj)->UnlockIndexBuffer();
}

LPDIRECT3DDEVICE9 D3DWnd::GetDevice()
{
	return device;
}

void D3DWnd::ChangeMultiSample()
{
	if (!lpD3D)
		return;

	D3DMULTISAMPLE_TYPE newmst = (D3DMULTISAMPLE_TYPE)(mst + 1 > D3DMULTISAMPLE_16_SAMPLES ? D3DMULTISAMPLE_NONE : mst + 1);
	while (newmst != mst)
	{
		if (lpD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL, displaymode.Format, true,
			newmst, NULL) == D3D_OK)
		{
			// 保存多采样类型
			d3dpp.MultiSampleType = mst = newmst;
			break;
		}
		newmst = (D3DMULTISAMPLE_TYPE)(newmst + 1 > D3DMULTISAMPLE_16_SAMPLES ? D3DMULTISAMPLE_NONE : newmst + 1);
	}
}

void D3DWnd::DisplayAdapter()
{
	if (!lpD3D)
		return;

	D3DADAPTER_IDENTIFIER9 adapterID;
	WCHAR strBuffer[2048];
	DWORD dwDisplayCount = lpD3D->GetAdapterCount();
	for (DWORD i = 0; i < dwDisplayCount; i++)
	{
		if (lpD3D->GetAdapterIdentifier(i/*D3DADAPTER_DEFAULT*/, 0, &adapterID) != D3D_OK)
		{
			break;
		}
		_stprintf_s(strBuffer, L"Driver: %S\n\
Description: %S\n\
Device Name: %S\n\
Vendor id:%4x\n\
Device id: %4x\n\
Product: %x\n\
Version:%x\n\
SubVersion: %x\n\
Build: %x %d.%d.%d.%d\n\
SubSysId: %x\n\
Revision: %x\n\
GUID %S\n\
WHQLLevel:%d\n",
			adapterID.Driver, adapterID.Description, adapterID.DeviceName, adapterID.VendorId, adapterID.DeviceId,
			HIWORD(adapterID.DriverVersion.HighPart), LOWORD(adapterID.DriverVersion.HighPart),
			HIWORD(adapterID.DriverVersion.LowPart), LOWORD(adapterID.DriverVersion.LowPart),
			HIWORD(adapterID.DriverVersion.HighPart), LOWORD(adapterID.DriverVersion.HighPart),
			HIWORD(adapterID.DriverVersion.LowPart), LOWORD(adapterID.DriverVersion.LowPart),
			adapterID.SubSysId, adapterID.Revision, print_guid(adapterID.DeviceIdentifier), adapterID.WHQLLevel);
		MessageBox(hWnd, strBuffer, L"Graphics Identity", MB_OK | MB_APPLMODAL);
	}
}



