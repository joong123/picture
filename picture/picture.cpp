// picture.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "picture.h"

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PICTURE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
	//添加
	Init();
	D3DInit();

	WCHAR *param = 0;
	param = GetCommandLineW();
	OnLoadFile(param);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PICTURE));

	MSG msg = { 0 };

    // 主消息循环: 
	while (msg.message != WM_QUIT)
	{
		//
		//在发布消息前做的动作
		//
		//更新鼠标坐标,在发布消息前更新鼠标位置，防止消息处理函数中数据错误
		lastcursor = cursor;
		GetCursorPos(&cursor);
		GetCurPos();//得到鼠标位置（种类）,不放在onmousemove中因为这是全屏范围的鼠标信息。

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}


		if (IsIconic(mainwnd))
		{
			Sleep(10);//最小化状态降低资源消耗
			continue;
		}

		//
		//发布消息后要做的动作
		//
		DelayFlag();//更新延迟的标志
		//bool oldsize = onsize;
		//if (oldsize && !onsize)//size结束动作
		//{
		//	Get2WndRect();
		//	if (maindevice)
		//	{
		//		ResetDevice();//处理设备丢失
		//	}
		//	if (!mainbmp.Empty())
		//		RefreshSurf();//不重新创建图片可能显示错误
		//}
		//SetCursor();//设置鼠标样式
		//拖动时或鼠标静止时不获取屏幕颜色
		//if (!ondrag && !onzoom)
		//{
		//	HDC pdc = ::GetDC(NULL);
		//	screencolor = GetPixel(pdc, cursor.x, cursor.y);
		//	DeleteDC(pdc);
		//	//低1，3字节互换
		//	DWORD low = screencolor & 0xFF;
		//	DWORD mid = screencolor & 0xFF00;
		//	DWORD high = screencolor & 0xFF0000;
		//	DWORD top = screencolor & 0xFF000000;
		//	screencolor = top + mid + (low << 16) + (high >> 16);
		//}

		//计数
		loopcount++;
		time(&nowtime);

		//渲染
		Render();

		//降低资源消耗
		QueryPerformanceCounter(&etime);
		float testfps;
		if (etime.QuadPart != stime.QuadPart)
			testfps = (float)frequency.QuadPart / (etime.QuadPart - stime.QuadPart);
		if (ondrag || onzoom)
		{
			if (testfps > NORMAL_FPS)
				Sleep((UINT32)(1000.0f / NORMAL_FPS - 1000.0f / testfps));
		}
		else
		{
			if (testfps > MIN_FPS)
				Sleep((UINT32)max(1000.0f / MIN_FPS - 1000.0f / testfps, 1));
		}

		//计算fps
		QueryPerformanceCounter(&etime);
		if (nowtime != lasttime)//每秒执行
		{
			fps = (float)frequency.QuadPart / (etime.QuadPart - stime.QuadPart);
			lasttime = nowtime;
		}

		stime = etime;//开始计时
		QueryPerformanceCounter(&stime);
    }
    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PICTURE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PICTURE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowExW(WS_EX_ACCEPTFILES, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 800, 520, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   mainwnd = hWnd;//存储主窗口句柄

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//
	//surfsrc改变，一定要重新生成surface。仅窗口尺寸改变，不需要重新生成，但要更新窗口信息Get2WndRect()
	//Zoom()中已包含重新生成surface
	//
	//

	char key;
	UINT uFileNum;
//	D3DSURFACE_DESC surfDesc;

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case IDM_OPEN:
				OPENFILENAME opfn;
				WCHAR openfilename[MAX_PATH];//存放文件名  

				//初始化     
				ZeroMemory(&opfn, sizeof(OPENFILENAME));
				opfn.lStructSize = sizeof(OPENFILENAME);//结构体大小
				//设置过滤     
				opfn.lpstrFilter = L"所有文件\0*.*\0bmp文件\0*.bmp\0png文件\0*.png\0jpg文件\0*.jpg\0";
				//默认过滤器索引设为1     
				opfn.nFilterIndex = 1;
				//文件名的字段必须先把第一个字符设为\0
				opfn.lpstrFile = openfilename;
				opfn.lpstrFile[0] = '\0';
				opfn.nMaxFile = sizeof(openfilename);
				//设置标志位，检查目录或文件是否存在     
				opfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
				//opfn.lpstrInitialDir = NULL;     
				// 显示对话框让用户选择文件     
				if (GetOpenFileName(&opfn))
				{
					//选中文件后操作
					//SendMessage(mainwnd, WM_SETTEXT, NULL, (LPARAM)openfilename);
					OnDropFile(openfilename);
				}
				break;
			case IDM_CLEAR:
				Clear();
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
			if (!IsIconic(mainwnd))
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				// TODO: 在此处添加使用 hdc 的任何绘图代码...
				Render();
				EndPaint(hWnd, &ps);
			}
			else
			{

			}
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_DROPFILES:
		uFileNum = ::DragQueryFile((HDROP)wParam, 0xffffffff, NULL, 0);
		::DragQueryFileW((HDROP)wParam, 0, strFileName, MAX_PATH);//获取文件名
		OnDropFile(strFileName);
		break;
	case WM_SIZE:
		if (!IsIconic(mainwnd))
		{
			onsize = true;
			sizetick = GetTickCount64();

			GetCursorPos(&cursor);//size时主循环中获取鼠标不运行
			int lastwrwidth = Widthof(clientrect);
			int lastwrheight = Heightof(clientrect);
			Get2WndRect();//更新窗口区域，如果改为size结束动作时做，TODO：设置另一窗口区域信息实时更新

			if (maindevice)
			{
				ResetDevice();//处理设备丢失
			}

			//!!!surface拷贝到backbuffer中作了调整，可以不重设surface，无论当前clip是什么状态
			//有图片时，设置clip并且窗口在扩大，则重新生成surface
			if (!mainbmp.Empty() 
				&& clip 
				&&(Widthof(clientrect)>= lastwrwidth|| Heightof(clientrect) >= lastwrheight))
				RefreshSurf();
			SurfStatusChange();
		}
		break;
	case WM_MOVE:
		onmove = true;
		movetick = GetTickCount64();
		if (!onsize)
		{
			Get2WndRect();//size时不做
			Render();//size时的move消息不重绘，在onpaint重绘
		}
		break;
	case WM_KEYDOWN:
		key = wParam;
		if (mode == MODE_PIC)
		{
			switch (key)
			{
			case 'B'://缩放率切回1
				realzoom = zoom = 1;
				Zoom(oldzoom, zoom);
				oldzoom = zoom;

				SurfStatusChange();
				break;
			case 'C'://窗口调整到正好包括图像
				if (!mainbmp.Empty())
				{
					wndrect.right = wndrect.left + zoomw + wbias;
					wndrect.bottom = wndrect.top + zoomh + hbias;

					//movewindow发出wmsize消息，需最后做
					MoveWindow(mainwnd, wndrect.left, wndrect.top
						, wndrect.right - wndrect.left
						, wndrect.bottom - wndrect.top, TRUE);
					Get2WndRect();

					surfsrc = { 0, 0 };

					RefreshSurf();

					SurfStatusChange();
				}
				break;
			case 'E'://切换信息显示
				infoshow = !infoshow;
				break;
			case 'F'://根据，设置标准窗口尺寸
				if (!mainbmp.Empty())
				{
					FitWnd();
					Get2WndRect();

					surfsrc = { 0,0 };

					RefreshSurf();

					SurfStatusChange();
				}
				break;
			case 'M'://图片居中
				if (!mainbmp.Empty())
				{
					CenterPic();

					RefreshSurf();

					SurfStatusChange();
				}
				break;
			case 'P'://清除图像
				Clear();
				break;
			case 'Q'://图像放回左上角
				surfsrc = { 0,0 };
				RefreshSurf();

				SurfStatusChange();
				break;
			case 'R'://手动渲染一次
				Render();
				break;
			}
		}
		else if (mode == MODE_CMD)
		{
			;
		}
		break;
	case WM_MOUSEWHEEL:
		if (!mainbmp.Empty())
		{
			Togglezoom(wParam);//修改zoom
			Zoom(oldzoom, realzoom);//放大，调整surface位置
			oldzoom = realzoom;

			SurfStatusChange();
		}

		break;
	case WM_SETCURSOR:
		//GetCursorPos(&cursor);
		if (inside(cursor, clientrect))
		{
			if(NoPic())
				SetCursor(LoadCursor(NULL, IDC_ARROW));
			else
				SetCursor(LoadCursor(NULL, IDC_ARROW));//可设置其他鼠标
		}
		else
			SetCursor(LoadCursor(NULL, IDC_ARROW));

		break;
	case WM_LBUTTONDOWN:
		if (!mainbmp.Empty())
		{
			SetCapture(mainwnd);
			//GetCursorPos(&lastpos);
			lastpos = cursor;
			ondrag = true;
			break;
		}
	case WM_MOUSEMOVE:
		if (ondrag)
		{
			//移动图片
			surfsrc.x -= cursor.x - lastpos.x;
			surfsrc.y -= cursor.y - lastpos.y;

			//记录鼠标位置
			lastpos = cursor;

			bool lastpicclipped = picclipped;

			//移动时如果有clip标志，并且图片小于一定尺寸，则取消clip，重新生成surface，增加拖动速度
			//（尤其对于zoom<1的情况）
			if (zoomw <= MAX_FORCECLIP_WIDTH_DRAG && zoomh <= MAX_FORCECLIP_HEIGHT_DRAG && clip)
			{
				clip = false;
				RefreshSurf();
			}

			//如果图片始终全部在窗口客户区范围内（前后surfclipped状态都是0），或者不设置clip标志，就不更新
			if ((lastpicclipped || picclipped) && clip)
				RefreshSurf();

			CalcClipped();//计算图片clip状态

			CalcSurfMapInfo();//更新surface拷贝到backbuffer参数
		}
		else
		{
			//GetCursorPos(&cursor);
			GetCurInfo();//不放在主循环里,防止拖动图像时鼠标当前所在像素值抖动
		}
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		ondrag = false;
		break;
	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE)
		{
			//失去焦点，清除标志
			ClearFlag();
		}
		break;
	case WM_ERASEBKGND:
		//不采取动作，防止重绘时闪烁
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

