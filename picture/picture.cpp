#include "stdafx.h"
#include "picture.h"

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明: 
//ATOM                MyRegisterClass(HINSTANCE hInstance);
//BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PICTURE, szWindowClass, MAX_LOADSTRING);

	//舍弃的方法
    //MyRegisterClass(hInstance);
    //// 执行应用程序初始化: 
    //if (!InitInstance (hInstance, nCmdShow))
    //{
    //    return FALSE;
    //}

	//初始化窗口
	pd3dwnd = new D3DWnd();
	if (!pd3dwnd->D3DCreateWindow(szTitle, WndProc, hInstance
		, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS/* | CS_DROPSHADOW*/
		, WS_OVERLAPPEDWINDOW, WS_EX_ACCEPTFILES
		, WINDOWPOSX_INIT, WINDOWPOSY_INIT, WINDOWWIDTH_INIT, WINDOWHEIGHT_INIT
		, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PICTURE))
		, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL))
		, /*MAKEINTRESOURCEW(IDC_PICTURE)*/0
		, szWindowClass, COLOR_BKG_INIT))
		return FALSE;

	mainwnd = pd3dwnd->GetHWND();//存储主窗口句柄

	//初始化参数
	Init();
	D3DInit();
	//SetTimer(mainwnd, 0, STATIC_FRAMETIME_NORMAL, TimerProc);

	//初始窗口参数处理
	WCHAR *param = 0;
	param = GetCommandLine();
	OnWininitFile(param);

	//舍弃的方法
    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PICTURE));

	//事件捕捉
	bool lastonzoom = false;

    // 主消息循环: 
	MSG msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
	while (msg.message != WM_QUIT)
	{
		/*
		 * 在发布消息前做的动作
		 */
		//更新鼠标坐标,在发布消息前更新鼠标位置，防止消息处理函数中数据错误(图片拖动问题)
		GetCursorPos(&cursor);

		//消息
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		/*
		 * 发布消息后要做的动作
		 */
		if (IsIconic(mainwnd))
		{
			Sleep(ICONIC_SLEEP);//最小化状态降低资源消耗
			continue;
		}

		//计数
		loopcount++;
		time(&nowtime);

		bool oldsize = onsize;
		DelayFlag();//更新延迟的标志

		//帧率限制在主循环主要任务（Render）之后做会造成窗口响应慢
		QueryPerformanceCounter(&etime);
		float testfps;
		if (fpslimit)
		{
			if (etime.QuadPart != stime.QuadPart)
				testfps = (float)frequency.QuadPart / (etime.QuadPart - stime.QuadPart);
			if (dragging || onzoom || draggingzoom)//图片需快速更新
			{
				short ftdiff = (short)(NORMAL_FRAMETIME - 1000.0f / testfps);
				if (/*testfps > NORMAL_FPS*/
					ftdiff >= 1)
				{
					//Sleep(ftdiff / 2);
					Sleep(1);
					continue;
				}
			}
			else//静止状态，低帧率刷新界面
			{
				short ftdiff = (short)(staticframetime - 1000.0f / testfps + 0.5f);
				if (testfps > staticfps*1.05f)
				{
					Sleep(max(ftdiff / 2, 1));
					//较准确，cpu占用率小的方法
					//Sleep(1);
					continue;
				}
			}
		}

		if (nowtime != lasttime)//每秒执行
		{
			lasttime = nowtime;
			surfer.surfrefreshcount = 0;

			//计算内存占用 
			PROCESS_MEMORY_COUNTERS pmc;
			GetProcessMemoryInfo(hprocess, &pmc, sizeof(pmc));
			memoryin = pmc.WorkingSetSize / B_TO_MB;//内存占用量
			memoryout = pmc.PagefileUsage / B_TO_MB;//虚拟内存占用量

			//计算fps
			fpscount++;
			QueryPerformanceCounter(&etime);
			fps = (float)frequency.QuadPart / (etime.QuadPart - stime.QuadPart);
			
			frametime = 1000.0f / fps;				//帧时间
			if (avgfps < 0)							//平均帧率
				avgfps = fps;
			else
				avgfps = avgfps*0.9f + fps*0.1f;
			if (cvgfps < 0)							//收敛帧率
				cvgfps = fps;
			else
				cvgfps = cvgfps*fpscount / (fpscount + 1) + fps / (fpscount + 1);

		}
		//开始计时
		stime.QuadPart = etime.QuadPart;


		//事件捕捉
		if (oldsize)
		{
			if (!onsize && needforcerenew)
			{
				PostMessage(mainwnd, WM_SURFFORCERENEW, 0, 1);
			}
		}
		//事件捕捉
		if (lastonzoom)
		{
			if (!onzoom && needforcerenew)
			{
				lastonzoom = false;
				PostMessage(mainwnd, WM_SURFFORCERENEW, 0, 1);
			}
		}
		else if (onzoom)
		{
			lastonzoom = true;
		}

		//渲染
		Render();
    }
    return (int) msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//ATOM MyRegisterClass(HINSTANCE hInstance)
