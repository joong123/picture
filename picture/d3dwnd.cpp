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
	, D3DCOLOR backcolor)
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

	HBRUSH hb = CreateSolidBrush(RGB(backcolor>>16, (backcolor>>8)&0xFF, backcolor & 0xFF));

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
		//return false;
	}


	hwnd = CreateWindowExW(ExStyle, szWindowClass, lpWindowName, Style,
		WINDOWPOSX_INIT, WINDOWPOSY_INIT, WINDOWWIDTH_INIT, WINDOWHEIGHT_INIT
		, nullptr, nullptr, hInstance, nullptr);

	if (!hwnd)
	{
		return FALSE;
	}

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	return true;
}

void D3DWnd::SetHWND(HWND hwnd)
{
	this->hwnd = hwnd;
}

HWND D3DWnd::GetHWND()
{
	return hwnd;
}

inline void D3DWnd::Get2WndRect()
{
	if (!hwnd)
		return;

	GetClientRect(hwnd, &clientrect);//得到client区域尺寸
	POINT clienttl = { 0, 0 };
	ClientToScreen(hwnd, &clienttl);//获得client区域左上角的屏幕坐标
									   //得到client真实屏幕区域
	clientrect.left = clienttl.x;
	clientrect.top = clienttl.y;
	/*clientrect.right += clientrect.left;
	clientrect.bottom += clientrect.top;*/
	clientrect.right += clienttl.x;
	clientrect.bottom += clienttl.y;

	GetWindowRect(hwnd, &windowrect);//得到窗口区域
}

bool D3DWnd::CreateDevice(D3DFORMAT format, UINT backbuffercount)
{
	if (!hwnd)
		return false;

	Get2WndRect();

	HRESULT hr;

	//Direct3D 接口对象
	SAFE_RELEASE(lpD3D);
	lpD3D = Direct3DCreate9(D3D_SDK_VERSION);

	//caps
	if (FAILED(lpD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps)))
		MessageBoxW(hwnd, L"get caps FAILED!", L"ERROR", MB_OK | MB_APPLMODAL);

	//display mode
	hr = lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displaymode);
	if (FAILED(hr))
		MessageBoxW(hwnd, L"get displaymode FAILED!", L"ERROR", MB_OK | MB_APPLMODAL);

	//D3DPRESENT_PARAMETERS
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = WIDTHOF(clientrect);
	d3dpp.BackBufferHeight = HEIGHTOF(clientrect);
	d3dpp.BackBufferFormat = format;//加速，displaymode.Format
	d3dpp.BackBufferCount = backbuffercount;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.Windowed = TRUE;
	d3dpp.EnableAutoDepthStencil = FALSE;//深度缓冲
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;//加速用D3DFMT_D24S8不用D3DFMT_D16
	d3dpp.Flags = 0;//允许backbuffer lockrect:D3DPRESENTFLAG_LOCKABLE_BACKBUFFER
	d3dpp.FullScreen_RefreshRateInHz = 0;
	//关闭垂直同步,(极大增加帧率，略增加内存占用，较大增加cpu占用率)
	//在主循环限制帧率情况下，开启默认垂直同步可能略增加cpu占用率
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;

	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vertexprocessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vertexprocessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	SAFE_RELEASE(device);
	hr = lpD3D->CreateDevice(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd
		, vertexprocessing, &d3dpp, &device
	);

	if (FAILED(hr))
	{
		//MessageBoxW(mainwnd, L"Device create FAILED!", L"ERROR", MB_OK | MB_APPLMODAL);
		return false;
	}

	return true;
}