short GetSizeType(CPoint point)
{
	short sizetype = 0;

	if (point.x <= wndrect.left + SIZE_NEAR_PIXEL && point.x >= wndrect.left - SIZE_NEAR_PIXEL
		&& point.y <= wndrect.top + SIZE_NEAR_PIXEL && point.y >= wndrect.top - SIZE_NEAR_PIXEL)
		sizetype = HTTOPLEFT;
	else if (point.x <= wndrect.right + SIZE_NEAR_PIXEL && point.x >= wndrect.right - SIZE_NEAR_PIXEL
		&& point.y <= wndrect.bottom + SIZE_NEAR_PIXEL && point.y >= wndrect.bottom - SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOMRIGHT;
	else if (point.x <= wndrect.right + SIZE_NEAR_PIXEL && point.x >= wndrect.right - SIZE_NEAR_PIXEL
		&& point.y <= wndrect.top + SIZE_NEAR_PIXEL && point.y >= wndrect.top - SIZE_NEAR_PIXEL)
		sizetype = HTTOPRIGHT;
	else if (point.x <= wndrect.left + SIZE_NEAR_PIXEL && point.x >= wndrect.left - SIZE_NEAR_PIXEL
		&& point.y <= wndrect.bottom + SIZE_NEAR_PIXEL && point.y >= wndrect.bottom - SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOMLEFT;
	else if (point.x <= wndrect.left + SIZE_NEAR_PIXEL && point.x >= wndrect.left - SIZE_NEAR_PIXEL)
		sizetype = HTLEFT;
	else if (point.x <= wndrect.right + SIZE_NEAR_PIXEL && point.x >= wndrect.right - SIZE_NEAR_PIXEL)
		sizetype = HTRIGHT;
	else if (point.y <= wndrect.top + SIZE_NEAR_PIXEL && point.y >= wndrect.top - SIZE_NEAR_PIXEL)
		sizetype = HTTOP;
	else if (point.y <= wndrect.bottom + SIZE_NEAR_PIXEL && point.y >= wndrect.bottom - SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOM;
	else if (point.y <= wndrect.bottom + SIZE_NEAR_PIXEL && point.y >= wndrect.bottom - SIZE_NEAR_PIXEL)
		sizetype = 0;
	else
	{
		sizetype = 0;//点击其他区域移动窗口
	}

	return sizetype;
}

inline void SetCursor()
{
	if (sizeEnable)
	{
		mousestate = GetSizeType(cursor);
		switch (mousestate)
		{
		case HTLEFT:
		case HTRIGHT:
			::SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			break;
		case HTTOP:
		case HTBOTTOM:
			::SetCursor(LoadCursor(NULL, IDC_SIZENS));
			break;
		case HTTOPLEFT:
		case HTBOTTOMRIGHT:
			::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			break;
		case HTTOPRIGHT:
		case HTBOTTOMLEFT:
			::SetCursor(LoadCursor(NULL, IDC_SIZENESW));
			break;
		default:
			::SetCursor(LoadCursor(NULL, IDC_ARROW));
			break;
		}
	}
}

void Get2WndRect()
{
	GetClientRect(mainwnd, &clientrect);//得到client区域尺寸
	POINT clienttl = { 0, 0 };
	ClientToScreen(mainwnd, &clienttl);//获得client区域左上角的屏幕坐标
	//得到client真实屏幕区域
	clientrect.left = clienttl.x;
	clientrect.top = clienttl.y;
	/*clientrect.right += clientrect.left;
	clientrect.bottom += clientrect.top;*/
	clientrect.right += clienttl.x;
	clientrect.bottom += clienttl.y;

	GetWindowRect(mainwnd, &wndrect);//得到窗口区域
	wbias = Widthof(wndrect) - Widthof(clientrect);
	hbias = Heightof(wndrect) - Heightof(clientrect);

	//更新clientsize
	clientsize = { Widthof(clientrect), Heightof(clientrect) };

	//修改文字显示区域
	textrect2.left = TEXTMARGIN_SIDE;
	textrect2.right = textrect2.left + 600;
	textrect2.bottom = clientrect.bottom - clientrect.top - TEXTMARGIN_BOTTOM;//底部左侧
	textrect2.top = clientrect.bottom - clientrect.top - 16;
}

bool Init()
{
	//int gwl_style = -16;
	//oldstyle = GetWindowLong(mainwnd, gwl_style);//-1946157056

	//oldstyle = -1946157056;
	////oldstyle = (oldstyle | WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
	////	& ~WS_CAPTION &~WS_BORDER&~WS_SYSMENU;
	//SetWindowLong(mainwnd, gwl_style, oldstyle);
	
	Get2WndRect();

	wlimit = 800;
	hlimit = 500;
	backcolor = COLOR_BKG;
	mode = MODE_PIC;
	loopcount = 0;
	ondrag = false;
	onzoom = false;
	onsize = false;
	onmove = false;
	sizeEnable = true;
	flagshow = true;
	infoshow = true;
	clientsize = { Widthof(clientrect), Heightof(clientrect) };

	QueryPerformanceFrequency(&frequency);
	etime = { 0 };
	stime = { 0 };
	lasttime = nowtime = 0;
	fps = 0;

	maindevice = NULL;
	mainsurf = NULL;
	realzoom = zoom = 1;
	actualzoomx = actualzoomy = realzoom;
	oldzoom = zoom; 
	zoomw = (int)(realzoom*mainbmp.width);
	zoomh = (int)(realzoom*mainbmp.height);
	clip = realzoom >= 1;
	surfsrc.x = 0;
	surfsrc.y = 0;

	//piccount = 0;
	//piclistlen = PICLISTLEN_INIT;
	//piclist = new PicPack[piclistlen];
	//mainpack = NULL;

	infoshow = true;
	textrect.left = TEXTMARGIN_SIDE;
	textrect.top = TEXTMARGIN_TOP;
	textrect.right = 400;
	textrect.bottom = 600;



	return true;
}

bool D3DInit()
{
	HRESULT hr;

	//Direct3D 接口对象
	lpD3D = Direct3DCreate9(D3D_SDK_VERSION);

	//caps
	if (FAILED(lpD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps)))
		MessageBoxW(mainwnd, L"get caps FAILED!", L"", 0);


	//display mode
	hr = lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displaymode);
	if (FAILED(hr))
		MessageBoxW(mainwnd, L"get displaymode FAILED!", L"", 0);

	//D3DPRESENT_PARAMETERS
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = Widthof(clientrect);
	d3dpp.BackBufferHeight = Heightof(clientrect);
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;//加速，displaymode.Format
	d3dpp.BackBufferCount = 2;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = mainwnd;
	d3dpp.Windowed = TRUE;
	d3dpp.EnableAutoDepthStencil = FALSE;//深度缓冲
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;//加速用D3DFMT_D24S8不用D3DFMT_D16
	d3dpp.Flags = 0;
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
	
	InitDevice();

	D3DXCreateFontW(
		maindevice,
		13, 5, 0, 1000, 0,			// 字体字符的宽高、是否加粗、Mipmap级别、是否为斜体	
		DEFAULT_CHARSET,			// 默认字符集
		OUT_DEFAULT_PRECIS,			// 输出精度，使用默认值
		CLEARTYPE_NATURAL_QUALITY,	// 文本质量NONANTIALIASED_QUALITY/CLEARTYPE_NATURAL_QUALITY
		DEFAULT_PITCH | FF_DONTCARE,
		L"Arial Rounded MT Bold",	//Arial Rounded MT Bold
		&font
	);
	D3DXCreateFontW(
		maindevice,
		16, 7, 0, 1, 0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		NONANTIALIASED_QUALITY,//NONANTIALIASED_QUALITY, DEFAULT_QUALITY
		DEFAULT_PITCH | FF_DONTCARE,
		L"Calibri",
		&font2
	);

	return true;
}

bool InitDevice()
{
	HRESULT hr = lpD3D->CreateDevice(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mainwnd
		, vertexprocessing, &d3dpp, &maindevice
	);

	if (FAILED(hr))
	{
		//MessageBoxW(mainwnd, L"Device create FAILED!", L"", 0);
		return false;
	}

	maindevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	return true;
}

bool OnDropFile(WCHAR file[])
{
	if (!maindevice)
	{
		//if (!ResetDevice())
			return false;
	}

	//获取图像信息
	HRESULT hr;
	ZeroMemory(&imginfo, sizeof(D3DXIMAGE_INFO));
	D3DXGetImageInfoFromFile(file, &imginfo);
	//获取其余信息
	if (!imginfo0.GetFile(file))
	{
		MessageBox(mainwnd, L"info read failed", L"1", 0);
		return false;
	}

	//清除非空表面
	if (tempsurf)
		tempsurf->Release();
	tempsurf = NULL;
	//创建与图像匹配表面
	hr = maindevice->CreateOffscreenPlainSurface(
		(LONG)imginfo.Width, (LONG)imginfo.Height, D3DFMT_A8R8G8B8
		, D3DPOOL_SYSTEMMEM, &tempsurf, NULL);
	if (FAILED(hr))
		MessageBoxW(mainwnd, L"CreateOffscreenPlainSurface FAILED!", L"", 0);
	//装载图像
	hr = D3DXLoadSurfaceFromFileW(
		tempsurf, NULL, NULL, file
		, NULL, D3DX_FILTER_NONE, 0x00000000, NULL);
	if (FAILED(hr))
	{
		MessageBoxW(mainwnd, L"D3DXLoadSurfaceFromFileW FAILED!", L"", 0);
		return false;
	}
	//存入BMP
	mainbmp.Load(tempsurf);
	zoomw = (int)(realzoom*mainbmp.width);
	zoomh = (int)(realzoom*mainbmp.height);
	//清除surface
	tempsurf->Release();
	tempsurf = NULL;

	//将图片居中
	CenterPic();
	//导入为surface
	RefreshSurf();
	//渲染
	Render();
	//保存（测试）
	//D3DXSaveSurfaceToFile(L"E:\\1.bmp", D3DXIFF_BMP, mainsurf, NULL, NULL);

	//piclist[0].Read(maindevice, file);
	//mainpack = &(piclist[0]);

	SetWindowTextW(mainwnd, file);

	return true;
}

bool OnLoadFile(LPWSTR file)
{
	LPWSTR *szArgList;
	int argCount;
	szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);

	//MessageBoxW(mainwnd, szArgList[argCount - 1], L"", 0);
	if(argCount!=1)
		OnDropFile(szArgList[argCount - 1]);

	return false;
}