//{
//    WNDCLASSEXW wcex;
//
//    wcex.cbSize = sizeof(WNDCLASSEX);
//
//    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
//    wcex.lpfnWndProc    = WndProc;
//    wcex.cbClsExtra     = 0;
//    wcex.cbWndExtra     = 0;
//    wcex.hInstance      = hInstance;
//    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PICTURE));
//    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
//    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
//    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PICTURE);
//    wcex.lpszClassName  = szWindowClass;
//    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
//
//    return RegisterClassExW(&wcex);
//}

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
//BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
//{
//   hInst = hInstance; // 将实例句柄存储在全局变量中
//
//   HWND hWnd = CreateWindowExW(WS_EX_ACCEPTFILES, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
//	   WINDOWPOSX_INIT, WINDOWPOSY_INIT, WINDOWWIDTH_INIT, WINDOWHEIGHT_INIT, nullptr, nullptr, hInstance, nullptr);
//
//   if (!hWnd)
//   {
//      return FALSE;
//   }
//   //mainwnd = hWnd;
//
//   ShowWindow(hWnd, nCmdShow);
//   UpdateWindow(hWnd);
//
//   return TRUE;
//}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT uFileNum;

    switch (message)
    {
	//  WM_COMMAND  - 处理应用程序菜单
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
				OpenFileWin();
				break;
			case IDM_SAVE:
				SaveFileWin();
				break;
			case IDM_CLEAR:
				Clear();
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT://高效，重绘
        {
			if (!IsIconic(mainwnd))
			{
				Render();//使得调整窗口尺寸时自动重绘（调整窗口尺寸时不进入主循环）

				//不可以去掉下面4行(尽管没有添加绘图代码)，会导致帧率降低
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				// TODO: 在此处添加使用 hdc 的任何绘图代码...
				EndPaint(hWnd, &ps);
			}
			else
			{
				;
			}
        }
        break; 
	//case WM_RESETDEVICE:
	//	ResetDevice();//处理设备丢失
	//	break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_DROPFILES://拖入文件
		uFileNum = ::DragQueryFile((HDROP)wParam, 0xffffffff, NULL, 0);
		::DragQueryFileW((HDROP)wParam, 0, strFileName, MAX_PATH);//获取文件名
		OnLoadFile(strFileName);
		break;
	case WM_SIZE://*********************高效，重绘*********************
		if (!IsIconic(mainwnd))
		{
			GetCursorPos(&cursor);//size时不进入主循环，需获取鼠标位置
			OnWinChange();//更新窗口区域，如果改为size结束动作时做，TODO：设置另一窗口区域信息实时更新
			MaintainWindowStyle();//维持窗口样式

			ResetDevice();

			//surface更新
			needforcerenew = surfer.OnWinsize_Custom();

			/*
			 * 标志
			 */
			if (needforcerenew)
			{
				surfrefresh = true;
				surfrefreshtick = GetTickCount64();
			}
			onsize = true;
			sizetick = GetTickCount64();
		}
		else
		{
			;
		}
		break;
	case WM_MOVE://高效
		GetCursorPos(&cursor);//WM_MOVE时不进入主循环，需要更新鼠标位置
		if (!onsize)
		{
			//更新窗口区域
			OnWinChange();//size时不做，(尽管WM_MOVE先于WM_SIZE)

			//不重绘，防止窗口尺寸跳变时渲染有跳变。
			//Render();//size时的move消息不重绘，在onpaint重绘
		}
		break;
	case WM_LBUTTONDBLCLK:
		if (iswindowedfullscreen)
		{
			FullScreen_Windowed(!iswindowedfullscreen);
		}
		else
			PostMessage(mainwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		break;
	case WM_LBUTTONDOWN:
		if (GetAsyncKeyState(VK_CONTROL) & KEYSTATEMASK_DOWN)
		{
			if (iswindowedfullscreen)
			{
				FullScreen_Windowed(!iswindowedfullscreen);
			}
			else
				ClickDragWindow_Custom(cursor.x, cursor.y);
		}
		else
		{
			if (mainbmp->isNotEmpty())
			{
				BeginDragPic();//开始拖动
			}
		}

		CustomWinSizeProc();//自定义窗口拉伸功能（如果结束拖动）
		break;
	case WM_RBUTTONDOWN:
		if (mainbmp->isNotEmpty())
		{
			BeginDragZoomPic();//开始拖动
		}
		break;
	case WM_RBUTTONUP:
		if (ondragzoom)
		{
			EndDragZoomPic();
		}
		break;
	case WM_LBUTTONUP:
		//如果在拖动图片状态，退出拖动状态
		if (ondrag)
		{
			EndDragPic();
		}
		else if (onsize)
		{
			if (needforcerenew)//如果窗口拉伸过程中需要更新图片，在结束时用合适方法重新生成surface
				PostMessage(mainwnd, WM_SURFFORCERENEW, 0, 1);
		}
		break;
	case WM_SURFFORCERENEW://强制刷新surface，用合适的生成方法
		if (!ondrag && !onzoom && !onsize)//仅在适当时候（静止时）更新
		{
			//经测试三种状态结束后，都可以成功进这里，保证needforcerenew更新
			surfer.SurfRenew(false);
			needforcerenew = false;
			
			surfer.GetCurInfo(cursor, clientrect);//获取信息，可不做

			/*
			* 标志
			*/
			surfrefresh = true;
			surfrefreshtick = GetTickCount64();
		}
		break;
	case WM_KEYDOWN:
		KeyDownProc(wParam);
		break;
	case WM_MOUSEWHEEL:	//**********************************需要高效，重绘**************************************
		if (mainbmp->isNotEmpty())
		{
			//缩放
			surfer.SurfAdjustZoom_wheel((short)HIWORD(wParam));
			surfer.SurfZoomRenew({ cursor.x - clientrect.left,cursor.y - clientrect.top }, false, true);

			needforcerenew = true;

			/*
			* 标志
			*/
			onzoom = true;		//设置onzoom标志
			zoomtick = GetTickCount64();

			surfrefresh = true;	//设置surfrefresh标志
			surfrefreshtick = GetTickCount64();
		}

		break;
	case WM_SETCURSOR:
		//不同窗口模式，不同处理
		if (winmode == WINMODE_ROUND)
		{
			if( iswindowedfullscreen)
				::SetCursor(LoadCursor(NULL, IDC_ARROW));
			else
				SetCursor_Custom();//设置鼠标样式
		}
		else
			DefWindowProc(hWnd, message, wParam, lParam);
		break;
	case WM_MOUSEMOVE://****************************************需要高效，重绘***********************************
		if (mainbmp->isNotEmpty())
		{
			if (ondrag)//*****************************************高效*******************************************
			{
				needforcerenew = surfer.OnDrag_Custom(MINUSPOINT(cursor, lastpos));

				lastpos = cursor;//记录鼠标位置

				/*
				* 标志
				*/
				if (needforcerenew)
				{
					surfrefresh = true;
					surfrefreshtick = GetTickCount64();
				}
				dragging = true;
				dragtick = GetTickCount64();
			}
			else if (ondragzoom)//*****************************************高效********************************
			{
				surfer.SurfAdjustZoom_normal1((cursor.x - lastpos.x) * 4);//调整放大倍率
				surfer.SurfZoomRenew(surfer.basepoint, false, true);//缩放
				surfer.GetCurInfo(cursor, clientrect);//需要的时候也可以获得信息

				needforcerenew = true;

				lastpos = cursor;//记录鼠标位置

				/*
				* 标志
				*/
				draggingzoom = true;//设置draggingzoom标志
				dragzoomtick = GetTickCount64();

				surfrefresh = true;	//设置surfrefresh标志
				surfrefreshtick = GetTickCount64();
			}
			else
			{
				surfer.GetCurInfo(cursor, clientrect);
			}
		}
		break;
	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE)
		{
			//窗口失去焦点，清除标志
			ClearFlag();
		}
		break;
	case WM_ERASEBKGND:	//背景擦除
		//不采取动作，防止窗口重绘时闪烁
		break;
	/*case WM_NCCALCSIZE:
		NCCALCSIZE_PARAMS *pnccsp;
		OnWinChange();
		pnccsp = (NCCALCSIZE_PARAMS*)lParam;
		pnccsp->rgrc[0].top = wndrect.top + 8;
		pnccsp->rgrc[0].bottom = pnccsp->rgrc[0].top + HEIGHTOF(wndrect)-16;
	case WM_NCPAINT:
		break;*/
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

short MYCALL1 GetSizeType(CPoint point)
{
	short sizetype = 0;

	if (point.x <= clientrect.left + SIZE_NEAR_PIXEL && point.x >= clientrect.left - SIZE_NEAR_PIXEL
		&& point.y <= wndrect.top + 12 && point.y >= wndrect.top - SIZE_NEAR_PIXEL)
		sizetype = HTTOPLEFT;
	else if (point.x <= clientrect.right + SIZE_NEAR_PIXEL && point.x >= clientrect.right - SIZE_NEAR_PIXEL
		&& point.y <= clientrect.bottom + SIZE_NEAR_PIXEL && point.y >= clientrect.bottom - SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOMRIGHT;
	else if (point.x <= clientrect.right + SIZE_NEAR_PIXEL && point.x >= clientrect.right - SIZE_NEAR_PIXEL
		&& point.y <= wndrect.top + 12 && point.y >= wndrect.top - SIZE_NEAR_PIXEL)
		sizetype = HTTOPRIGHT;
	else if (point.x <= clientrect.left + SIZE_NEAR_PIXEL && point.x >= clientrect.left - SIZE_NEAR_PIXEL
		&& point.y <= clientrect.bottom + SIZE_NEAR_PIXEL && point.y >= clientrect.bottom - SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOMLEFT;
	else if (point.x <= clientrect.left + SIZE_NEAR_PIXEL && point.x >= clientrect.left - SIZE_NEAR_PIXEL)
		sizetype = HTLEFT;
	else if (point.x <= clientrect.right + SIZE_NEAR_PIXEL && point.x >= clientrect.right - SIZE_NEAR_PIXEL)
		sizetype = HTRIGHT;
	else if (point.y <= wndrect.top + 12 && point.y >= wndrect.top - SIZE_NEAR_PIXEL)
		sizetype = HTTOP;
	else if (point.y <= clientrect.bottom + SIZE_NEAR_PIXEL && point.y >= clientrect.bottom - SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOM;
	else
		sizetype = HTNOWHERE;

	return sizetype;
}

inline void MYCALL1 SetCursor_Custom()
{
	if (sizeEnable && winmode == WINMODE_ROUND)
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
	else
	{
		;
	}
}

void MYCALL1 OnWinChange()
{
	GetClientRect(mainwnd, &clientrect);//得到client区域尺寸
	POINT clienttl = { 0, 0 };
	ClientToScreen(mainwnd, &clienttl);//获得client区域左上角的屏幕坐标
	//得到client真实屏幕区域
	clientrect.left = clienttl.x;
	clientrect.top = clienttl.y;
	clientrect.right += clienttl.x;
	clientrect.bottom += clienttl.y;

	//得到窗口区域
	GetWindowRect(mainwnd, &wndrect);
	//客户区和窗口的长宽差
	wbias = WIDTHOF(wndrect) - WIDTHOF(clientrect);
	hbias = HEIGHTOF(wndrect) - HEIGHTOF(clientrect);

	//更新clientsize
	clientsize.x = WIDTHOF(clientrect);
	clientsize.y = HEIGHTOF(clientrect);

	//更新文字信息显示区域
	RefreshTextRect();
}

inline void MYCALL1 RefreshTextRect()
{
	//文字显示区域
	textrect2.left = TEXTMARGIN_SIDE;
	textrect2.top = HEIGHTOF(clientrect) - TEXTMARGIN_BOTTOM - 16;
	textrect2.right = textrect2.left + 600;
	textrect2.bottom = textrect2.top + 16;

	//命令行显示区域
	cmdrect.left = TEXTMARGIN_SIDE;
	cmdrect.top = textrect2.top - 21;
	cmdrect.right = WIDTHOF(clientrect);
	cmdrect.bottom = textrect2.top - 5;
}

bool MYCALL1 Init()
{
	//窗口信息&状态
	OnWinChange();

	GetCursorPos(&cursor);
	lastpos.x = cursor.x;
	lastpos.y = cursor.y;

	wlimit = WINDOWWIDTH_INIT;
	hlimit = WINDOWHEIGHT_INIT;

	hprocess = GetCurrentProcess();
	//状态标志
	purewnd = false;
	iswindowedfullscreen = false;
	ondrag = false;
	ondragzoom = false;
	dragging = false;
	draggingzoom = false;
	onzoom = false;
	onsize = false;
	surfrefresh = false;
	needforcerenew = false;
	//选项标志
	mode = MODE_PIC;
	SetWindowMode(WINMODE_INIT);//WINMODE_NORMALWINDOWS
	backcolor = COLOR_BKG_INIT;
	if(winmode == WINMODE_ROUND)
		sizeEnable = true;
	else
		sizeEnable = false;
	easymoveEnable = true;
	flagshow = true;
	infoshow = true;
	fpslimit = true;
	screencoloron = false;
	screencolor = 0;

	//计时
	loopcount = 0;
	QueryPerformanceFrequency(&frequency);
	etime.QuadPart = 0;
	stime.QuadPart = 0;
	lasttime = 0;
	nowtime = 0;
	fps = 0.0f;
	staticfps = STATIC_FPS_NORMAL;
	staticframetime = 1000.0f / staticfps;
	avgfps = -1.0f;
	cvgfps = -1.0f;
	fpscount = 0;
	frametime = 0.0f;

	//D3D
	maindevice = NULL;

	//piccount = 0;
	//piclistlen = PICLISTLEN_INIT;
	//piclist = new PicPack[piclistlen];
	//mainpack = NULL;

	//命令行	
	up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	cmdlight = NULL;
	inputer0.SetCallbackCMD(CMDProc);

	//信息显示
	textrect0 = RECT(TEXTMARGIN_SIDE, TEXTMARGIN_TOP, 400, 100);
	textrect = RECT(TEXTMARGIN_SIDE, textrect0.bottom, 400, 500);

	RefreshTextRect();

	return true;
}

bool MYCALL1 D3DInit()
{
	if (!pd3dwnd->CreateDevice(D3DFMT_A8R8G8B8, 1))
		return false;

	//DEVICE
	maindevice = pd3dwnd->GetDevice();
	//BUFFER SIZE CACHE
	pbufferw = pd3dwnd->GetPBufferWidth();
	pbufferh = pd3dwnd->GetPBufferHeight();

	//SURFER
	mainbmp = &mainpicpack.bmp;
	surfer.BindDevice(maindevice);
	surfer.BindBuf(pbufferw, pbufferh);
	surfer.BindBMP(mainbmp);
	surfer.SetBackcolor(backcolor);

	//FONT
	pd3dwnd->D3DCreateFont(&font, L"Arial Rounded MT Bold"
		, 13, 5, 0, CLEARTYPE_NATURAL_QUALITY);
	pd3dwnd->D3DCreateFont(&font2, L"Calibri"
		, 16, 7, 0, CLEARTYPE_NATURAL_QUALITY);
	pd3dwnd->D3DCreateFont(&fontcmd, L"PixelSix10"
		, 20, 10, 0, CLEARTYPE_NATURAL_QUALITY);
	/*d3dfont1 = new CD3DFont(L"Arial Rounded MT Bold", 12, 0);
	d3dfont1->InitDeviceObjects(maindevice);
	d3dfont1->RestoreDeviceObjects();*/

	//SPRITE
	/*D3DXCreateSprite(maindevice, &m_sprite);
	D3DXMATRIX matTransform;
	D3DXMatrixIdentity(&matTransform);
	m_sprite->SetTransform(&matTransform);*/

	//OBJECT
	//矩形框
	//vertexbuffer方式
	//CUSTOMVERTEX0 g_vertices2[4] =
	//{
	//	{ 0, HEIGHTOF(clientrect) - 38, 0.0f, 1.0f, CMDRECT_COLOR2 }
	//	,{ WIDTHOF(clientrect), HEIGHTOF(clientrect) - 38, 0.0f, 1.0f, CMDRECT_COLOR2 }
	//	,{ WIDTHOF(clientrect), HEIGHTOF(clientrect) - 18, 0.0f, 1.0f, CMDRECT_COLOR2 }
	//	,{ 0, HEIGHTOF(clientrect) - 18, 0.0f, 1.0f, CMDRECT_COLOR2 }
	//};
	//if (FAILED(maindevice->CreateVertexBuffer(sizeof(g_vertices2), 0,
	//	FVF_CUSTOM0, D3DPOOL_SYSTEMMEM, &cmdlightrhw, NULL)))
	//	return false;
	//	// Fill the vertex buffer.
	//void *ptr;
	//if (FAILED(cmdlightrhw->Lock(0, sizeof(g_vertices2), (void**)&ptr, 0)))
	//	return false;
	//memcpy(ptr, g_vertices2, sizeof(g_vertices2));
	//cmdlightrhw->Unlock();

	//mesh方式 
	FVF1 g_vertices[4] =
	{
		{ D3DXVECTOR3(-20, -5.35f, 0), CMDRECT_COLOR_USING }
		,{ D3DXVECTOR3(-20, -4.8f, 0), CMDRECT_COLOR_USING }
		,{ D3DXVECTOR3(20, -4.8f, 0), CMDRECT_COLOR_USING }
		,{ D3DXVECTOR3(20, -5.35f, 0), CMDRECT_COLOR_USING }
	};
	WORD index[6] =
	{
		0, 1, 2
		,0, 2, 3
	};
	cmdlight = NULL;
	pd3dwnd->CreateMeshFVF(&cmdlight, g_vertices, index, sizeof(FVF1), sizeof(WORD), 2, 4, FVF_1PD);

	//VIEW
	SetView();

	//RENDERSTATE
	SetRenderState();

	//GUI
	//g_gui = new CD3DGUISystem(maindevice, WIDTHOF(clientrect), HEIGHTOF(clientrect));
	//g_gui->CreateFontW(L"PixelSix10", &fontID1, 20, 0, 0, CLEARTYPE_NATURAL_QUALITY);
	//g_gui->AddStaticText(STATIC_ID_1, L"Main Menu Version: 1.0",
	//	2, 70, COLOR_CMD_INIT, fontID1);
	//g_gui->AddButton(BUTTON_ID_1, 50, 200, L"1.bmp",
	//	L"11.bmp", L"12.bmp");

	return true;
}

bool MYCALL1 OnLoadFile(WCHAR file[])
{
	if (!maindevice)
	{
		return false;
	}

	if (mainpicpack.LoadFile(maindevice, file))
	{
		surfer.Refresh();

		surfrefresh = true;
		surfrefreshtick = GetTickCount64();

		SetWindowTextW(mainwnd, file);//窗口标题

		wcscpy_s(strFileName, file);//文件名
		wcscpy_s(picinfostr, mainpicpack.GetPicInfoStrW());//图片信息字符串更新
	}
	else
	{
		SetForegroundWindow(mainwnd);
		MessageBoxW(mainwnd, L"Load File FAILED!", L"ERROR", MB_OK | MB_APPLMODAL);

		return false;
	}

	return true;
}

bool MYCALL1 OnWininitFile(LPWSTR file)
{
	LPWSTR *szArgList;
	int argCount;
	szArgList = CommandLineToArgvW(file, &argCount);

	if (argCount > 1)
		OnLoadFile(szArgList[argCount - 1]);

	return false;
}

inline void MYCALL1 DelayFlag()
{
	time_t nowtick = GetTickCount64();

	if (onzoom)
	{
		if (nowtick < zoomtick || nowtick - zoomtick > FLAGDELAY_ZOOM)
			onzoom = false;
	}
	if (onsize)
	{
		if (nowtick < sizetick || nowtick - sizetick > FLAGDELAY_SIZE)
			onsize = false;
	}
	if (draggingzoom)
	{
		if (nowtick < dragzoomtick || nowtick - dragzoomtick > FLAGDELAY_DRAGZOOM)
			draggingzoom = false;
	}
	if (dragging)
	{
		if (nowtick < dragtick || nowtick - dragtick > FLAGDELAY_DRAG)
			dragging = false;
	}
	if (surfrefresh)
	{
		if (nowtick < surfrefreshtick || nowtick - surfrefreshtick > FLAGDELAY_SURFREFRESH)
			surfrefresh = false;
	}
}

void MYCALL1 ClearFlag()
{
	//LMBDown = false;
	if (ondrag)
	{
		ondrag = false;
		ReleaseCapture();
	}
	ondragzoom = false;
	draggingzoom = false;
	dragging = false;
	onzoom = false;
	onsize = false;
	surfrefreshtick = false;
}

//inline void WinSizeProc()
//{
//	if (!IsIconic(mainwnd))
//	{
//		onsize = true;
//		sizetick = GetTickCount64();
//
//		GetCursorPos(&cursor);//size时不进入主循环
//		int lastcrwidth = WIDTHOF(clientrect);
//		int lastcrheight = HEIGHTOF(clientrect);
//		OnWinChange();//更新窗口区域，如果改为size结束动作时做，TODO：设置另一窗口区域信息实时更新
//
//		ResetDevice();//处理设备丢失
//
//		MaintainWindowStyle();
//
//		bool renew = surfer.OnWinsize_Custom();
//
//		if (renew)
//		{
//
//
//			surfrefresh = true;
//			surfrefreshtick = GetTickCount64();
//		}
//	}
//	else
//	{
//		;
//	}
//}

void CustomWinSizeProc()
{
	if (sizeEnable && winmode == WINMODE_ROUND)
	{
		if (mousestate != 0)
		{
			if (mainbmp->isNotEmpty())
			{
				EndDragPic();//结束拖动图片才能控制窗口
			}
			PostMessage(mainwnd, WM_NCLBUTTONDOWN, mousestate, 0);
		}
	}
}

void ClickDragWindow_Custom(int cursorx, int cursory)
{
	if (cursorx < MIDDLEXOF(clientrect) + WIDTHOF(clientrect) / 5
		&& cursorx > MIDDLEXOF(clientrect) - WIDTHOF(clientrect) / 5
		&& cursory < MIDDLEYOF(clientrect) + HEIGHTOF(clientrect) / 5
		&& cursory > MIDDLEYOF(clientrect) - HEIGHTOF(clientrect) / 5)
	{
		PostMessage(mainwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
	}
	else
	{
		if (cursory < MIDDLEYOF(clientrect))
		{
			if (cursorx < MIDDLEXOF(clientrect))
				PostMessage(mainwnd, WM_NCLBUTTONDOWN, HTTOPLEFT, 0);
			else
				PostMessage(mainwnd, WM_NCLBUTTONDOWN, HTTOPRIGHT, 0);
		}
		else
		{
			if (cursorx < MIDDLEXOF(clientrect))
				PostMessage(mainwnd, WM_NCLBUTTONDOWN, HTBOTTOMLEFT, 0);
			else
				PostMessage(mainwnd, WM_NCLBUTTONDOWN, HTBOTTOMRIGHT, 0);
		}
	}
}

void KeyDownProc(WPARAM wParam)
{
	if (mode == MODE_PIC)
	{
		if (GetAsyncKeyState(VK_CONTROL) & KEYSTATEMASK_DOWN)
		{
			switch (wParam)
			{
			case 'F':	//设置标准窗口尺寸
				OpenFileWin();
				break;
			case 'S':	//手动重新生成surface一次
				if (mainbmp->isNotEmpty())
				{
					SaveFileWin();
				}
				break;
			case 'W':	//改变窗口模式
				if (winmode == WINMODE_ROUND)
					SetWindowMode(WINMODE_NORMALWINDOWS);
				else
					SetWindowMode(WINMODE_ROUND);
				break;
			}
		}
		else
		{
			switch (wParam)
			{
			case VK_OEM_2://进入命令行模式
				EnterCMDMode();
				break;
			case VK_ESCAPE:
				FullScreen_Windowed(false);
				break;
			case 'A':
			case VK_LEFT:
				if (mainbmp->isNotEmpty())
					surfer.OnDrag_Custom({ -1,0 });
				break;
			case 'B':	//图片恢复原始像素
				surfer.SurfSetZoom(1);
				surfer.SurfZoomRenew({ cursor.x - clientrect.left,cursor.y - clientrect.top }, true, false);

				onzoom = true;//设置标志
				zoomtick = GetTickCount64();

				surfrefresh = true;
				surfrefreshtick = GetTickCount64();

				break;
			case 'C':	//窗口调整到正好包括图像
				if (mainbmp->isNotEmpty())
				{
					wndrect.right = wndrect.left + surfer.zoomw + wbias;
					wndrect.bottom = wndrect.top + surfer.zoomh + hbias;
					//movewindow发出wmsize消息，[需最后做???]
					MoveWindow(mainwnd, wndrect.left, wndrect.top
						, wndrect.right - wndrect.left
						, wndrect.bottom - wndrect.top, TRUE);

					surfer.SurfHoming();
					surfer.SurfRenew(false);

					surfrefresh = true;
					surfrefreshtick = GetTickCount64();
				}
				break;
			case 'D':
			case VK_RIGHT:
				if (mainbmp->isNotEmpty())
					surfer.OnDrag_Custom({ 1,0 });
				break;
			case 'E':	//切换信息显示
				infoshow = !infoshow;
				break;
			case 'F':	//设置标准窗口尺寸
				if (mainbmp->isNotEmpty())
				{
					FitWnd(mainbmp->width, mainbmp->height);

					surfer.SurfHoming();
					surfer.SurfRenew(false);

					surfrefresh = true;
					surfrefreshtick = GetTickCount64();
				}
				break;
			case 'M':	//图片居中
				if (mainbmp->isNotEmpty())
				{
					surfer.SurfCenter(*pbufferw, *pbufferh);
					surfer.SurfRenew(false);

					surfrefresh = true;
					surfrefreshtick = GetTickCount64();
				}
				break;
			case 'N':
				if (backcolor == COLOR_BKG_INIT)
				{
					backcolor = COLOR_BKGNIGHT;
					//surfer.SetBackcolor(COLOR_BKGNIGHT);
				}
				else
				{
					backcolor = COLOR_BKG_INIT;
					//surfer.SetBackcolor(COLOR_BKG_INIT);
				}

				break;
			case 'P':	//清除图像
				Clear();
				break;
			case 'Q':	//图像放回左上角
				if (mainbmp->isNotEmpty())
				{
					surfer.SurfHoming();
					surfer.SurfRenew(false);

					surfrefresh = true;
					surfrefreshtick = GetTickCount64();
				}
				break;
			case 'R':	//手动渲染一次
				Render();
				break;
			case 'S':
			case VK_DOWN:
				if (mainbmp->isNotEmpty())
					surfer.OnDrag_Custom({ 0,1 });
				break;
			case 'V':	//手动重新生成surface一次
				if (mainbmp->isNotEmpty())
				{
					surfer.SurfRenew(false);

					surfrefresh = true;
					surfrefreshtick = GetTickCount64();
					
				}
				break;
			case 'W':	//改变窗口模式
			case VK_UP:
				if (mainbmp->isNotEmpty())
					surfer.OnDrag_Custom({ 0,-1 });
				break;
			case 'X':	//重置收敛帧率值（重新开始计算）
				fpscount = 0;
				cvgfps = -1;
				break;
			case VK_F1:	//显示显卡信息			
				pd3dwnd->DisplayAdapter();
				break;
			case VK_F3:
				PureWindow(!purewnd);
				break;
			case VK_F4:
				FullScreen_Windowed(!iswindowedfullscreen);
				break;
			}
		}
	}
	else if (mode == MODE_CMD)
	{
		switch (wParam)
		{
		case VK_ESCAPE:
		case VK_OEM_2:
			ExitCMDMode();
			break;
		default:
			inputer0.input((WCHAR)wParam);
			break;
		}
	}
}

void OpenFileWin()
{
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
		OnLoadFile(openfilename);
	}
}

void SaveFileWin()
{
	OPENFILENAME svfn;
	WCHAR savefilename[MAX_PATH];//存放文件名 
	wcscpy_s(savefilename, strFileName);

	//初始化     
	ZeroMemory(&svfn, sizeof(OPENFILENAME));
	svfn.lStructSize = sizeof(OPENFILENAME);//结构体大小
	//设置过滤     
	svfn.lpstrFilter = L"所有文件\0*.*\0bmp文件\0*.bmp\0png文件\0*.png\0jpg文件\0*.jpg\0";
	//默认过滤器索引
	svfn.nFilterIndex = 1;
	WCHAR *ftype = wcsrchr(strFileName, L'.');
	if (ftype)
	{
		if (_wcsicmp(ftype, L".bmp") == 0)
			svfn.nFilterIndex = 2;
		else if(_wcsicmp(ftype, L".png") == 0)
			svfn.nFilterIndex = 3;
		else if(_wcsicmp(ftype, L".jpg") == 0)
			svfn.nFilterIndex = 4;

	}
	//文件名的字段必须先把第一个字符设为\0
	svfn.lpstrFile = savefilename;
	//svfn.lpstrFile[0] = '\0';
	svfn.nMaxFile = sizeof(savefilename);
	//设置标志位，检查目录或文件是否存在     
	svfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	//svfn.lpstrInitialDir = NULL;     
	// 显示对话框让用户选择文件     
	if (GetSaveFileName(&svfn))
	{
		ftype = wcsrchr(savefilename, L'.');
		if (!ftype)
		{
			if (svfn.nFilterIndex == 2)
				wcscat_s(savefilename, L".bmp");
			else if (svfn.nFilterIndex == 3)
				wcscat_s(savefilename, L".png");
			else if (svfn.nFilterIndex == 4)
				wcscat_s(savefilename, L".jpg");
		}
		//选中文件后操作
		if (_waccess(savefilename, 0) == 0)//判断文件存在
		{
			if (IDYES == MessageBoxW(mainwnd, L"是否覆盖？", L"文件已存在", MB_YESNO | MB_APPLMODAL))
				OnSaveFile(savefilename);
			else
				;
		}
		else
			OnSaveFile(savefilename);
	}
}

bool OnSaveFile(WCHAR file[])
{
	/*WCHAR savefile[MAX_PATH];
	wcscpy_s(savefile, strFileName);
	WCHAR *dotpos = wcsrchr(savefile, L'.');

	WCHAR type[32];
	if (dotpos)
	{
		wcscpy_s(type, dotpos);
		*dotpos = L'\0';
	}
	wcscat_s(savefile, L"修改");

	if (dotpos)
		wcscat_s(savefile, type);*/

	if (!mainpicpack.SaveFile(maindevice, file))
	{
		MessageBoxW(mainwnd, L"Save File FAILED!", L"ERROR", MB_OK | MB_APPLMODAL);
		return false;
	}
	else
	{
		SetWindowTextW(mainwnd, file);//窗口标题

		wcscpy_s(strFileName, file);
		wcscpy_s(picinfostr, mainpicpack.GetPicInfoStrW());

		return true;
	}
}

void MYCALL1 SetWindowMode(UINT8 wmode)
{
	//设置矩形区域
	winmode = wmode;
	if (winmode == WINMODE_ROUND)
	{
		HRGN hrgn;

		RECT rgnrect;
		rgnrect.left = clientrect.left - wndrect.left;
		rgnrect.top = 8;
		rgnrect.right = WIDTHOF(wndrect) - rgnrect.left + 1/*WIDTHOF(clientrect) + 9*/;
		rgnrect.bottom = HEIGHTOF(wndrect) - rgnrect.top + 1/*HEIGHTOF(clientrect) + 22*/;
		hrgn = CreateRoundRectRgn(rgnrect.left, rgnrect.top
			, rgnrect.right, rgnrect.bottom, 2, 2);//尺寸，要+1
		SetWindowRgn(mainwnd, hrgn, TRUE);

		DeleteObject(hrgn);

		//SetClassLongPtr(mainwnd, GCL_STYLE
		//	, GetClassLong(mainwnd, GCL_STYLE) | CS_DROPSHADOW);//阴影
		//SetWindowLong(mainwnd, GWL_STYLE, GetWindowLong(mainwnd, GWL_STYLE)| CS_DROPSHADOW);
	}
	else
	{
		SetWindowRgn(mainwnd, NULL, TRUE);//恢复正常窗口

		//SetClassLong(mainwnd, GCL_STYLE
		//	, GetClassLong(mainwnd, GCL_STYLE) & (~CS_DROPSHADOW));//阴影
	}
}

inline void MaintainWindowStyle()
{
	if (winmode == WINMODE_ROUND)
	{
		HRGN hrgn;

		RECT rgnrect;
		rgnrect.left = clientrect.left - wndrect.left;
		rgnrect.top = 8;
		rgnrect.right = WIDTHOF(wndrect) - rgnrect.left + 1/*WIDTHOF(clientrect) + 9*/;
		rgnrect.bottom = HEIGHTOF(wndrect) - rgnrect.top + 1/*HEIGHTOF(clientrect) + 22*/;
		if(iswindowedfullscreen)
			hrgn = CreateRectRgn(rgnrect.left, rgnrect.top
				, rgnrect.right, rgnrect.bottom);
		else
			hrgn = CreateRoundRectRgn(rgnrect.left, rgnrect.top
				, rgnrect.right, rgnrect.bottom, 2, 2);//尺寸，要+1
		SetWindowRgn(mainwnd, hrgn, TRUE);

		DeleteObject(hrgn);
	}
}

void MYCALL1 BeginDragPic()
{
	SetCapture(mainwnd);//允许鼠标在窗口外拖动

	lastpos = cursor;

	ondrag = true;
}

void MYCALL1 EndDragPic()
{
	ReleaseCapture();

	ondrag = false;
	dragging = false;

	if (needforcerenew)//如果拖动过程中需要更新图片，在结束时用合适方法重新生成surface
		PostMessage(mainwnd, WM_SURFFORCERENEW, 0, 1);
}

void MYCALL1 BeginDragZoomPic()
{
	SetCapture(mainwnd);//允许鼠标在窗口外拖动

	lastpos = cursor;
	surfer.SetBasePoint({ cursor.x - clientrect.left,cursor.y - clientrect.top });

	ondragzoom = true;
}

void MYCALL1 EndDragZoomPic()
{
	ReleaseCapture();

	ondragzoom = false;
	draggingzoom = false;

	if (needforcerenew)//如果拖动放大过程中需要更新图片，在结束时用合适方法重新生成surface
		PostMessage(mainwnd, WM_SURFFORCERENEW, 0, 1);
}

inline void MYCALL1 EnterCMDMode()
{
	mode = MODE_CMD;
}

inline void MYCALL1 ExitCMDMode()
{
	inputer0.clear();
	mode = MODE_PIC;
}

void MYCALL1 FitWnd(int bmpwith, int bmpheight)
{
	wndrect.right = wndrect.left + wlimit + wbias;
	wndrect.bottom = wndrect.top + wlimit * bmpwith / bmpheight + hbias;
	if (HEIGHTOF(wndrect) > hlimit)
	{
		wndrect.bottom = wndrect.top + hlimit + hbias;
		wndrect.right = wndrect.left + hlimit * bmpwith / bmpheight + wbias;
	}

	MoveWindow(mainwnd
		, wndrect.left, wndrect.top, WIDTHOF(wndrect), HEIGHTOF(wndrect), TRUE);
}

bool FullScreen_Windowed(bool tofull)
{
	if (iswindowedfullscreen == tofull)
		return false;
	iswindowedfullscreen = tofull;
	if (iswindowedfullscreen)//全屏化
	{
		originwndrect = wndrect;
		RECT m_FullScreenRect;
		m_FullScreenRect.left = wndrect.left - clientrect.left;
		m_FullScreenRect.top = wndrect.top - clientrect.top + 22;
		if (purewnd)
			m_FullScreenRect.top -= 22;
		m_FullScreenRect.right = wndrect.right
			- clientrect.right + GetSystemMetrics(SM_CXSCREEN) + 12;
		m_FullScreenRect.bottom = wndrect.bottom
			- clientrect.bottom + GetSystemMetrics(SM_CYSCREEN);

		//隐藏任务栏
		/*HWND taskwnd, startbutton;
		taskwnd = FindWindow(L"Shell_TrayWnd", NULL);
		ShowWindow(taskwnd, SW_HIDE);
		startbutton = FindWindow(_T("Button"), NULL);
		ShowWindow(startbutton, SW_HIDE);
		taskwnd=GetDlgItem(FindWindow(L"Shell_TrayWnd", NULL), 0x130);
		ShowWindow(taskwnd, SW_HIDE);*/

		//去除边框，顺利进入全屏
		LONG tmp = GetWindowLong(mainwnd, GWL_STYLE);
		tmp &= ~WS_BORDER;
		tmp |= WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		SetWindowLong(mainwnd, GWL_STYLE, tmp);
		purewnd = true;

		/*MoveWindow(mainwnd, m_FullScreenRect.left, m_FullScreenRect.top
			, WIDTHOF(m_FullScreenRect), HEIGHTOF(m_FullScreenRect), FALSE);*/
		SetWindowPos(mainwnd, HWND_TOP, m_FullScreenRect.left, m_FullScreenRect.top
			, m_FullScreenRect.right, m_FullScreenRect.bottom + 8, SWP_NOREDRAW);//+8

	}
	else//取消全屏
	{
		//显示任务栏
		/*HWND taskwnd, startbutton;
		taskwnd = FindWindow(L"Shell_TrayWnd", NULL);
		ShowWindow(taskwnd, SW_SHOW);
		startbutton = FindWindow(_T("Button"), NULL);
		ShowWindow(startbutton, SW_SHOW);
		taskwnd = GetDlgItem(FindWindow(L"Shell_TrayWnd", NULL), 0x130);
		ShowWindow(taskwnd, SW_SHOW);*/

		LONG tmp = GetWindowLong(mainwnd, GWL_STYLE);
		tmp |= WS_POPUPWINDOW;
		tmp |= WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		SetWindowLong(mainwnd, GWL_STYLE, tmp);
		purewnd = false;

		/*MoveWindow(mainwnd, originwndrect.left, originwndrect.top
			, WIDTHOF(originwndrect), HEIGHTOF(originwndrect), FALSE);*/
		//SetWindowPos比MoveWindow发送更少的消息，加速，减缓图片闪烁
		SetWindowPos(mainwnd, HWND_TOP, originwndrect.left, originwndrect.top
			, WIDTHOF(originwndrect), HEIGHTOF(originwndrect), SWP_NOREDRAW);
	}

	return true;
}

bool PureWindow(bool topure)
{
	if (purewnd == topure)
		return false;
	purewnd = topure;
	if (purewnd)
	{
		LONG tmp = GetWindowLong(mainwnd, GWL_STYLE);
		tmp &= ~WS_BORDER;
		tmp |= WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		SetWindowLong(mainwnd, GWL_STYLE, tmp);
	}
	else
	{
		LONG tmp = GetWindowLong(mainwnd, GWL_STYLE);
		tmp |= WS_POPUPWINDOW;
		tmp |= WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		SetWindowLong(mainwnd, GWL_STYLE, tmp);
	}
	//PostMessage(mainwnd, WM_PAINT, 0, 0);
	//UpdateWindow(mainwnd);
	//ShowWindow(mainwnd, SW_SHOW);
	MoveWindow(mainwnd, wndrect.left, wndrect.top
		, WIDTHOF(wndrect), HEIGHTOF(wndrect)-1, TRUE);
	MoveWindow(mainwnd, wndrect.left, wndrect.top
		, WIDTHOF(wndrect), HEIGHTOF(wndrect)+1, TRUE);

	return false;
}

inline void MYCALL1 SetRenderState()
{
	//标配：D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA
	maindevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	maindevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	maindevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	maindevice->SetRenderState(D3DRS_LIGHTING, FALSE);
}

inline void MYCALL1 SetView()
{
	//vertexbuffer方法
	//cmdlightrhw->Release();
	//CUSTOMVERTEX0 g_vertices2[4] =
	//{
	//	{ 0, HEIGHTOF(clientrect) - 38, 0.0f, 1.0f, CMDRECT_COLOR2 }
	//	,{ WIDTHOF(clientrect), HEIGHTOF(clientrect) - 38, 0.0f, 1.0f, CMDRECT_COLOR2 }
	//	,{ WIDTHOF(clientrect), HEIGHTOF(clientrect) - 18, 0.0f, 1.0f, CMDRECT_COLOR2 }
	//	,{ 0, HEIGHTOF(clientrect) - 18, 0.0f, 1.0f, CMDRECT_COLOR2 }
	//};
	//if (FAILED(maindevice->CreateVertexBuffer(sizeof(g_vertices2), 0,
	//	FVF_CUSTOM0, D3DPOOL_SYSTEMMEM, &cmdlightrhw, NULL)))
	//	return;
	//// Fill the vertex buffer.
	//void *ptr;
	//if (FAILED(cmdlightrhw->Lock(0, sizeof(g_vertices2), (void**)&ptr, 0)))
	//	return;
	//memcpy(ptr, g_vertices2, sizeof(g_vertices2));
	//cmdlightrhw->Unlock();

	//mesh方法:视角
	eye.y = -5.82f + HEIGHTOF(clientrect) / 75.0f;
	eye.z = -HEIGHTOF(clientrect) / 43.3f;
	at.y = -5.82f + HEIGHTOF(clientrect) / 75.0f;
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &eye, &at, &D3DXVECTOR3(0.0f, 1.0f, 0.0f));//?unsafe
	maindevice->SetTransform(D3DTS_VIEW, &matView);
	D3DXMATRIXA16 proj;
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 3
		, (float)WIDTHOF(clientrect) / HEIGHTOF(clientrect)
		, 0.02f, 100.0f);
	maindevice->SetTransform(D3DTS_PROJECTION, &proj);
}

//inline void MYCALL1 CMDRender()
//{
//	//使用CD3DGUISystem后恢复
//	//maindevice->SetTexture(NULL, NULL);
//
//	//vertexbuffer
//	/*maindevice->SetStreamSource(0, cmdlightrhw, 0, sizeof(CUSTOMVERTEX0));
//	maindevice->SetFVF(FVF_CUSTOM0);
//	maindevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);*/
//
//	//mesh方式
//	cmdlight->DrawSubset(0);//命令行矩形框
//
//	fontcmd->DrawTextW(NULL, inputer0.wstr, -1, &cmdrect, DT_LEFT | DT_NOCLIP, COLOR_CMD_INIT);
//}

bool MYCALL1 InfoRender()
{
	WCHAR subinfo[256] = { 0 }; 
	WCHAR infowstr[512] = { 0 };

	//d3dfont1->DrawTextW(100, 200, COLOR_TEXT1, infowstr0, 0);
	font->DrawTextW(NULL, picinfostr, -1, &textrect0, DT_LEFT | DT_NOCLIP, COLOR_TEXT1);

	//buffer尺寸、surface尺寸
	if (surfer.surf)
	{ 
		swprintf_s(subinfo, _T("mode: %d\n\
			buffer: %d× %d\n\
			intended surface: %d× %d\n\
			surface: %d× %d      renew time:%.6f\n\
			zoom: %.3f\n\
			actzoom: X %.4f Y %.4f\n")
			, mode
			, *pbufferw, *pbufferh
			, surfer.zoomw, surfer.zoomh
			, surfer.surfsize.x, surfer.surfsize.y, surfer.renewtime
			, surfer.surfzoom
			, surfer.actualzoomx, surfer.actualzoomy);
	}
	else
	{
		swprintf_s(subinfo, _T("mode: %d\n\
			buffer: %d× %d\n\
			intended surface: -×-\n\
			surface: -×-\n\
			zoom: %.3f\n\
			actzoom: X %.4f Y %.4f\n")
			, mode
			, *pbufferw, *pbufferh
			, surfer.surfzoom
			, surfer.actualzoomx, surfer.actualzoomy);
	}
	wcscat_s(infowstr, subinfo);

	//surface起始点、clipsurface起始点
	swprintf_s(subinfo, _T("surface source: %d, %d\n\
		clipsurface base: %d, %d\n")
		, -surfer.surfsrc.x, -surfer.surfsrc.y
		, surfer.surfbase.x, surfer.surfbase.y);
	wcscat_s(infowstr, subinfo);

	//fps
	swprintf_s(subinfo, _T("fps: %.2f/%.2f (%.1f)  %.3fms   %lld   %d\n\
		%.1fMB, %.1fMB\n")
		, fps, avgfps, cvgfps, frametime, loopcount, surfer.surfrefreshcount
		, memoryin, memoryout);
	wcscat_s(infowstr, subinfo);

	//鼠标位置、鼠标相对窗口客户区位置、鼠标位置(type)
	const string cursorposshow[3] = { "PIC", "BLANK", "OUTSIDE CLIENT" };
	swprintf_s(subinfo, _T("cursor: %d, %d\n\
		cursor client: %d, %d\n\
		cursor pos: %S\n")
		, cursor.x, cursor.y
		, cursor.x - clientrect.left, cursor.y - clientrect.top
		, cursorposshow[surfer.cursorpos].c_str());
	wcscat_s(infowstr, subinfo);

	//鼠标像素位置
	if (mainbmp->isNotEmpty())
		swprintf_s(subinfo, _T("picture pixel: %d, %d\n"), surfer.picpixel.x, surfer.picpixel.y);
	else
		swprintf_s(subinfo, _T("picture pixel:-, -\n"));
	wcscat_s(infowstr, subinfo);

	//鼠标像素颜色、屏幕像素颜色、背景色
	swprintf_s(subinfo, _T("pixel color: %02X.%06X.ARGB\n\
		screen color: %02X.%06X.ARGB\n\
		backcolor: %08X.ARGB\n")
		, (surfer.picpixelcolor >> 24), (surfer.picpixelcolor & 0xFFFFFF)
		, (screencolor >> 24), (screencolor & 0xFFFFFF)
		, surfer.backcolor);
	wcscat_s(infowstr, subinfo);

	//第一部分信息显示
	font->DrawTextW(NULL, infowstr, -1, &textrect, DT_LEFT | DT_NOCLIP, COLOR_TEXT1);

	//标志
	//const WCHAR yesno1[2] = { L'×', L'√' };
	const WCHAR yesno2[2] = { L'×', L'●' };
	swprintf_s(infowstr,
		L"pic: %lc\
		     surf: %lc\
		     onzoom: %lc\
		     drag: %lc\
		     onsize: %lc\
		     dragzoom: %lc\
		     clipon: %lc\
		     surfclipped: %lc\
		     picclipped: %lc\
		     picout: %lc\
		     surfrefresh: %lc"
		, yesno2[mainbmp->isNotEmpty()]
		, yesno2[(surfer.surf != NULL)], yesno2[onzoom]
		, yesno2[dragging], yesno2[onsize]
		, yesno2[draggingzoom], yesno2[surfer.clip]
		, yesno2[surfer.surfclipped], yesno2[surfer.picclipped]
		, yesno2[surfer.outsideclient], yesno2[surfrefresh]);//●

	font2->DrawTextW(NULL, infowstr, -1, &textrect2
		, DT_LEFT | DT_TOP | DT_NOCLIP, COLOR_TEXT1);

	return true;
}

void MYCALL1 Render()
{
	HRESULT hr;

	//绘制
	maindevice->Clear(0, NULL, D3DCLEAR_TARGET, backcolor, 1.0f, 0);
	maindevice->BeginScene();

	surfer.Render();

	if (infoshow)//信息显示
		InfoRender();

	if (mode == MODE_CMD)//命令行显示
	{
		cmdlight->DrawSubset(0);//命令行矩形框

		fontcmd->DrawTextW(NULL, inputer0.wstr, -1, &cmdrect, DT_LEFT | DT_NOCLIP, COLOR_CMD_INIT);
	}

	//ProcessGUI(g_gui, LMBDown, cursor.x- clientrect.left, cursor.y- clientrect.top, GUICallback);

	maindevice->EndScene();
	hr = maindevice->Present(NULL, NULL, NULL, NULL);

	//处理设备丢失
	/*if (hr == D3DERR_DEVICELOST)
	{
		if (maindevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ResetDevice();
		}
	}*/
}

inline bool MYCALL1 NoPic()
{
	return (mainpicpack.isEmpty() || /*SurfEmpty()*/surfer.surf);
}

void MYCALL1 Clear()
{
	mainbmp->Clear();
	surfer.Clear();

	SetWindowText(mainwnd, L"");

	picinfostr[0] = L'\0';
}

bool MYCALL1 ResetDevice()
{
	if (!maindevice)
		return false; 

	//修改device长宽
	OnLostDevice();
	return OnResetDevice(WIDTHOF(clientrect),HEIGHTOF(clientrect));
}

void MYCALL1 OnLostDevice()
{
	pd3dwnd->OnLostDevice();

	//g_gui->OnLost();
	font->OnLostDevice();
	font2->OnLostDevice();
	fontcmd->OnLostDevice();
}

inline bool MYCALL1 OnResetDevice(int clientw, int clienth)
{
	if (!pd3dwnd->OnResetDevice(clientw, clienth))
	{
		return false;
	}

	//g_gui->OnReset(maindevice);
	fontcmd->OnResetDevice();
	font2->OnResetDevice();
	font->OnResetDevice();

	SetView();
	SetRenderState();

	return true;
}

bool MYCALL1 OnResetDevice()
{
	if (!pd3dwnd->OnResetDevice())
	{
		return false;
	}

	//g_gui->OnReset(maindevice);
	fontcmd->OnResetDevice();
	font2->OnResetDevice();
	font->OnResetDevice();

	//清空重置方法
	//SAFE_RELEASE(maindevice);
	//if (!InitDevice())
	//	return false;
	//SAFE_RELEASE(font);
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

	SetView();
	SetRenderState();

	return true;
}

//void CALLBACK GUICallback(int id, int state)
//{
//	switch (id)
//	{
//	case BUTTON_ID_1:
//		break;
//	}
//}

void CALLBACK CMDProc(WCHAR *wstr)
{
	if (!wstr)
		return;

	WCHAR *p = wcsstr(wstr, L" ");
	if (p)
	{
		*p = 0;//截断wstr
		p++;
	}

	WCHAR initial = wstr[0];

	if (_wcsicmp(wstr, L"quit") == 0)
	{
		PostMessage(mainwnd, WM_QUIT, NULL, NULL);
	}
	else if (_wcsicmp(wstr, L"open") == 0)
	{
		OpenFileWin();
	}
	else if (_wcsicmp(wstr, L"maxwin") == 0)
	{
		SendMessage(mainwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);//SC_RESTORE
	}
	else if (_wcsicmp(wstr, L"rstwin") == 0)
	{
		SendMessage(mainwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	}
	else if (_wcsicmp(wstr, L"minwin") == 0)
	{
		SendMessage(mainwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}
	else if (_wcsicmp(wstr, L"exit") == 0)
	{
		ExitCMDMode();
	}
	else if (_wcsicmp(wstr, L"gray") == 0)
	{
		if (mainbmp->isNotEmpty())
		{
			mainbmp->Gray();
			surfer.SurfRenew(false);

			surfrefresh = true;
			surfrefreshtick = GetTickCount64();
		}
	}
	else if (_wcsicmp(wstr, L"loser") == 0)
	{
		if (mainbmp->isNotEmpty())
		{
			mainbmp->LOSE_R();
			surfer.SurfRenew(false);

			surfrefresh = true;
			surfrefreshtick = GetTickCount64();
		}
	}
	else if (_wcsicmp(wstr, L"loseg") == 0)
	{
		if (mainbmp->isNotEmpty())
		{
			mainbmp->LOSE_G();
			surfer.SurfRenew(false);

			surfrefresh = true;
			surfrefreshtick = GetTickCount64();
		}
	}
	else if (_wcsicmp(wstr, L"loseb") == 0)
	{
		if (mainbmp->isNotEmpty())
		{
			mainbmp->LOSE_B();
			surfer.SurfRenew(false);

			surfrefresh = true;
			surfrefreshtick = GetTickCount64();
		}
	}
	else if (_wcsicmp(wstr, L"inv") == 0)
	{
		if (mainbmp->isNotEmpty())
		{
			mainbmp->Inverse();
			surfer.SurfRenew(false);

			surfrefresh = true;
			surfrefreshtick = GetTickCount64();
		}
	}
	else if (_wcsicmp(wstr, L"inva") == 0)
	{
		if (mainbmp->isNotEmpty())
		{
			mainbmp->InverseAlpha();
			surfer.SurfRenew(false);

			surfrefresh = true;
			surfrefreshtick = GetTickCount64();
		}
	}
	else if (_wcsicmp(wstr, L"inv4") == 0)
	{
		if (mainbmp->isNotEmpty())
		{
			mainbmp->InverseAll();
			surfer.SurfRenew(false);

			surfrefresh = true;
			surfrefreshtick = GetTickCount64();
		}
	}
	else
	{
		if (!p)
			return;

		float num = 0;
		swscanf_s(p, L"%f", &num);

		if (_wcsicmp(wstr, L"zoom") == 0)
		{
			if (mainbmp->isNotEmpty() && num > MIN_ZOOM)
			{
				surfer.SurfSetZoom(num);
				surfer.SurfZoomRenew({ cursor.x-clientrect.left,cursor.y-clientrect.top }, true, false);//放大，调整surface位置
				
				onzoom = true;//设置标志
				zoomtick = GetTickCount64();

				surfrefresh = true;
				surfrefreshtick = GetTickCount64();
			}
		}
	}
}

/*void CALLBACK TimerProc(HWND hwnd, UINT message, UINT iTimerID, DWORD dwTime)
{
	switch(iTimerID){
	case 0:
		//size结束动作
		//bool oldsize = onsize;
		//if (oldsize && !onsize)
		//{
		//	OnWinChange();
		//	if (maindevice)
		//	{
		//		ResetDevice();//处理设备丢失
		//	}
		//	if (mainbmp->isNotEmpty())
		//		RefreshSurf();//不重新创建图片可能显示错误
		//}
		if (onsize)
			DelayFlag();//size时不进入主循环，需要完成。用PostMsg WM_INFORENER

		//拖动时或缩放不获取屏幕颜色
		if (screencoloron)
		{
			if (!dragging && !onzoom)
			{
				HDC pdc = ::GetDC(NULL);
				screencolor = GetPixel(pdc, cursor.x, cursor.y);
				DeleteDC(pdc);
				//低1，3字节互换
				DWORD low = screencolor & 0xFF;
				DWORD mid = screencolor & 0xFF00;
				DWORD high = screencolor & 0xFF0000;
				DWORD top = screencolor & 0xFF000000;
				screencolor = top + mid + (low << 16) + (high >> 16);
			}
		}
		break;
	default:
		break;
	}
}
*/