bool D3DWnd::D3DCreateFont(LPD3DXFONT * lpf, WCHAR *fontName
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

LPDIRECT3DDEVICE9 D3DWnd::GetDevice()
{
	return device;
}

void D3DWnd::DisplayAdapter()
{
	D3DADAPTER_IDENTIFIER9 adapterID;
	WCHAR strBuffer[2048];
	DWORD dwDisplayCount = lpD3D->GetAdapterCount();
	for (DWORD i = 0; i < dwDisplayCount; i++)
	{
		if (lpD3D->GetAdapterIdentifier(i/*D3DADAPTER_DEFAULT*/, 0, &adapterID) != D3D_OK)
		{
			break;
		}
		_stprintf_s(strBuffer, L"Driver: %S\n, Description: %S\n, Device Name: %S\n, Vendor id:%4x\n, Device id: %4x\n, Product: %x\n\
, Version:%x\n, SubVersion: %x\n, Build: %x %d.%d.%d.%d\n\
, SubSysId: %x\n, Revision: %x\n, GUID %S\n, WHQLLevel:%d\n",
			adapterID.Driver, adapterID.Description, adapterID.DeviceName, adapterID.VendorId, adapterID.DeviceId,
			HIWORD(adapterID.DriverVersion.HighPart), LOWORD(adapterID.DriverVersion.HighPart),
			HIWORD(adapterID.DriverVersion.LowPart), LOWORD(adapterID.DriverVersion.LowPart),
			HIWORD(adapterID.DriverVersion.HighPart), LOWORD(adapterID.DriverVersion.HighPart),
			HIWORD(adapterID.DriverVersion.LowPart), LOWORD(adapterID.DriverVersion.LowPart),
			adapterID.SubSysId, adapterID.Revision, print_guid(adapterID.DeviceIdentifier), adapterID.WHQLLevel);
		MessageBox(hwnd, strBuffer, L"Identity", MB_OK | MB_APPLMODAL);
	}
}


string D3DWnd::GetFMTStr(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_R8G8B8:
		return "D3DFMT_R8G8B8";
	case D3DFMT_A8R8G8B8:
		return "D3DFMT_A8R8G8B8";
	case D3DFMT_X8R8G8B8:
		return "D3DFMT_X8R8G8B8";
	case D3DFMT_R5G6B5:
		return "D3DFMT_R5G6B5";
	case D3DFMT_X1R5G5B5:
		return "D3DFMT_X1R5G5B5";
	case D3DFMT_A1R5G5B5:
		return "D3DFMT_A1R5G5B5";
	case D3DFMT_A4R4G4B4:
		return "D3DFMT_A4R4G4B4";
	case D3DFMT_R3G3B2:
		return "D3DFMT_R3G3B2";
	case D3DFMT_A8:
		return "D3DFMT_A8";
	case D3DFMT_A8R3G3B2:
		return "D3DFMT_A8R3G3B2";
	case D3DFMT_X4R4G4B4:
		return "D3DFMT_X4R4G4B4";
	case D3DFMT_A2B10G10R10:
		return "D3DFMT_A2B10G10R10";
	case D3DFMT_A8B8G8R8:
		return "D3DFMT_A8B8G8R8";
	case D3DFMT_X8B8G8R8:
		return "D3DFMT_X8B8G8R8";
	case D3DFMT_G16R16:
		return "D3DFMT_G16R16";
	case D3DFMT_A2R10G10B10:
		return "D3DFMT_A2R10G10B10";
	case D3DFMT_A16B16G16R16:
		return "D3DFMT_A16B16G16R16";
	case D3DFMT_A8P8:
		return "D3DFMT_A8P8";
	case D3DFMT_P8:
		return "D3DFMT_P8";
	case D3DFMT_L8:
		return "D3DFMT_L8";
	case D3DFMT_A8L8:
		return "D3DFMT_A8L8";
	case D3DFMT_A4L4:
		return "D3DFMT_A4L4";
	case D3DFMT_V8U8:
		return "D3DFMT_V8U8";
	case D3DFMT_L6V5U5:
		return "D3DFMT_L6V5U5";
	case D3DFMT_X8L8V8U8:
		return "D3DFMT_X8L8V8U8";
	case D3DFMT_Q8W8V8U8:
		return "D3DFMT_Q8W8V8U8";
	case D3DFMT_V16U16:
		return "D3DFMT_V16U16";
	case D3DFMT_A2W10V10U10:
		return "D3DFMT_A2W10V10U10";
	default:
		return "UNKNOWN";
	}
}