inline void DelayFlag()
{
	time_t nowtick = GetTickCount64();

	if (onzoom)
	{
		if (nowtick < zoomtick || nowtick - zoomtick>ZOOMFLAG_DELAY)
			onzoom = false;
	}
	if (onsize)
	{
		if (nowtick < sizetick || nowtick - sizetick>SIZEFLAG_DELAY)
			onsize = false;
	}
	if (onmove)
	{
		if (nowtick < movetick || nowtick - movetick>MOVEFLAG_DELAY)
			onmove = false;
	}
}

inline void ClearFlag()
{
	ondrag = false;
	onzoom = false;
	onsize = false;
}

bool FitWnd()
{
	wndrect.right = wndrect.left + wlimit + wbias;
	wndrect.bottom = wndrect.top + wlimit * (LONG)imginfo.Height / (LONG)imginfo.Width + hbias;
	if (wndrect.bottom - wndrect.top > hlimit)
	{
		wndrect.bottom = wndrect.top + hlimit + hbias;
		wndrect.right = wndrect.left + hlimit * (LONG)imginfo.Width / (LONG)imginfo.Height + wbias;
	}

	MoveWindow(mainwnd, wndrect.left, wndrect.top
		, wndrect.right - wndrect.left
		, wndrect.bottom - wndrect.top, TRUE);

	return true;
}

bool InfoRender()
{
	WCHAR subinfo[200] = { 0 };

	//显示状态
	if(mainbmp.Empty())
		swprintf_s(infowstr, _T("pic: FALSE"));
	else
	{
		if(outsideclient)
			swprintf_s(infowstr, _T("pic: TRUE (OUTSIDE SCREEN！)"));
		else
			swprintf_s(infowstr, _T("pic: TRUE"));

		if (/*!mainbmp.Empty() && */!mainsurf)//mainbmp存在但surface不存在
		{
			wcscat_s(infowstr, L"  ");
			swprintf_s(subinfo, _T("SURFACE FAILED!"));
			wcscat_s(infowstr, subinfo);
		}
	}
	wcscat_s(infowstr, L"\n");

	//图像尺寸
	swprintf_s(subinfo, _T("pic size: %d× %d  zoom: %.3f/%.3f"), mainbmp.width, mainbmp.height, zoom, realzoom);
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//图像格式
	swprintf_s(subinfo, _T("format: %S"), GetFMT(imginfo.Format).c_str());
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//图像文件大小
	swprintf_s(subinfo, _T("file size: %.3lf KB"), (double)imginfo0.bytecount / 1000);
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//surface尺寸
	if (mainsurf)
	{
		swprintf_s(subinfo, _T("intended surface: %d× %d")
			, zoomw, zoomh);
		wcscat_s(infowstr, subinfo);
		wcscat_s(infowstr, L"\n");

		swprintf_s(subinfo, _T("surface: %d× %d  actzoom: X %.4f Y %.4f")
			, (LONG)surfDesc.Width, (LONG)surfDesc.Height, actualzoomx, actualzoomy);
	}
	else
	{
		swprintf_s(subinfo, _T("intended surface: -×-\n"));
		wcscat_s(infowstr, subinfo);
		swprintf_s(subinfo, _T("surface: -×-"));
	}

	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//surface起始点
	swprintf_s(subinfo, _T("surface base: %d, %d"), -surfsrc.x, -surfsrc.y);
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n"); 
		
	//clipsurface起始点
	swprintf_s(subinfo, _T("clipsurface base: %d, %d"), surfbase.x, surfbase.y);
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");


	//buffer尺寸
	swprintf_s(subinfo, _T("buffer: %d× %d"), d3dpp.BackBufferWidth, d3dpp.BackBufferHeight);
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//fps
	swprintf_s(subinfo, _T("fps: %.1f"), fps);
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//计数
	/*swprintf_s(subinfo, _T("loops: %lld"), loopcount);
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");*/

	//背景色
	swprintf_s(subinfo, _T("backcolor: %08X.ARGB"), backcolor);
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//鼠标像素位置#1
	swprintf_s(subinfo, _T("cursor pos: %S"), cursorposshow[cursorpos].c_str());
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//鼠标像素位置#2
	if(NoPic())
		swprintf_s(subinfo, _T("cursor pixel: -, -"));
	else
		swprintf_s(subinfo, _T("cursor pixel: %d, %d"), curpixel.x, curpixel.y);
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//鼠标像素颜色
	swprintf_s(subinfo, _T("pixel color: %02X.%06X.ARGB")
		, (pixelcolor >> 24), (pixelcolor & 0xFFFFFF));
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n"); 

	//鼠标像素颜色
	swprintf_s(subinfo, _T("screen color: %02X.%06X.ARGB")
		, (pixelcolor >> 24), (screencolor & 0xFFFFFF));
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//鼠标位置
	swprintf_s(subinfo, _T("cursor: %d, %d"), cursor.x, cursor.y);
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//鼠标相对窗口客户区位置
	swprintf_s(subinfo, _T("cursor client: %d, %d")
		, cursor.x - clientrect.left, cursor.y - clientrect.top);
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"\n");

	//font->DrawTextW(NULL, infowstr, -1, &textrect, DT_LEFT | DT_NOCLIP, COLOR_TEXT0);
	font->DrawTextW(NULL, infowstr, -1, &textrect, DT_LEFT | DT_NOCLIP, COLOR_TEXT1);



	wcscpy_s(infowstr, L"");
	//标志
	if (!mainbmp.Empty())
		swprintf_s(subinfo, _T("pic: √"));
	else
		swprintf_s(subinfo, _T("pic: ×"));
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"    ");

	if (mainsurf)
		swprintf_s(subinfo, _T("surf: √"));
	else
		swprintf_s(subinfo, _T("surf: ×"));
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"    ");

	if (onzoom)
		swprintf_s(subinfo, _T("onzoom: √"));//○√×
	else
		swprintf_s(subinfo, _T("onzoom: ×"));
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"    ");

	if (ondrag)
		swprintf_s(subinfo, _T("ondrag: √"));
	else
		swprintf_s(subinfo, _T("ondrag: ×"));
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"    ");

	if (onsize)
		swprintf_s(subinfo, _T("onsize: √"));
	else
		swprintf_s(subinfo, _T("onsize: ×"));
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"    ");

	if (onmove)
		swprintf_s(subinfo, _T("onmove: √"));
	else
		swprintf_s(subinfo, _T("onmove: ×"));
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"    ");

	if (clip)
		swprintf_s(subinfo, _T("clipon: √"));
	else
		swprintf_s(subinfo, _T("clipon: ×"));
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"    ");

	if (surfclipped)
		swprintf_s(subinfo, _T("surfclipped: √"));
	else
		swprintf_s(subinfo, _T("surfclipped: ×"));
	wcscat_s(infowstr, subinfo);
	wcscat_s(infowstr, L"    ");

	if (picclipped)
		swprintf_s(subinfo, _T("picclipped: √"));
	else
		swprintf_s(subinfo, _T("picclipped: ×"));
	wcscat_s(infowstr, subinfo); 
	wcscat_s(infowstr, L"    ");

	if (outsideclient)
		swprintf_s(subinfo, _T("picout: √"));
	else
		swprintf_s(subinfo, _T("picout: ×"));
	wcscat_s(infowstr, subinfo); 
		wcscat_s(infowstr, L"    ");

	font2->DrawTextW(NULL, infowstr, -1, &textrect2, DT_LEFT | DT_TOP | DT_NOCLIP, COLOR_TEXT1);

	return true;
}

inline bool LoadBackbuffer()
{
	//获得backbuffer
	LPDIRECT3DSURFACE9 backbuffer = NULL;
	HRESULT hr = maindevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	if (FAILED(hr))
	{
		//MessageBoxW(mainwnd, L"GetBackBuffer FAILED!", L"", 0);
		return false;
	}
	//背景
	/*const short rwidth = 8;
	RECT rect = {0, 0, rwidth, rwidth};
	for (int i = 0; (i-1)*rwidth < Heightof(clientrect); i++)
	{
		for (int j = 0; (j-1)*rwidth < Widthof(clientrect); j+=2)
		{
			if ((i & 0x01) == 0)
				rect = { j*rwidth,i*rwidth,j*rwidth + rwidth,i*rwidth + rwidth };
			else
				rect = { (j + 1)*rwidth,i*rwidth,(j + 1)*rwidth + rwidth,i*rwidth + rwidth };
			maindevice->ColorFill(backbuffer, &rect, D3DCOLOR_XRGB(204, 203, 204));
		}
	}*/
	//拷贝到backbuffer
	//if (clip)
	//{
	//	RECT surfrect;
	//	surfrect.left = 0;//如果起始点surfsrc某一维小于0，则平移区域到0起点，并设置destpoint为非0
	//	surfrect.top = 0;
	//	surfrect.right = min(Widthof(clientrect) - surfbase.x, surfDesc.Width);//如果起始点右下区域不够窗口客户区，则削减右侧和下侧
	//	surfrect.bottom = min(Heightof(clientrect) - surfbase.y, surfDesc.Height);

	//	hr = maindevice->UpdateSurface(mainsurf, &surfrect, backbuffer, &surfbase);//新surface方法
	//}
	//else
	//{
	//	//surface拷贝区域，旧surface方法
	//	POINT destpoint;
	//	RECT surfrect;
	//	if (surfsrc.x < 0)
	//		destpoint.x = -surfsrc.x;
	//	else
	//		destpoint.x = 0;
	//	if (surfsrc.y < 0)
	//		destpoint.y = -surfsrc.y;
	//	else
	//		destpoint.y = 0;
	//	surfrect.left = max(surfsrc.x, 0);//如果起始点surfsrc某一维小于0，则平移区域到0起点，并设置destpoint为非0
	//	surfrect.top = max(surfsrc.y, 0);
	//	surfrect.right = min(zoomw, surfrect.left + Widthof(clientrect) - destpoint.x);//如果起始点右下区域不够窗口客户区，则削减右侧和下侧
	//	surfrect.bottom = min(zoomh, surfrect.top + Heightof(clientrect) - destpoint.y);

	//	hr = maindevice->UpdateSurface(mainsurf, &surfrect, backbuffer, &destpoint);//旧surface方法
	//}
	hr = maindevice->UpdateSurface(mainsurf, &surfrect, backbuffer, &surfbase);//旧surface方法

	//maindevice->StretchRect(mainsurf, 0, backbuffer, 0, D3DTEXF_NONE);
	if (backbuffer)
		backbuffer->Release();
}

bool Render()
{
	/*if (!maindevice)
	{
		if (!ResetDevice())
			return false;
	}*/

	HRESULT hr;

	//绘制
	maindevice->Clear(0, NULL, D3DCLEAR_TARGET, backcolor, 1.0f, 0);
	maindevice->BeginScene();
	if (!NoPic())
	{
		//surface超出窗口区域判断(与窗口客户区无交集),终止渲染
		if (//区域超出surface右或下
			surfsrc.x > zoomw || surfsrc.y > zoomh
			//区域右下不够surface左或上
			|| surfsrc.x + Widthof(clientrect) <= 0 || surfsrc.y + Heightof(clientrect) <= 0
			)
		{
			outsideclient = true;
		}
		else
		{
			LoadBackbuffer();//surface装载到backbuffer
			outsideclient = false;
		}
	}
	if (infoshow)
		InfoRender();
	maindevice->EndScene();
	hr = maindevice->Present(NULL, NULL, NULL, NULL);

	//处理设备丢失
	if (hr == D3DERR_DEVICELOST)
	{
		if (maindevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ResetDevice();
		}
	}

	return true;
}

void CenterPic()
{
	if (zoomw <= d3dpp.BackBufferWidth
		&& zoomh <= d3dpp.BackBufferHeight)
	{
		surfsrc.x = (LONG)(zoomw / 2 - d3dpp.BackBufferWidth / 2);
		surfsrc.y = (LONG)(zoomh / 2 - d3dpp.BackBufferHeight / 2);
	}
	else
	{
		surfsrc.x = 0;
		surfsrc.y = 0;
	}
}

void Togglezoom(WPARAM wParam)
{
	float adds = 0;
	QueryPerformanceCounter(&wheeltick);

	if (zoom<1)
		adds = (float)(frequency.QuadPart / (wheeltick.QuadPart - lastwheeltick.QuadPart)
			*(0.012*zoom));//增长速度与时间和放大倍率有关
	else
		adds = (float)(frequency.QuadPart / (wheeltick.QuadPart - lastwheeltick.QuadPart)
			*(0.002*zoom));//增长速度与时间和放大倍率有关
	lastwheeltick = wheeltick;
	//>=1放大倍率时特殊处理
	if (zoom >= 1 && adds < 1)
	{
		if ((short)HIWORD(wParam) > 0)//增加
			adds = 1;
		else//减少
		{
			if (zoom >= 2)
				adds = 1;
			else if (zoom > 1)
				adds = zoom - 1;
		}
	}
	//adds限制
	if (adds < MIN_DELTAZOOM)
		adds = MIN_DELTAZOOM;
	if ((short)HIWORD(wParam) < 0)
		adds = -adds;//adds取反

					 //zoom += (short)HIWORD(wParam) / 120;

	if (adds > 0)
	{
		zoom += adds;
	}
	else
	{
		zoom += adds;
	}
	//zoom上下限
	if (zoom > MAX_ZOOM)
		zoom = MAX_ZOOM;
	if (zoom*min(mainbmp.width, mainbmp.height) < 1)
		zoom = 1.0f / min(mainbmp.width, mainbmp.height);

	//设置实际使用的zoom
	if (zoom >= 1)
		realzoom = roundf2ui16(zoom);
	else
		realzoom = zoom;
	onzoom = true;//设置标志
	zoomtick = GetTickCount64();
}

bool Zoom(float oldzoom, float zoom)
{
	zoomw = (int)(zoom*mainbmp.width);
	zoomh = (int)(zoom*mainbmp.height);

	actualzoomx = (float)zoomw / mainbmp.width;
	actualzoomy = (float)zoomh / mainbmp.height;

	if (oldzoom == zoom)
		return false;

	UINT oldswidth = (UINT)(mainbmp.width*oldzoom), oldsheight = (UINT)(mainbmp.height*oldzoom);
	/*POINT cursorpos;
	GetCursorPos(&cursorpos);*/
	//客户区的屏幕坐标
	POINT clienttl = { 0, 0 };
	ClientToScreen(mainwnd, &clienttl);
	//鼠标相对客户区坐标
	POINT cursortoclient;
	cursortoclient.x = cursor.x - clienttl.x;
	cursortoclient.y = cursor.y - clienttl.y;
	//鼠标位置相对surface的坐标
	POINT cursortosurface;
	cursortosurface.x = surfsrc.x + cursortoclient.x;
	cursortosurface.y = surfsrc.y + cursortoclient.y;
	//调整起始点#1
	short altstate = GetAsyncKeyState(VK_MENU);
	if (altstate & 0x8000)
	{
		surfsrc.x = surfsrc.x + ((zoom - oldzoom)*mainbmp.width) / 2;
		surfsrc.y = surfsrc.y + ((zoom - oldzoom)*mainbmp.height) / 2;
	}
	else
	{
		surfsrc.x = (LONG)(roundf2l((float)cursortosurface.x*zoomw / oldswidth) - cursortoclient.x);
		surfsrc.y = (LONG)(roundf2l((float)cursortosurface.y*zoomh / oldsheight) - cursortoclient.y);
	}
	//调整起始点#2保持鼠标所处像素位置(需要设置变量记录需要保持的鼠标像素，否则误差大)
	/*surfsrc.x = round(curpixel.x*realzoom - cursortoclient.x);
	surfsrc.y = round(curpixel.y*realzoom - cursortoclient.y);*/

	//如果在放大阶段，窗口客户区可以完全容纳surface，则控制surface显示区域
	//防止小图片放大迅速偏离窗口客户区
	if (zoomw <= d3dpp.BackBufferWidth
		&& zoomh <= d3dpp.BackBufferHeight
		&& zoom > oldzoom)
	{
		if (surfsrc.x > 0)
			surfsrc.x = 0;
		if (surfsrc.y > 0)
			surfsrc.y = 0;
		if (surfsrc.x < (LONG)zoomw - (LONG)d3dpp.BackBufferWidth)
			surfsrc.x = (LONG)zoomw - (LONG)d3dpp.BackBufferWidth;
		if (surfsrc.y < (LONG)zoomh - (LONG)d3dpp.BackBufferHeight)
			surfsrc.y = (LONG)zoomh - (LONG)d3dpp.BackBufferHeight;
	}
	//设置clip标志
	clip = (realzoom >= 1)
		|| (realzoom < 1 && (zoomw > MAX_FORCECLIP_WIDTH || zoomh > MAX_FORCECLIP_HEIGHT));

	RefreshSurf();

	GetCurInfo();//更新所处像素位置,因为zoom可能导致微小偏移

	if (!mainsurf)
		return false;

	return true;
}

bool RefreshSurf()
{
	//mainbmp.CreateSurf(maindevice, &mainsurf, backcolor, zoom);//旧surface方法
	mainbmp.CreateSurf_Clip(maindevice, &mainsurf
		, surfsrc, clientsize, surfbase, surfclipped
		, realzoom, clip, backcolor);//新surface方法

	if (mainsurf)
	{
		mainsurf->GetDesc(&surfDesc);
		CalcSurfMapInfo();//更新surface拷贝到backbuffer参数
		return true;
	}
	else
		return false;
}

inline void DisplayStatusChange(bool winchange, bool surfchange)
{
	if(winchange)
		Get2WndRect();//重新计算窗口大小

	if (surfchange)
		RefreshSurf();//重新生成图片

	if (surfchange || winchange)
		CalcClipped();//计算图片clip状态

	if (surfchange)
		CalcSurfMapInfo();//更新surface拷贝到backbuffer参数
}

inline void SurfStatusChange()
{
	CalcClipped();//计算图片clip状态

	CalcSurfMapInfo();//更新surface拷贝到backbuffer参数
}


inline void CalcSurfMapInfo()
{
	if (clip)
	{
		surfrect.left = 0;//如果起始点surfsrc某一维小于0，则平移区域到0起点，并设置destpoint为非0
		surfrect.top = 0;
		surfrect.right = min(Widthof(clientrect) - surfbase.x, surfDesc.Width);//如果起始点右下区域不够窗口客户区，则削减右侧和下侧
		surfrect.bottom = min(Heightof(clientrect) - surfbase.y, surfDesc.Height);
	}
	else
	{
		//surface拷贝区域，旧surface方法
		if (surfsrc.x < 0)
			surfbase.x = -surfsrc.x;
		else
			surfbase.x = 0;
		if (surfsrc.y < 0)
			surfbase.y = -surfsrc.y;
		else
			surfbase.y = 0;
		surfrect.left = max(surfsrc.x, 0);//如果起始点surfsrc某一维小于0，则平移区域到0起点，并设置destpoint为非0
		surfrect.top = max(surfsrc.y, 0);
		surfrect.right = min(zoomw, surfrect.left + Widthof(clientrect) - surfbase.x);//如果起始点右下区域不够窗口客户区，则削减右侧和下侧
		surfrect.bottom = min(zoomh, surfrect.top + Heightof(clientrect) - surfbase.y);
	}
}

inline void CalcClipped()
{
	POINT surfsize = { zoomw, zoomh };//预计的surface尺寸，用作clip判断
	surfsize.y = zoomh;
	if (surfsrc.x > 0)//surface左侧需clip
	{
		surfsize.x -= surfsrc.x;
	}
	if (surfsrc.y > 0)//surface上侧需clip
	{
		surfsize.y -= surfsrc.y;
	}
	if (zoomw - surfsrc.x > Widthof(clientrect))//surface右侧需clip
		surfsize.x -= (zoomw - surfsrc.x - Widthof(clientrect));
	if (zoomh - surfsrc.y > Heightof(clientrect))//surface下侧需clip
		surfsize.y -= (zoomh - surfsrc.y - Heightof(clientrect));
	if (surfsize.x < zoomw || surfsize.y < zoomh)
		picclipped = true;
	else
		picclipped = false;
}

inline void GetCurInfo()
{
	/*POINT cursor;
	GetCursorPos(&cursor);*/

	//客户区的屏幕坐标
	POINT clienttl = { 0, 0 };
	ClientToScreen(mainwnd, &clienttl);
	//鼠标相对客户区坐标
	POINT cursortoclient;
	cursortoclient.x = cursor.x - clienttl.x;
	cursortoclient.y = cursor.y - clienttl.y;
	//鼠标位置相对surface的坐标=surfsrc.x + cursortoclient.x,这里不用四舍五入
	curpixel.x = (LONG)((surfsrc.x + cursortoclient.x) / actualzoomx);
	curpixel.y = (LONG)((surfsrc.y + cursortoclient.y) / actualzoomy);

	//获得像素位置后续信息的获取
	GetCurColor();
}

inline void GetCurPos()
{
	//客户区的屏幕坐标
	POINT clienttl = { 0, 0 };
	ClientToScreen(mainwnd, &clienttl);
	//鼠标相对客户区坐标
	POINT cursortoclient;
	cursortoclient.x = cursor.x - clienttl.x;
	cursortoclient.y = cursor.y - clienttl.y;

	//获得当前鼠标位置（种类）
	POINT cursortosurf = { surfsrc.x + cursortoclient.x, surfsrc.y + cursortoclient.y };
	RECT surfrect = { 0, 0, zoomw - 1, zoomh - 1 };
	if (!inside(cursor, clientrect))
		cursorpos = CURSORPOS_OUTWINDOW;
	else if (NoPic())
		cursorpos = CURSORPOS_BLANK;
	else if (/*curpixel.x < 0 || curpixel.x + 1>mainbmp.width
			 || curpixel.y < 0 || curpixel.y + 1>mainbmp.height*/
		!inside(cursortosurf, surfrect))
		cursorpos = CURSORPOS_BLANK;
	else
		cursorpos = CURSORPOS_PIC;
}

inline bool GetCurColor()
{
	if (NoPic())
	{
		pixelcolor = 0;
		return false;
	}

	if (curpixel.x < 0 || curpixel.x + 1>mainbmp.width
		|| curpixel.y < 0 || curpixel.y + 1>mainbmp.height
		|| !inside(cursor, clientrect))//所处像素超出图片范围获鼠标超出窗口范围
	{
		pixelcolor = 0;
		return false;
	}
	else
		return mainbmp.GetPixel(curpixel.x, curpixel.y, &pixelcolor);

}

inline bool NoPic()
{
	return (mainbmp.Empty() || !mainsurf);
}

inline void Clear()
{
	mainbmp.Clear();
	if (mainsurf)
		mainsurf->Release();
	mainsurf = NULL;
	ZeroMemory(&imginfo, sizeof(D3DXIMAGE_INFO));
	ZeroMemory(&imginfo0, sizeof(PicInfo));
}

bool ResetDevice()
{
	if (!maindevice)
		return false;

	d3dpp.BackBufferWidth = clientrect.right - clientrect.left;
	d3dpp.BackBufferHeight = clientrect.bottom - clientrect.top;
	/*if(maindevice)
		maindevice->Release();
	if (!InitDevice())
		return false;*/

	//修改device长宽
	OnLostDevice();
	return OnResetDevice();

	//font->Release();
	//D3DXCreateFontW(
	//	maindevice,
	//	13, 5, 0, 1000, 0,			// 字体字符的宽高、是否加粗、Mipmap级别、是否为斜体	
	//	DEFAULT_CHARSET,        // 默认字符集
	//	OUT_DEFAULT_PRECIS,     // 输出精度，使用默认值
	//	CLEARTYPE_NATURAL_QUALITY,	// 文本质量NONANTIALIASED_QUALITY/CLEARTYPE_NATURAL_QUALITY
	//	DEFAULT_PITCH | FF_DONTCARE,
	//	L"Arial Rounded MT Bold",
	//	&font
	//);

	//return true;
}

void OnLostDevice()
{
	font->OnLostDevice();
	font2->OnLostDevice();
	/*if(mainsurf)
		mainsurf->Release();*/
}

bool OnResetDevice()
{
	if (FAILED(maindevice->Reset(&d3dpp)))
	{
		return false;
	}
	font2->OnResetDevice();
	font->OnResetDevice();

	return true;
}

string GetFMT(D3DFORMAT fmt)
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

inline int Widthof(const RECT & rect)
{
	return rect.right - rect.left;
}

inline int Heightof(const RECT & rect)
{
	return rect.bottom - rect.top;
}

inline bool inside(const POINT & point, const RECT & rect)
{
	if (point.x<rect.left || point.x>rect.right
		|| point.y<rect.top || point.y>rect.bottom)
		return false;
	else
		return true;
}
