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

	// 舍弃的方法
    //MyRegisterClass(hInstance);
    //// 执行应用程序初始化: 
    //if (!InitInstance (hInstance, nCmdShow))
    //{
    //    return FALSE;
    //}

	// 初始化窗口
	pD3DWnd = new D3DWnd();
	if (!pD3DWnd->D3DCreateWindow(szTitle, WndProc, hInstance
		, CS_HREDRAW | CS_VREDRAW/* | CS_DROPSHADOW*/
		, WS_OVERLAPPEDWINDOW, WS_EX_ACCEPTFILES
		, WINDOWPOSX_INIT, WINDOWPOSY_INIT, WINDOWWIDTH_INIT, WINDOWHEIGHT_INIT
		, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PICTURE))
		, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL))
		, /*MAKEINTRESOURCEW(IDC_PICTURE)*/0
		, szWindowClass, COLOR_BKG_INIT))
		return FALSE;

	hWndMain = pD3DWnd->GetHWND();//存储主窗口句柄
	if (hWndMain == NULL)
		return 0;

	// 启动窗口
	//startup = new D3DWnd();
	//RECT rcMain;
	//GetWindowRect(hWndMain, &rcMain);
	//float fBias = 0.618f;
	//POINT ptMid = { (LONG)(rcMain.left + WIDTHOF(rcMain)*fBias)
	//	, (LONG)(rcMain.top + HEIGHTOF(rcMain)*fBias) };
	//int iW = 240, iH = 220;
	//if (startup->D3DCreateWindow(L"startUp", StartWndProc, 0
	//	, CS_DROPSHADOW, WS_POPUP, 0
	//	, ptMid.x - iW / 2, ptMid.y - iH / 2, iW, iH
	//	, 0, 0, 0, 0, 0xFFD0BBFF))
	//{
	//	hWndStartup = startup->GetHWND();
	//	//SetWindowPos(hWndStartup, HWND_NOTOPMOST,	0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	//}

	// 初始化参数
	Init();
	D3DInit();
	//TimerID_main = timeSetEvent(16, 0, TimerProc, 0, TIME_PERIODIC);//多媒体定时器

	// 初始窗口启动参数
	OnWinInitFile(GetCommandLine());

	// 销毁启动窗口
	/*if(hWndStartup)
		SendMessage(hWndStartup, WM_CLOSE, 0, 0);*/

	// 舍弃的方法
    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PICTURE));

	// 优先级
	//SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	// 循环参数初始化
	time_t nowtime = 0, lasttime = 0;//计时
	LARGE_INTEGER frequency = { 0 }, stime = { 0 }, etime = { 0 };
	QueryPerformanceFrequency(&frequency);

    // 主消息循环: 
	timeBeginPeriod(1);//精度设置
	MSG msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
	while (msg.message != WM_QUIT)
	{
		// 消息
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// ERROR 有时退出时会崩溃!

		// 帧率限制
		QueryPerformanceCounter(&etime);
		float testfps;
		if (bFpsLimited)
		{
			if (etime.QuadPart != stime.QuadPart)
				testfps = (float)frequency.QuadPart / (etime.QuadPart - stime.QuadPart);
			if (bDragging || bOnZoom || bDragzooming)//图片需快速更新
			{
				short ftdiff = (short)(normalFrameTime - 1000.0f / testfps);
				if (/*testfps > normalFps*///可能莫名奇妙降低帧率
					ftdiff >= 1)
				{
					Sleep(1);
					continue;
				}
			}
			else// 静止状态，低帧率刷新界面
			{
				if (testfps > staticFps)
				{
					Sleep(1);
					continue;
				}
			}
		}

		// 计数
		nLoops++;
		time(&nowtime);

		// 更新延迟的标志
		DelayFlag();

		// 每秒执行(帧率等)
		if (nowtime != lasttime)
		{
			lasttime = nowtime;

			// 计算内存占用 
			PROCESS_MEMORY_COUNTERS pmc;
			GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
			memoryIn = pmc.WorkingSetSize / B_TO_MB;//内存占用量
			memoryOut = pmc.PagefileUsage / B_TO_MB;//虚拟内存占用量

			// 计算fps
			fpsCount++;
			fps = (float)frequency.QuadPart / (etime.QuadPart - stime.QuadPart);
			
			frameTime = 1000.0f / fps;				//帧时间
			if (avgFps < 0)							//平均帧率
				avgFps = fps;
			else
				avgFps = avgFps*0.9f + fps*0.1f;

			if (cvgFps < 0)							//收敛帧率
				cvgFps = fps;
			else
				cvgFps = (cvgFps*fpsCount + fps) / (fpsCount + 1);
		}
		stime.QuadPart = etime.QuadPart;// 开始计时

		// 渲染
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
//   //hWndMain = hWnd;
//
//   ShowWindow(hWnd, nCmdShow);
//   UpdateWindow(hWnd);
//
//   return TRUE;
//}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT uFileNum;
	POINT ptCur;
	bool bSucceed = false;

	if (g_gui != NULL && !(message == WM_MOUSEMOVE && (bOnDrag || bOnDragzoom || bOnDragRotate)))
		g_gui->MsgProc(hWnd, message, wParam, lParam);

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
				if (pLivePicpack != NULL)
					SaveFileWin(pLivePicpack->GetFileName());
				break;
			case IDM_CLEAR:
				Drop();
				PostPicPackChange();
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	//	WM_PAINT	- 处理窗口重绘，需要高效
    case WM_PAINT:
        {
			if (!IsIconic(hWnd))
			{
				Render();//使得调整窗口尺寸时自动重绘（调整窗口尺寸时不进入主循环）

				//不可以去掉下面3行(尽管没有添加绘图代码)，会导致帧率降低
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);

				// 在此处添加使用 hdc 的任何绘图代码...
				//hf = CreateFontW(13, 5, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET
				//	, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY
				//	, FF_MODERN, L"Arial Rounded MT Bold");//Arial Rounded MT Bold
				//
				//SelectObject(hdc, (HGDIOBJ)hf);

				//RECT rg = { 0,60,100,100 };
				//SetBkMode(hdc, TRANSPARENT);
				//SetTextColor(hdc, 0x00FF006E);
				//DrawText(hdc, L"dwqwqa7", -1, &rg, DT_NOCLIP);

				EndPaint(hWnd, &ps);
			}
			else
			{
				;
			}
        }
        break;
	//	WM_ERASEBKGND	- 背景擦除
	case WM_ERASEBKGND:
		// 不采取动作，防止窗口重绘时闪烁
		// 后果是窗口都以白底刷新
		break;
	//	WM_MOUSEMOVE	- 鼠标移动，需要高效
	case WM_MOUSEMOVE:
		ptCur = { LOSHORT(lParam),HISHORT(lParam) };
		if (bPicOn)
		{
			if (bOnDrag)//*****************************************高效*******************************************
			{
				//pLiveBmp->TestAlpha(pLiveBmp);// 测试
				SF_SRFR(surfer.OnDrag_Custom(MINUSPOINT(ptCur, ptLastCursor)));
				SF_DR();
			}
			else if (bOnDragzoom)//*****************************************高效********************************
			{
				surfer.SurfAdjustZoom_DragPR(ptCur.x - ptLastCursor.x, true, ISALTDOWN);//调整放大倍率
				SF_SRFR(surfer.SurfRenew(!ISKEYDOWN('V')));
				//bNeedForceRenew |= surfer.IsSurfNull();//surface 更新失败仍然激发刷新

				// 标志
				bDragzooming = true;
				dragzoomTick = GetTickCount();
			}
			else if (bOnDragRotate)
			{
				surfer.SurfAdjustRotate_DragPR(ptCur.x - ptLastCursor.x, true);
				SF_SRFR(surfer.SurfRenew(!ISKEYDOWN('V')));
			}
			else
			{
				;
			}
		}
		else
			;
		// 获取信息
		surfer.GetCurInfo(&ptCur);

		ptLastCursor = ptCur;//保存鼠标位置
		break;
	//	WM_MOUSEWHEEL	- 鼠标滚轮，需要高效
	case WM_MOUSEWHEEL:
		ptCur = { LOSHORT(lParam), HISHORT(lParam) };
		ScreenToClient(hWndMain, &ptCur);//WM_MOUSEWHEEL参数是屏幕坐标
		if (bPicOn)
		{
			//缩放
			surfer.SetBasePoint(ptCur);
			surfer.SurfAdjustZoom_WheelPR(HISHORT(wParam), true, ISALTDOWN);
			SF_SRFR(surfer.SurfRenew(!ISKEYDOWN('V')));

			// 获取信息
			surfer.GetCurInfo(&ptCur);

			// 标志
			bOnZoom = true;
			zoomTick = GetTickCount();
		}
		break;
	//	WM_SIZE	- 窗口拉伸，需要高效
	case WM_SIZE:
		if (!IsIconic(hWnd))
		{
			OnWinChange();// 更新窗口区域。如果使用size结束重置设备，这句可不执行，添加另一窗口区域变量实时更新。
			MaintainWindowStyle();// 维持窗口样式

			//如果使用 WM_SIZE 结束重置设备，ResetDevice 和 surface 更新都只在结束时执行
			// 重置设备
			ResetDevice();

			// surface更新
			SF_SR(surfer.OnWinsize_Custom());

			// 获取信息
			surfer.GetCurInfo(&ptCur);

			// 标志
			bOnSize = true;
			sizeTick = GetTickCount();
		}
		else
		{
			;
		}
		break;
	//	WM_MOVE	- 窗口移动，需要高效
	case WM_MOVE:
		if (!bOnSize)
		{
			//更新窗口区域
			OnWinChange();// bOnSize 时不做，WM_SIZE 已经执行
		}
		break;
	//	WM_SETCURSOR	- 设置鼠标样式
	case WM_SETCURSOR:
		switch (GetSizeType())
		{
		case HTLEFT:
		case HTRIGHT:
		case HTTOP:
		case HTBOTTOM:
		case HTTOPLEFT:
		case HTBOTTOMRIGHT:
		case HTTOPRIGHT:
		case HTBOTTOMLEFT:
			if (hCursorG != NULL)
			{
				SetCursor(hCursorG);
				bSucceed = true;
			}
			break;
		default:
			if (hCursorMain != NULL)
			{
				SetCursor(hCursorMain);
				bSucceed = true;
			}
			break;
		}
		if(!bSucceed)
			DefWindowProc(hWnd, message, wParam, lParam);
		break;
	//	WM_LBUTTONDBLCLK	- 鼠标左键双击
	//case WM_LBUTTONDBLCLK:
	//	if (bWindowedFullscreen)
	//	{
	//		FullScreen_Windowed(!bWindowedFullscreen);
	//	}
	//	else
	//	{
	//		PostMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);//双击移动窗口
	//	}
	//	break;
	//	WM_LBUTTONDOWN	- 鼠标左键按下
	case WM_LBUTTONDOWN:		
		//自定义客户区内拉伸窗口
		ptCur = { LOSHORT(lParam),HISHORT(lParam) };
		if (ISCONTROLDOWN)
		{
			if (bWindowedFullscreen)
				FullScreen_Windowed(!bWindowedFullscreen, false);//退出全屏
			
			BeginDragWindow_Inner(ptCur);
			break;
		}
		//无边框窗口拉伸
		else if (winMode == WINMODE_ROUND
			&& GetSizeType() != HTNOWHERE && !bWindowedFullscreen)
		{
			BeginDragWindow_Custom(ptCur);
			break;
		}
		//拖动图片
		else
		{
			if (bPicOn && !bOnDragzoom && !bOnDragRotate)
			{
				BeginDragPic();//开始拖动
			}
		}

		break;
	//	WM_LBUTTONUP	- 鼠标左键抬起
	case WM_LBUTTONUP:
		if (bOnDrag)
		{
			EndDragPic();
		}
		break;
	//	WM_RBUTTONDOWN	- 鼠标右键按下
	case WM_RBUTTONDOWN:
		if (bPicOn && !bOnDrag && !bOnDragRotate)
		{
			if (ISKEYDOWN('R'))
				BeginDragRotatePic();
			else
				BeginDragZoomPic();//开始拖动
		}
		break;
	//	WM_RBUTTONDOWN	- 鼠标右键抬起
	case WM_RBUTTONUP:
		if (bOnDragzoom)
		{
			EndDragZoomPic();
		}
		else if (bOnDragRotate)
		{
			EndDragRotatePic();
		}
		break;
	//case WM_NCLBUTTONDOWN:
	//	DefWindowProc(hWnd, message, wParam, lParam); 
	//	//防止发送NCLBUTTONDOWN后鼠标抬起不响应
	//	PostMessage(hWnd, WM_LBUTTONUP, GetSizeType(), lParam);
	//	break;
	/*case WM_SYSCOMMAND:
		switch (wParam & 0xFFF0)
		{
		case SC_MOVE:
			break;
		}
		break;*/
	//	WM_KEYDOWN	- 键盘按键
	case WM_KEYDOWN:
		KeyDownProc(wParam);
		//g_gui->HandleKeyboard(message, wParam, lParam);
		break;
	//	WM_SURFFORCERENEW	- 强制刷新surface（用合适的生成方法）
	case WM_SURFFORCERENEW:
		if (!bOnDrag && !bOnZoom && !bOnDragzoom && !bOnDragRotate && !bOnSize)
		{
			SF_SR(surfer.SurfRenew(false));
			bNeedForceRenew = false;//清除需要强制刷新的标志

			UpdateSurfaceInfo();

		}
		break;
	case WM_MOUSEWHEELEND:
		if (bNeedForceRenew)
			PostMessage(hWnd, WM_SURFFORCERENEW, 0, 1);
		break;
	//case WM_SIZEEND:
	//	OnWinChange();//更新窗口区域，如果改为size结束动作时做，TODO：设置另一窗口区域信息实时更新
	//	MaintainWindowStyle();//维持窗口样式

	//	ResetDevice();

	//	// surface更新
	//	bSurfRenew = surfer.OnWinsize_Custom();
	//	bNeedForceRenew |= bSurfRenew;

	//	// 标志
	//	if (bSurfRenew)
	//		surfRenewTick = GetTickCount();

	//	break;
	//	WM_ACTIVATE	- 激活窗口
	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE)
		{
			//处理没有更新的Surface
			if (bNeedForceRenew)
				PostMessage(hWnd, WM_SURFFORCERENEW, 0, 1);

			//窗口失去焦点，清除标志
			ClearFlag();
			SetStaticFps(STATIC_FPS_SILENT);
		}
		else if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
		{
			SetStaticFps(STATIC_FPS_NORMAL);
		}
		break;
	//	WM_IME_CHAR	- 输入法输入
	case WM_IME_CHAR:
		g_gui->HandleKeyboard(message, wParam, lParam);
		break;
	//	WM_DROPFILES	- 处理拖入文件
	case WM_DROPFILES:
		uFileNum = ::DragQueryFile((HDROP)wParam, 0xffffffff, NULL, 0);
		WCHAR file[MAX_PATH];
		::DragQueryFile((HDROP)wParam, 0, file, MAX_PATH);//获取文件名
		LoadFile(file);
		break;
	case WM_TOGGLEFULLSCREEN:
		FullScreen_Windowed(!bWindowedFullscreen);
		break;
	case WM_CLOSE:
		if (hWndAid)
			PostMessage(hWndAid, WM_CLOSE, 0, 1);
		if (hWndStartup)
			PostMessage(hWndStartup, WM_CLOSE, 0, 1);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	//	WM_DESTROY	- 窗口关闭
	case WM_DESTROY:
		timeEndPeriod(1);
		PostQuitMessage(0);
		break;
	/*
	//	WM_NCCALCSIZE	- 非客户区尺寸
	case WM_NCCALCSIZE:
		NCCALCSIZE_PARAMS *pnccsp;
		OnWinChange();
		pnccsp = (NCCALCSIZE_PARAMS*)lParam;
		pnccsp->rgrc[0].top = rcWindow.top + 8;
		pnccsp->rgrc[0].bottom = pnccsp->rgrc[0].top + HEIGHTOF(rcWindow)-16;
	//	WM_NCPAINT	- 非客户区绘制
	case WM_NCPAINT:
		break;
	*/
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

short MYCALL1 GetSizeType()
{
	short sizetype = HTNOWHERE;
	POINT ptClient;
	GetCursorPos(&ptClient);
	ScreenToClient(hWndMain, &ptClient);

	int sizeRegW = 0, sizeRegH = 0;
	int sizeRegTop = 0;
	if (winMode == WINMODE_ROUND)
	{
		sizeRegW = WIDTHOF(rcClient);
		sizeRegH = HEIGHTOF(rcClient);
		sizeRegTop = -GetSystemMetrics(SM_CYMENU);//SM_CXBORDER,SM_CXDLGFRAME
	}
	else
	{
		sizeRegW = WIDTHOF(rcWindow);
		sizeRegH = HEIGHTOF(rcWindow);
		ptClient.x += rcClient.left - rcWindow.left;
		ptClient.y += rcClient.top - rcWindow.top;
	}

	if (ptClient.x >= SIZE_NEAR_PIXEL && ptClient.x < sizeRegW - SIZE_NEAR_PIXEL &&
		ptClient.y >= sizeRegTop + SIZE_NEAR_PIXEL && ptClient.y < sizeRegH - SIZE_NEAR_PIXEL)
		sizetype = HTNOWHERE;
	else if (ptClient.x >= -SIZE_NEAR_PIXEL && ptClient.x < SIZE_NEAR_PIXEL &&
		ptClient.y >= sizeRegTop - SIZE_NEAR_PIXEL && ptClient.y < sizeRegTop + SIZE_NEAR_PIXEL)
		sizetype = HTTOPLEFT;
	else if (ptClient.x >= sizeRegW - SIZE_NEAR_PIXEL && ptClient.x < sizeRegW + SIZE_NEAR_PIXEL &&
		ptClient.y >= sizeRegH - SIZE_NEAR_PIXEL && ptClient.y < sizeRegH + SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOMRIGHT;
	else if (ptClient.x >= sizeRegW - SIZE_NEAR_PIXEL && ptClient.x < sizeRegW + SIZE_NEAR_PIXEL &&
		ptClient.y >= sizeRegTop - SIZE_NEAR_PIXEL && ptClient.y < sizeRegTop + SIZE_NEAR_PIXEL)
		sizetype = HTTOPRIGHT;
	else if (ptClient.x >= -SIZE_NEAR_PIXEL && ptClient.x < SIZE_NEAR_PIXEL &&
		ptClient.y >= sizeRegH - SIZE_NEAR_PIXEL && ptClient.y < sizeRegH + SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOMLEFT;
	else if (ptClient.x >= -SIZE_NEAR_PIXEL && ptClient.x < SIZE_NEAR_PIXEL)
		sizetype = HTLEFT;
	else if (ptClient.x >= sizeRegW - SIZE_NEAR_PIXEL && ptClient.x < sizeRegW + SIZE_NEAR_PIXEL)
		sizetype = HTRIGHT;
	else if (ptClient.y >= sizeRegTop - SIZE_NEAR_PIXEL && ptClient.y < sizeRegTop + SIZE_NEAR_PIXEL)
		sizetype = HTTOP;
	else if (ptClient.y >= sizeRegH - SIZE_NEAR_PIXEL && ptClient.y < sizeRegH + SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOM;
	else
		sizetype = HTNOWHERE;

	return sizetype;
}

void MYCALL1 OnWinChange()
{
	GetClientRect(hWndMain, &rcClient);//得 到client区域尺寸
	POINT clienttl = { 0, 0 };
	ClientToScreen(hWndMain, &clienttl);//获得client区域左上角的屏幕坐标
	//得到client真实屏幕区域
	rcClient.left = clienttl.x;
	rcClient.top = clienttl.y;
	rcClient.right += clienttl.x;
	rcClient.bottom += clienttl.y;

	//得到窗口区域
	GetWindowRect(hWndMain, &rcWindow);

	//更新文字信息显示区域
	RefreshTextRect();
}

inline void MYCALL1 RefreshTextRect()
{
	// 状态信息显示区域
	SetRect(&rcFlag, TEXTMARGIN_SIDE, HEIGHTOF(rcClient) - TEXTMARGIN_BOTTOM - 16
		, TEXTMARGIN_SIDE + 600, HEIGHTOF(rcClient) - TEXTMARGIN_BOTTOM);

	// 图片状态显示区域
	SetRect(&rcPicState, WIDTHOF(rcClient) - 100, TEXTMARGIN_TOP
		, WIDTHOF(rcClient) - 10, TEXTMARGIN_TOP + 60);
}

bool MYCALL1 Init()
{
	// 窗口信息、状态
	OnWinChange();
	hCursorMain = LoadCursorFromFile(L"F://PIC//mycur1_16.cur");
	hCursorG = LoadCursorFromFile(L"F://PIC//mycur3.cur");
	GetCursorPos(&ptLastCursor);
	ScreenToClient(hWndMain, &ptLastCursor);

	hProcess = GetCurrentProcess();
	// 状态标志
	bPureWnd = false;
	bWindowedFullscreen = false;
	bOnDrag = false;
	bOnDragzoom = false;
	bDragging = false;
	bDragzooming = false;
	bOnDragRotate = false;
	bOnZoom = false;
	bOnSize = false;
	bSurfRenew = false;
	bNeedForceRenew = false;
	// 选项标志
	mode = MODE_PIC;
	SetWindowMode(WINMODE_INIT);//WINMODE_NORMAL
	BackgroundColor = COLOR_BKG_INIT;
	bFlagsShow = true;
	bInfoShow = true;
	bFpsLimited = true;

	// 计时
	nLoops = 0;
	fps = 0.0f;
	staticFps = STATIC_FPS_NORMAL;
	staticFrameTime = 1000.0f / staticFps;
	normalFps = NORMAL_FPS;
	normalFrameTime = 1000.0f / normalFps;
	avgFps = -1.0f;
	cvgFps = -1.0f;
	fpsCount = 0;
	frameTime = 0.0f;
	procTime = 0.0f;

	// D3D
	mainDevice = NULL;

	InitNonPic();

	// 信息显示
	rcPic = RECT(TEXTMARGIN_SIDE, TEXTMARGIN_TOP + 80, 400, 100);
	rcSurface = RECT(TEXTMARGIN_SIDE, rcPic.bottom, 400, 500);

	RefreshTextRect();

	return true;
}

bool MYCALL1 D3DInit()
{
	if (!pD3DWnd->CreateDevice(D3DFMT_A8R8G8B8, 1))
		return false;

	// DEVICE
	mainDevice = pD3DWnd->GetDevice();
	// BUFFER SIZE CACHE
	pBufferW = pD3DWnd->GetPBufferWidth();
	pBufferH = pD3DWnd->GetPBufferHeight();

	// SURFER
	surfer.BindDevice(mainDevice);
	surfer.BindBuf(pBufferW, pBufferH);

	// FONT
	pD3DWnd->DXCreateFont(&pFontPic, L"Arial Rounded MT Bold"
		, 13, 5, FW_NORMAL, CLEARTYPE_NATURAL_QUALITY);//DEFAULT_QUALITY
	pD3DWnd->DXCreateFont(&pFontFlags, L"Consolas"
		, 15, 0, FW_NORMAL, CLEARTYPE_NATURAL_QUALITY);
	pD3DWnd->DXCreateFont(&pFontPicState, L"苹方 常规"
		, 48, 0, FW_NORMAL, PROOF_QUALITY, 1U, false, 1UL, OUT_TT_PRECIS);
	// CD3DFont
	/*d3dfont1 = new CD3DFont(L"Arial Rounded MT Bold", 12, 0);
	d3dfont1->InitDeviceObjects(mainDevice);
	d3dfont1->RestoreDeviceObjects();*/
	
	// SPRITE
	/*D3DXCreateSprite(mainDevice, &m_sprite);
	D3DXMATRIX matTransform;
	D3DXMatrixIdentity(&matTransform);
	m_sprite->SetTransform(&matTransform);*/

	// OBJECT
	//pD3DWnd->CreateMesh_Custom1(&decorate);//自创模型并保存
	bDecorateOn = true;
	if(bDecorateOn)
		D3DXLoadMeshFromXW(L"crystal.x", D3DXMESH_MANAGED, mainDevice, NULL, NULL, NULL, NULL, &decorate);
	if (decorate && bDecorateOn)
	{
		D3DXMatrixIdentity(&matWorld);
		//D3DXMatrixRotationX(&matWorld, PI_F / 2);

		// 光照
		ZeroMemory(&light, sizeof(light));
		SetLight();

		ZeroMemory(&material, sizeof(D3DMATERIAL9));
		material.Ambient = { 0.3f,0.3f,0.3f,1.0f };
		material.Diffuse = { 1.0f,1.0f,1.0f,0.8f };
		material.Specular = { 0.6f,0.6f,0.6f,1.0f };
		material.Emissive = { 0.0f,0.0f,0.0f,0.0f };
		material.Power = 400.0f;

		// VIEW
		SetView();
	}

	// RENDERSTATE
	SetRenderState();

	// GUI
	g_gui = new CD3DGUISystem(mainDevice);
	g_gui->Bind(pD3DWnd);
	g_gui->SetCallbackEvent(GUICallback);
	g_gui->AddDXFont(L"Consolas", &fontID1, 15, 0, 0, CLEARTYPE_NATURAL_QUALITY);//PixelSix10,20,10

	float bX = 6, bY = 6, bW = 58, bH = 20;
	float bVM = 23;
	g_gui->AddButton(BUTTON_ID_OPEN, bX, bY, bW, bH, L"file");
	bY += bVM;
	g_gui->AddButton(BUTTON_ID_SAVE, bX, bY, bW, bH, L"save");
	bY += bVM;
	g_gui->AddButton(BUTTON_ID_AID, bX, bY, bW, bH, L"aid");
	g_gui->AddButton(BUTTON_ID_FULLSCREEN, bX, 50, bW, bH, L"full", 0xEEAAEEFF, 0, GUI_WINDOCK_RIGHT);
	g_gui->AddEdit(INPUT_IN_CMD, 0, 18.0f, 0, 20.0f, CMDRECT_COLOR_USING, COLOR_CMD_INIT, fontID1, GUI_WINDOCK_BOTTOMHSPAN);
	g_gui->HideControl(INPUT_IN_CMD);
	g_gui->AddBackdrop(L"test.jpg", 0.1f, 0.3f, 0.2f, 0.2f, GUI_WINDOCK_SCALE);

	return true;
}

void OnSave()
{
	if (bPicOn)
	{
		if (pLivePicpack)
			SaveFileWin(pLivePicpack->GetFileName());
	}
}

void OnSaveAs()
{
	if (bPicOn)
	{
		if (pLivePicpack)
			SaveFileWin(pLivePicpack->GetFileName());
	}
}

bool MYCALL1 OnWinInitFile(LPWSTR cmdline)
{
	LPWSTR *szArgList;
	int argCount;
	szArgList = CommandLineToArgvW(cmdline, &argCount);

	if (argCount > 1)
		LoadFile(szArgList[argCount - 1]);

	return false;
}

inline void MYCALL1 DelayFlag()
{
	time_t nowtick = GetTickCount();

	if (bOnZoom)
	{
		if (nowtick - zoomTick > FLAGDELAY_WHEEL || nowtick < zoomTick)
		{
			bOnZoom = false;
			PostMessage(hWndMain, WM_MOUSEWHEELEND, 0, 0);
		}
	}
	if (bOnSize)
	{
		if (nowtick - sizeTick > FLAGDELAY_SIZE || nowtick < sizeTick)
		{
			bOnSize = false;
			//PostMessage(hWndMain, WM_SIZEEND, 0, 0);
		}
	}
	if (bDragzooming)
	{
		if (nowtick - dragzoomTick > FLAGDELAY_DRAGZOOM || nowtick < dragzoomTick)
			bDragzooming = false;
	}
	if (bDragging)
	{
		if (nowtick - dragTick > FLAGDELAY_DRAG || nowtick < dragTick)
			bDragging = false;
	}
	if (bSurfRenew)
	{
		if (nowtick - surfRenewTick > FLAGDELAY_SURFREFRESH || nowtick < surfRenewTick)
			bSurfRenew = false;
	}
}

void MYCALL1 ClearFlag()
{
	ReleaseCapture();// 统一释放
	if (bOnDrag)
	{
		bOnDrag = false;
	}
	if (bOnDragzoom)
	{
		bOnDragzoom = false;
	}
	bDragzooming = false; 
	bOnDragRotate = false;
	bDragging = false;
	bOnZoom = false;
	bOnSize = false;
	surfRenewTick = false;
	bNeedForceRenew = false;
}

void PostPicPackChange(bool bRenew)
{
	UpdateTitle();

	SwitchSurface(pLivePicpack, pLastPicpack, bRenew);
	UpdateSurfaceInfo();
}

void PostPicPackInfoChange()
{
	UpdateLocalPicStr();
	UpdateTitle();
}

void UpdateSurfaceInfo()
{
	POINT ptClient;
	GetCursorPos(&ptClient);
	ScreenToClient(hWndMain, &ptClient);
	surfer.GetCurInfo(&ptClient);
}

void UpdateTitle()
{
	if (pLivePicpack)
		SetWindowTextW(hWndMain, pLivePicpack->GetFileName());// 窗口标题
	else
		SetWindowTextW(hWndMain, L"");
}

void SwitchSurface(PicPack *newpicpack, PicPack *oldpicpack, bool renew)
{
	if (newpicpack != oldpicpack || newpicpack == NULL)
	{
		if (oldpicpack)
			surfer.DeBindPic(oldpicpack);// 解绑
		surfer.BindPic(newpicpack, renew);// 捆绑

		// 标志
		if (renew)
		{
			bSurfRenew = true;// TODO强制设置为已更新？
			surfRenewTick = GetTickCount();

			bNeedForceRenew = true;
			PostMessage(hWndMain, WM_SURFFORCERENEW, 0, 1);

			Render();
		}
	}
}

void PicMove(int dx, int dy)
{
	SF_SRFR(surfer.OnDrag_Custom({ dx, dy }));

	if (bNeedForceRenew)
		PostMessage(hWndMain, WM_SURFFORCERENEW, 0, 1);

	UpdateSurfaceInfo();
}

void PicRestore()
{
	surfer.SurfCenterPR(*pBufferW, *pBufferH);
	surfer.SurfSetZoomPR(1, true);
	surfer.SurfSetRotatePR(0, true);
	SF_SR(surfer.SurfRenew(false));// 恢复图片，不需要加速
	SF_OZ();

	UpdateSurfaceInfo();
}

void PicClipWindow()
{
	POINTi64 surfBase = surfer.GetBase();
	surfBase.x = -surfBase.x;
	surfBase.y = -surfBase.y;
	SF_SRFR(surfer.OnDrag_Custom(surfBase));

	if (bNeedForceRenew)
		PostMessage(hWndMain, WM_SURFFORCERENEW, 0, 1);

	UpdateSurfaceInfo();

	// 客户区和窗口的长宽差
	int WinDiffW = WIDTHOF(rcWindow) - WIDTHOF(rcClient);
	int WinDiffH = HEIGHTOF(rcWindow) - HEIGHTOF(rcClient);
	// 更新窗口位置
	rcWindow.right = rcWindow.left + (LONG)surfer.GetZoomWidth() + WinDiffW;
	rcWindow.bottom = rcWindow.top + (LONG)surfer.GetZoomHeight() + WinDiffH;
	SetWindowPos(hWndMain, HWND_TOP, rcWindow.left, rcWindow.top
		, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top
		, 0);
}

void BeginDragWindow_Inner(POINT ptCur)
{
	int clientW = WIDTHOF(rcClient);
	int clientH = HEIGHTOF(rcClient);
	if (ptCur.x < clientW / 2 + WIDTHOF(rcClient) / 5
		&& ptCur.x > clientW / 2 - WIDTHOF(rcClient) / 5
		&& ptCur.y < clientH / 2 + HEIGHTOF(rcClient) / 5
		&& ptCur.y > clientH / 2 - HEIGHTOF(rcClient) / 5)
	{
		PostMessage(hWndMain, WM_NCLBUTTONDOWN, HTCAPTION, 0);
	}
	else
	{
		if (ptCur.y < clientH / 2)
		{
			if (ptCur.x < clientW / 2)
				PostMessage(hWndMain, WM_NCLBUTTONDOWN, HTTOPLEFT, 0);
			else
				PostMessage(hWndMain, WM_NCLBUTTONDOWN, HTTOPRIGHT, 0);
		}
		else
		{
			if (ptCur.x < clientW / 2)
				PostMessage(hWndMain, WM_NCLBUTTONDOWN, HTBOTTOMLEFT, 0);
			else
				PostMessage(hWndMain, WM_NCLBUTTONDOWN, HTBOTTOMRIGHT, 0);
		}
	}
}

void BeginDragWindow_Custom(POINT ptCur)
{
	PostMessage(hWndMain, WM_NCLBUTTONDOWN, GetSizeType()
		, MAKELPARAM(ptCur.x, ptCur.y));
}

void KeyDownProc(WPARAM wParam)
{
	if (mode == MODE_PIC)
	{
		if (ISCONTROLDOWN)
		{
			switch (wParam)
			{
			case 'F':	// 设置标准窗口尺寸
				OpenFileWin();
				break;
			case 'I':
				bFlagsShow = !bFlagsShow;
				break;
			case 'M':
			// multisample导致图片不显示
				pD3DWnd->ChangeMultiSample();
				ResetDevice();
				break;
			case 'S':
				OnSave();
				break;
			case 'W':	// 改变窗口模式
				if (winMode == WINMODE_ROUND)
					SetWindowMode(WINMODE_NORMAL);
				else
					SetWindowMode(WINMODE_ROUND);
				break;
			}
		}
		else
		{
			switch (wParam)
			{
			case 'A':
			case VK_LEFT:
				if (bPicOn)
					PicMove(-1, 0);
				break;
			case 'B':	// 图片恢复原始像素
				if (bPicOn)
					PicRestore();
				break;
			case 'C':	// 窗口调整到正好包括图片
				if (bPicOn)
					PicClipWindow();
				break;
			case 'D':
			case VK_RIGHT:
				if (bPicOn)
					PicMove(1, 0);
				break;
			case 'E':	// 切换信息显示
				bInfoShow = !bInfoShow;
				break;
			case 'M':	// 图片居中
				if (bPicOn)
					PicCenter();
				break;
			case 'N':
				ToggleNight();
				break;
			case 'Q':	// 图片放回左上角
				if (bPicOn)
					PicDock();
				break;
			case 'R':	// 手动渲染一次
				Render();
				break;
			case 'S':
			case VK_DOWN:
				if (bPicOn)
					PicMove(0, 1);
				break;
			case 'T':
				if (bPicOn)
					PicFit();
				break;
			case 'V':	// 手动重新生成surface一次
				if (bPicOn)
				{
					if (!bOnDragzoom && !bOnZoom)// drazoom 时按 v 在mousemove 中刷新 surface
					{
						SF_SR(surfer.SurfRenew(false));

						UpdateSurfaceInfo();
					}
				}
				break;
			case 'W':	// 改变窗口模式
			case VK_UP:
				if (bPicOn)
					PicMove(0, -1);
				break;
			case 'X':	// 重置收敛帧率值 ( 重新开始计算 )
				fpsCount = 0;
				cvgFps = -1;
				surfer.ClearTimeInfo();
				break;
			case VK_F1:	// 显示显卡信息			
				pD3DWnd->DisplayAdapter();
				break;
			case VK_F3:
				PureWindow(!bPureWnd);
				break;
			case VK_F4:
				PostMessage(hWndMain, WM_TOGGLEFULLSCREEN, 0, 1);
				break;
			case VK_OEM_2:// 进入命令行模式
				EnterCMDMode();
				break;
			case VK_ESCAPE:
				FullScreen_Windowed(false);
				break;
			case VK_OEM_MINUS:
				SetPrevPic();
				PostPicPackChange();
				break;
			case VK_OEM_PLUS:
			case VK_TAB:
				SetNextPic();
				PostPicPackChange();
				break;
			case VK_DELETE:
				Drop();
				PostPicPackChange();
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
			break;
		}
	}
}

void OpenFileWin()
{
	OPENFILENAME opfn;
	WCHAR openfilename[MAX_PATH];//存放文件名  

	// 初始化     
	ZeroMemory(&opfn, sizeof(OPENFILENAME));
	opfn.lStructSize = sizeof(OPENFILENAME);//结构体大小
	opfn.lpstrFilter = L"所有文件\0*.*\0bmp文件\0*.bmp\0png文件\0*.png\0jpg文件\0*.jpg\0";//设置过滤    
	opfn.nFilterIndex = 1;//默认过滤器索引
	opfn.lpstrFile = openfilename;
	opfn.lpstrFile[0] = '\0';// 文件名的字段必须先把第一个字符设为\0
	opfn.nMaxFile = sizeof(openfilename);
	opfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;//标志位:检查目录或文件是否存在
	opfn.hwndOwner = hWndMain;//模态
	//opfn.lpstrInitialDir = NULL;   

	// 显示对话框选择文件     
	if (GetOpenFileName(&opfn))
	{
		// 选中文件后操作
		LoadFile(openfilename);
	}
}

void SaveFileWin(const WCHAR file[])
{
	OPENFILENAME svfn;
	WCHAR savefilename[MAX_PATH] = { 0 };//存放文件名 
	if(file)
		StringCchCopy(savefilename, MAX_PATH, file);

	// 初始化     
	ZeroMemory(&svfn, sizeof(OPENFILENAME));
	svfn.lStructSize = sizeof(OPENFILENAME);//结构体大小
	svfn.lpstrFilter = L"所有文件\0*.*\0bmp文件\0*.bmp\0png文件\0*.png\0jpg文件\0*.jpg\0";//设置过滤   
	svfn.nFilterIndex = 1;//默认过滤器索引
	
	WCHAR *ftype = wcsrchr(savefilename, L'.');
	if (ftype)
	{
		if (_wcsicmp(ftype, L".bmp") == 0)
			svfn.nFilterIndex = 2;
		else if(_wcsicmp(ftype, L".png") == 0)
			svfn.nFilterIndex = 3;
		else if(_wcsicmp(ftype, L".jpg") == 0)
			svfn.nFilterIndex = 4;

	}
	svfn.lpstrFile = savefilename;
	//svfn.lpstrFile[0] = '\0';
	svfn.nMaxFile = sizeof(savefilename);
	svfn.Flags = OFN_OVERWRITEPROMPT ;//标志位:覆盖提醒
	svfn.hwndOwner = hWndMain;//模态
	//svfn.lpstrInitialDir = NULL;     
	// 显示对话框让用户选择文件     
	if (GetSaveFileName(&svfn))
	{
		ftype = wcsrchr(savefilename, L'.');
		if (!ftype)
		{
			if (svfn.nFilterIndex == 2)
				StringCchCat(savefilename, MAX_PATH, L".bmp");
			else if (svfn.nFilterIndex == 3)
				StringCchCat(savefilename, MAX_PATH, L".png");
			else if (svfn.nFilterIndex == 4)
				StringCchCat(savefilename, MAX_PATH, L".jpg");
		}
		
		SaveFile(savefilename);
	}
}

bool SaveFile(WCHAR file[])
{
	if (pLivePicpack == NULL)
		return false;

	if (pLivePicpack->SaveFile(mainDevice, file))
	{
		PostPicPackInfoChange();

		return true;
	}
	else
	{
		MessageBoxW(hWndMain, L"Save File FAILED!", L"ERROR", MB_OK | MB_APPLMODAL);
		return false;
	}
}

bool MYCALL1 LoadFile(WCHAR file[])
{
	if (!mainDevice)
	{
		return false;
	}

	PicPack *newpp = new PicPack;
	HRESULT hr = newpp->LoadFile(mainDevice, file);// 加载新图片
	if (SUCCEEDED(hr))
	{
		// 图片列表扩充、更新当前图片
		picList.Add(newpp);
		SetNewPic();

		PostPicPackChange(false);

		// 居中
		surfer.SurfCenterPR(*pBufferW, *pBufferH);
		if (pLiveBmp)
		{
			if (pLiveBmp->width > *pBufferW*PICMAXSCALE_INIT || pLiveBmp->height > *pBufferH*PICMAXSCALE_INIT)
			{
				double zoomNew = min(*pBufferW*PICMAXSCALE_INIT / pLiveBmp->width, *pBufferH*PICMAXSCALE_INIT / pLiveBmp->height);
				surfer.SurfSetZoomPR(zoomNew, true);
			}
		}
		bSurfRenew = surfer.SurfRenew(true);
		if (bSurfRenew)
			surfRenewTick = GetTickCount();
		bNeedForceRenew |= bSurfRenew;
		if (bNeedForceRenew)// 如果拖动过程中需要更新图片，在结束时用合适方法重新生成 surface
			PostMessage(hWndMain, WM_SURFFORCERENEW, 0, 1);

		// 更新 surface 信息 （因为又进行了次居中）TODO：居中操作合并
		UpdateSurfaceInfo();

		return true;
	}
	else
	{
		delete newpp;// 释放

		SetForegroundWindow(hWndMain);
		D3DErrorShow(hr, L"Load File", hWndMain);// 显示错误信息

		return false;
	}
}

void SetWindowMode(UINT8 wmode)
{
	//设置矩形区域
	winMode = wmode;
	if (winMode == WINMODE_ROUND)
	{
		HRGN hrgn;

		RECT rcRgn;
		rcRgn.left = rcClient.left - rcWindow.left;
		rcRgn.top = rcClient.left - rcWindow.left;
		rcRgn.right = WIDTHOF(rcWindow) - rcRgn.left + 1;
		rcRgn.bottom = HEIGHTOF(rcWindow) - rcRgn.top + 1;
		hrgn = CreateRoundRectRgn(rcRgn.left, rcRgn.top
			, rcRgn.right, rcRgn.bottom, 2, 2);//尺寸，要+1
		SetWindowRgn(hWndMain, hrgn, TRUE);

		DeleteObject(hrgn);
		//SetClassLongPtr(hWndMain, GCL_STYLE
		//	, GetClassLong(hWndMain, GCL_STYLE) | CS_DROPSHADOW);//阴影
		//SetWindowLong(hWndMain, GWL_STYLE, GetWindowLong(hWndMain, GWL_STYLE)| CS_DROPSHADOW);
	}
	else
	{
		SetWindowRgn(hWndMain, NULL, TRUE);//恢复正常窗口

		//SetClassLong(hWndMain, GCL_STYLE
		//	, GetClassLong(hWndMain, GCL_STYLE) & (~CS_DROPSHADOW));//阴影
	}
}

inline void MaintainWindowStyle()
{
	if (winMode == WINMODE_ROUND)
	{
		HRGN hrgn;

		RECT rgnrect;
		rgnrect.left = rcClient.left - rcWindow.left;
		rgnrect.top = 8;
		rgnrect.right = rcWindow.right - rcClient.left + 1;//rcWindow.right - rcClient.left + 1
		rgnrect.bottom = HEIGHTOF(rcWindow) - rgnrect.top + 1;//HEIGHTOF(rcWindow) - rcRgn.top + 1
		if (bWindowedFullscreen)
			hrgn = CreateRectRgn(rgnrect.left, rgnrect.top
				, rgnrect.right, rgnrect.bottom);
		else
			hrgn = CreateRoundRectRgn(rgnrect.left, rgnrect.top
				, rgnrect.right, rgnrect.bottom, 2, 2);
		SetWindowRgn(hWndMain, hrgn, TRUE);

		DeleteObject(hrgn);
	}
}

void MYCALL1 BeginDragPic()
{
	SetCapture(hWndMain);//允许鼠标在窗口外拖动

	POINT ptClient;
	GetCursorPos(&ptClient);
	ScreenToClient(hWndMain, &ptClient);
	ptLastCursor = ptClient;

	bOnDrag = true;
}

void MYCALL1 EndDragPic()
{
	ReleaseCapture();

	bOnDrag = false;
	bDragging = false;

	if (bNeedForceRenew)//如果拖动过程中需要更新图片，在结束时用合适方法重新生成surface
		PostMessage(hWndMain, WM_SURFFORCERENEW, 0, 1);
}

void MYCALL1 BeginDragZoomPic()
{
	SetCapture(hWndMain);//允许鼠标在窗口外拖动

	POINT ptClient;
	GetCursorPos(&ptClient);
	ScreenToClient(hWndMain, &ptClient);
	surfer.SetBasePoint(ptClient);// 设置基准点
	surfer.GetCurInfo(&ptClient);

	ptLastCursor = ptClient;

	bOnDragzoom = true;
}

void MYCALL1 EndDragZoomPic()
{
	ReleaseCapture();

	bOnDragzoom = false;
	bDragzooming = false;

	if (bNeedForceRenew)//如果拖动放大过程中需要更新图片，在结束时用合适方法重新生成surface
		PostMessage(hWndMain, WM_SURFFORCERENEW, 0, 1);
}

void BeginDragRotatePic()
{
	SetCapture(hWndMain);//允许鼠标在窗口外拖动

	POINT ptClient;
	GetCursorPos(&ptClient);
	ScreenToClient(hWndMain, &ptClient);
	surfer.SetBasePoint(ptClient);// 设置基准点
	surfer.GetCurInfo(&ptClient);

	ptLastCursor = ptClient;

	bOnDragRotate = true;
}

void EndDragRotatePic()
{
	ReleaseCapture();

	bOnDragRotate = false;

	if (bNeedForceRenew)//如果拖动放大过程中需要更新图片，在结束时用合适方法重新生成surface
		PostMessage(hWndMain, WM_SURFFORCERENEW, 0, 1);
}

void MYCALL1 EnterCMDMode()
{
	mode = MODE_CMD;
	g_gui->ShowControl(INPUT_IN_CMD);
	g_gui->SetFocus(INPUT_IN_CMD);
}

void MYCALL1 ExitCMDMode()
{
	mode = MODE_PIC;
	g_gui->SetControlText(INPUT_IN_CMD, L"");
	g_gui->HideControl(INPUT_IN_CMD);
}

void PicDock()
{
	POINTi64 offBase = surfer.GetBase();
	offBase.x = -offBase.x;
	offBase.y = -offBase.y;
	SF_SRFR(surfer.OnDrag_Custom(offBase));

	if (bNeedForceRenew)
		PostMessage(hWndMain, WM_SURFFORCERENEW, 0, 1);

	UpdateSurfaceInfo();
}

void PicCenter()
{
	surfer.SurfCenterPR(*pBufferW, *pBufferH);
	SF_SR(surfer.SurfRenew(false));

	UpdateSurfaceInfo();
}

void PicFit()
{
	SF_SR(surfer.SurfSuit(*pBufferW, *pBufferH));

	UpdateSurfaceInfo();
}

bool FullScreen_Windowed(bool tofull, bool restore)
{
	if (bWindowedFullscreen == tofull)
		return false;
	bWindowedFullscreen = tofull;
	if (bWindowedFullscreen)// 全屏化
	{
		rcOriginalWnd = rcWindow;
		RECT fullScreenRect;
		fullScreenRect.left = rcWindow.left - rcClient.left;
		fullScreenRect.top = rcWindow.top - rcClient.top + 22;
		if (bPureWnd)
			fullScreenRect.top -= 22;
		fullScreenRect.right = rcWindow.right
			- rcClient.right + GetSystemMetrics(SM_CXSCREEN) + 12;
		fullScreenRect.bottom = rcWindow.bottom
			- rcClient.bottom + GetSystemMetrics(SM_CYSCREEN);
		nLoops = GetSystemMetrics(SM_CXSCREEN);

		// 隐藏任务栏
		/*HWND taskwnd, startbutton;
		taskwnd = FindWindow(L"Shell_TrayWnd", NULL);
		ShowWindow(taskwnd, SW_HIDE);
		startbutton = FindWindow(_T("Button"), NULL);
		ShowWindow(startbutton, SW_HIDE);
		taskwnd=GetDlgItem(FindWindow(L"Shell_TrayWnd", NULL), 0x130);
		ShowWindow(taskwnd, SW_HIDE);*/

		// 去除边框，顺利进入全屏
		LONG tmp = GetWindowLong(hWndMain, GWL_STYLE);
		tmp &= ~WS_BORDER;
		tmp |= WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		SetWindowLong(hWndMain, GWL_STYLE, tmp);
		bPureWnd = true;

		SetWindowPos(hWndMain, HWND_TOP, fullScreenRect.left, fullScreenRect.top
			, fullScreenRect.right, fullScreenRect.bottom + 8
			, SWP_FRAMECHANGED);//+8

		g_gui->SetControlText(BUTTON_ID_FULLSCREEN, L"small");
	}
	else// 取消全屏
	{
		// 显示任务栏
		/*HWND taskwnd, startbutton;
		taskwnd = FindWindow(L"Shell_TrayWnd", NULL);
		ShowWindow(taskwnd, SW_SHOW);
		startbutton = FindWindow(_T("Button"), NULL);
		ShowWindow(startbutton, SW_SHOW);
		taskwnd = GetDlgItem(FindWindow(L"Shell_TrayWnd", NULL), 0x130);
		ShowWindow(taskwnd, SW_SHOW);*/

		LONG tmp = GetWindowLong(hWndMain, GWL_STYLE);
		tmp |= WS_POPUPWINDOW;
		tmp |= WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		SetWindowLong(hWndMain, GWL_STYLE, tmp);
		bPureWnd = false;

		if (restore)
		{
			//SetWindowPos比MoveWindow发送更少的消息，加速，减缓图片闪烁
			SetWindowPos(hWndMain, HWND_TOP, rcOriginalWnd.left, rcOriginalWnd.top
				, WIDTHOF(rcOriginalWnd), HEIGHTOF(rcOriginalWnd)
				, SWP_FRAMECHANGED);
		}
		g_gui->SetControlText(BUTTON_ID_FULLSCREEN, L"full");
	}

	return true;
}

bool PureWindow(bool topure)
{
	if (bPureWnd == topure)
		return false;
	bPureWnd = topure;

	// 更改窗口属性
	LONG oldwindowlong = GetWindowLong(hWndMain, GWL_STYLE);
	if (bPureWnd)
	{
		oldwindowlong &= ~WS_BORDER;
	}
	else
	{
		oldwindowlong |= WS_POPUPWINDOW;
	}
	SetWindowLong(hWndMain, GWL_STYLE, oldwindowlong);

	// 使得窗口属性更改后，外观立刻更新
	SetWindowPos(hWndMain, HWND_TOP, rcWindow.left, rcWindow.top
		, WIDTHOF(rcWindow), HEIGHTOF(rcWindow)
		, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOSENDCHANGING);

	return false;
}

void SetStaticFps(float sfps)
{
	if (sfps > 0)
	{
		staticFps = (short)sfps;
		staticFrameTime = 1000.0f / sfps;
	}
}

void ToggleNight()
{
	if (BackgroundColor == COLOR_BKG_INIT)
	{
		BackgroundColor = COLOR_BKGNIGHT;
		//surfer.SetBackcolor(COLOR_BKGNIGHT);
	}
	else
	{
		BackgroundColor = COLOR_BKG_INIT;
		//surfer.SetBackcolor(COLOR_BKG_INIT);
	}
}

inline void MYCALL1 SetRenderState()
{
	// 标准 blend：D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA
	mainDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	mainDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	mainDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	mainDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	mainDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	mainDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	mainDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	mainDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	mainDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	mainDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	//mainDevice->SetRenderState(D3DRS_AMBIENT,
	//	D3DCOLOR_COLORVALUE(0.3f, 0.3f, 0.3f, 1.0f));

	if (decorate && bDecorateOn)
	{
		mainDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
		mainDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);

		mainDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);// 默认 D3DMCS_COLOR1
		mainDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);// 默认 D3DMCS_MATERIAL
		//mainDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);// 默认 D3DMCS_COLOR2
		//mainDevice->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);// 默认 D3DMCS_MATERIAL

		mainDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		//mainDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
		//mainDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		//mainDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);//D3DCULL_NONE,D3DCULL_CCW
	}
}

inline void SetView()
{
	D3DXVECTOR3 eye;
	D3DXVECTOR3 at;
	D3DXVECTOR3 up;
	float eyeradius = 60.0f;//40.0f
	eye = D3DXVECTOR3(0.0f, 0.0f, -eyeradius);
	at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &eye, &at, &up);
	mainDevice->SetTransform(D3DTS_VIEW, &matView);
	D3DXMATRIXA16 proj;
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 3
		, (float)WIDTHOF(rcClient) / HEIGHTOF(rcClient)
		, 0.02f, 100.0f);
	mainDevice->SetTransform(D3DTS_PROJECTION, &proj);

	time_t nowtick = GetTickCount();
	const float angle = nowtick > decorateTick ? 0.0014f*(nowtick - decorateTick) : 0;
	decorateTick = nowtick;

	D3DXMATRIX rotnew;
	D3DXMatrixRotationY(&rotnew, -angle);// 逆时针旋转，取负号（Y轴朝上）
	//D3DXMatrixTranslation(&rotnew, -0.1, 0, 0);
	D3DXMatrixMultiply(&matWorld, &matWorld, &rotnew);
	mainDevice->SetTransform(D3DTS_WORLD, &matWorld);
}

inline void SetLight()
{
	light.Type = D3DLIGHT_POINT;
	float radius = 20.0f;
	D3DXVECTOR3 vLightDir = D3DXVECTOR3(-4.0f, -1.0f, -5.0f);
	D3DXVec3Normalize(&vLightDir, &vLightDir);
	light.Position = vLightDir*radius;// 光源在观察者同侧，左部偏下
	light.Range = 35.0f;
	float iD = 1.0f;
	float iA = 0.6f;
	float iS = 1.0f;
	light.Diffuse = { iD,iD,iD,1.0f };
	light.Ambient = { iA,iA,iA,1.0f };
	light.Specular = { iS,iS,iS,1.0f };
	light.Attenuation0 = 0.5f;
	light.Attenuation1 = 0.04f;
	mainDevice->SetLight(0, &light);
	mainDevice->LightEnable(0, TRUE);
}

bool InfoRender()
{
	WCHAR infowstr[512] = { 0 };

	// 图片状态信息显示
	StringCchPrintfW(infowstr, 512, L"%d/%d", picList.GetCurPos(), picList.GetCount());
	if (pFontPicState != NULL)
		pFontPicState->DrawTextW(NULL, infowstr, -1, &rcPicState, DT_RIGHT | DT_NOCLIP, COLOR_TEXT4);

	// 第1部分信息显示（图片信息）
	// d3dfont1->DrawTextW(100, 200, COLOR_TEXT1, infowstr0, 0);//测试高速字体绘制
	if (pFontPic != NULL)
	{
		pFontPic->DrawTextW(NULL, picInfoStr, -1, &rcPic, DT_LEFT | DT_NOCLIP, COLOR_TEXT1);
	}

	// 窗口信息 & surfer信息
	POINT ptClient;
	GetCursorPos(&ptClient);
	ScreenToClient(hWndMain, &ptClient);
	StringCchPrintfW(infowstr, 521, L"MODE: %d\n\
		FPS: %.2f/%.2f (%.2f)  %.3fms   %lld\n\
		MEM: %.1fMB,  %.1fMB\n\
		BUF: %d × %d\n\
		CLIENT: %d, %d\n\
		PROC: %.3fms\n\
		MULTISAMPLE: %d\n"
		, mode
		, fps, avgFps, cvgFps, frameTime, nLoops
		, memoryIn, memoryOut
		, *pBufferW, *pBufferH
		, ptLastCursor.x, ptLastCursor.y
		, procTime
		, pD3DWnd->GetMultiSample());
	StringCchCat(infowstr, 512, L"   -    -    -    -   \n");
	StringCchCat(infowstr, 512, surfer.GetInfoStr());
	if (pFontPic != NULL)
		pFontPic->DrawTextW(NULL, infowstr, -1, &rcSurface, DT_LEFT | DT_NOCLIP, COLOR_TEXT1 );
	//pFontPic->PreloadText();

	// 附加信息
	//const string cursorposshow[3] = { "PIC", "BLANK", "OUTSIDE CLIENT" };
	//	"BackgroundColor: %08X.ARGB\n
	//	screen color: %02X.%06X.ARGB
	//	cursor: %d, %d\n\"
	//	, (screenColor >> 24), (screenColor & 0xFFFFFF)
	//	, cursor.x, cursor.y

	// 标志信息
	if (bFlagsShow)
	{
		const WCHAR yesno2[2] = { L'×', L'●' };//'√'
		StringCchPrintfW(infowstr, 512,
			L"PIC:%lc\
		  SURF:%lc\
		  ZOOM:%lc\
		  DRAG:%lc\
		  DRAGZ:%lc\
		  SIZE:%lc\
		  CLIP:%lc\
		  SCLIP:%lc\
		  PCLIP:%lc\
		  OUT:%lc\
		  RENEW:%lc"
			, yesno2[bPicOn]
			, yesno2[surfer.IsSurfNotNull()], yesno2[bOnZoom]
			, yesno2[bDragging], yesno2[bDragzooming]
			, yesno2[bOnSize], yesno2[surfer.bClip]
			, yesno2[surfer.GetBoolSurfClipped()], yesno2[surfer.bPicClipped]
			, yesno2[surfer.bOutClient], yesno2[bSurfRenew]);

		// 第3部分信息显示
		if (pFontFlags != NULL)
			pFontFlags->DrawTextW(NULL, infowstr, -1, &rcFlag
			, DT_LEFT | DT_TOP | DT_NOCLIP, COLOR_TEXT3);//COLOR_TEXT3
	}

	return true;
}

void MYCALL1 Render()
{
	static HRESULT hr;

	if (mainDevice == NULL)
		return;

	mainDevice->Clear(0, NULL, D3DCLEAR_TARGET/* | D3DCLEAR_ZBUFFER*/, BackgroundColor, 1.0f, 0);
	mainDevice->BeginScene();
	//Sleep(1);

	if(g_gui != NULL)
		g_gui->RenderBack();

	// 装饰绘制
	if (decorate && bDecorateOn)
	{
		SetView();
		mainDevice->SetMaterial(&material);
		mainDevice->SetTexture(NULL, NULL);
		mainDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		mainDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		mainDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
		decorate->DrawSubset(0);
		mainDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	}

	// 图片绘制
	hr = surfer.Render();

	if (bInfoShow)//信息显示
		InfoRender();

	// D3DGUI
	if (g_gui != NULL)
		g_gui->Render();//D3DBLEND_INVSRCCOLOR,D3DBLEND_INVSRCCOLOR;D3DBLEND_ZERO,D3DBLEND_INVDESTCOLOR

	mainDevice->EndScene();
	hr = mainDevice->Present(NULL, NULL, NULL, NULL);

	// 处理设备丢失
	if (hr == D3DERR_DEVICELOST)
	{
		if (mainDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ResetDevice();
		}
	}
}

bool ResetDevice()
{
	if (mainDevice == NULL)
		return false; 

	// 修改device长宽
	OnLostDevice();
	bool result = OnResetDevice();

	PostResetDevice();

	return result;
}

void OnLostDevice()
{
	pD3DWnd->OnLostDevice();

	g_gui->OnLostDevice();

	//D3D
	pFontPic->OnLostDevice();
	pFontFlags->OnLostDevice();
	pFontPicState->OnLostDevice();
}

bool OnResetDevice()
{
	if (!pD3DWnd->OnResetDevice())
	{
		return false;
	}

	g_gui->OnResetDevice();

	pFontPic->OnResetDevice();
	pFontFlags->OnResetDevice();
	pFontPicState->OnResetDevice();

	return true;
}

bool OnResetDevice2(int clientw, int clienth)
{
	if (!pD3DWnd->OnResetDevice(clientw, clienth))
	{
		return false;
	}

	g_gui->OnResetDevice();

	pFontPic->OnResetDevice();
	pFontFlags->OnResetDevice();
	pFontPicState->OnResetDevice();

	return true;
}

inline void PostResetDevice()
{
	// D3D更新,设备丢失后必做
	if (decorate && bDecorateOn)
	{
		SetLight();
		SetView();
	}
	SetRenderState();
}

void CALLBACK GUICallback(int ID, WPARAM wp, LPARAM lp)
{ 
	switch (ID)
	{
	case BUTTON_ID_OPEN:
		if (wp == GUI_EVENT_LBUTTONCLICKED)
		{
			PostMessage(hWndMain, WM_COMMAND, IDM_OPEN, 0);
		}
		break;
	case BUTTON_ID_SAVE:
		if (wp == GUI_EVENT_LBUTTONCLICKED)
		{
			PostMessage(hWndMain, WM_COMMAND, IDM_SAVE, 0);
		}
		break;
	case BUTTON_ID_AID:
		if (wp == GUI_EVENT_LBUTTONCLICKED)
		{
			//memset(&si, 0, sizeof(si));
			//si.cb = sizeof(STARTUPINFO);
			//si.dwFlags = STARTF_USESHOWWINDOW;
			//si.wShowWindow = SW_SHOW;//SW_HIDE隐藏窗口  

			//WCHAR chCommandLine[MAX_PATH];
			//StringCchCopy(chCommandLine, MAX_PATH, L".\\picture.exe");

			//BOOL ret = CreateProcessW(NULL, chCommandLine, NULL, NULL, FALSE
			//	, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
			//int nError = GetLastError();

			//if (ret)
			//{
			//	hWndAid = GetHwndByPid(pi.dwProcessId);
			//	hWndAid = GetHwndByProcessId(pi.dwProcessId);
			//	//MessageBox(hWndMain, L"aid succeed", L"aid result", 0);
			//}
			//else
			//{
			//	WCHAR strInfo[64];
			//	StringCchPrintf(strInfo, 64, L"创建进程失败-%d", nError);
			//	MessageBox(hWndMain, strInfo, L"aid result", 0);
			//}RECT rcMain;

			if (startup == NULL)
			{
				startup = new D3DWnd;
				if (startup == NULL)
					break;
			}
			RECT rcMain;
			GetWindowRect(hWndMain, &rcMain);
			float fBias = 0.618f;
			POINT ptMid = { (LONG)(rcMain.left + WIDTHOF(rcMain)*fBias)
				, (LONG)(rcMain.top + HEIGHTOF(rcMain)*fBias) };
			int iW = 240, iH = 220;
			if (startup->D3DCreateWindow(L"startUp", StartWndProc, 0
				, CS_DROPSHADOW, WS_POPUP, 0
				, ptMid.x - iW / 2, ptMid.y - iH / 2, iW, iH
				, 0, 0, 0, 0, 0xFFD0BBFF))
			{
				hWndStartup = startup->GetHWND();
				//SetWindowPos(hWndStartup, HWND_NOTOPMOST,	0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
			DoModal(&hWndStartup, hWndMain);
		}
		break;
	case BUTTON_ID_FULLSCREEN:
		if (wp == GUI_EVENT_LBUTTONCLICKED)
		{
			PostMessage(hWndMain, WM_TOGGLEFULLSCREEN, 0, 1);
		}
		break;
	case INPUT_IN_CMD:
		if (wp == GUI_EVENT_CMD)
			CMDProc((WCHAR*)lp);
		break;
	}
}

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

	LARGE_INTEGER s = { 0 }, e = { 0 }, freq = { 0 };
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&s);
	if (_wcsicmp(wstr, L"quit") == 0)
	{
		PostMessage(hWndMain, WM_QUIT, NULL, NULL);
	}
	else if (_wcsicmp(wstr, L"open") == 0)
	{
		OpenFileWin();
	}
	else if (_wcsicmp(wstr, L"exit") == 0)
	{
		ExitCMDMode();
	}
	else if (_wcsicmp(wstr, L"gray") == 0)
	{
		if (bPicOn)
		{
			pLiveBmp->Gray();
			bSurfRenew = surfer.SurfRenew(false);

			if (bSurfRenew)
				surfRenewTick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"loser") == 0)
	{
		if (bPicOn)
		{
			pLiveBmp->LOSE_R();
			bSurfRenew = surfer.SurfRenew(false);

			if (bSurfRenew)
				surfRenewTick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"loseg") == 0)
	{
		if (bPicOn)
		{
			pLiveBmp->LOSE_G();
			bSurfRenew = surfer.SurfRenew(false);

			if (bSurfRenew)
				surfRenewTick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"loseb") == 0)
	{
		if (bPicOn)
		{
			pLiveBmp->LOSE_B();
			bSurfRenew = surfer.SurfRenew(false);

			if (bSurfRenew)
				surfRenewTick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"inv") == 0)
	{
		if (bPicOn)
		{
			pLiveBmp->Inverse();
			bSurfRenew = surfer.SurfRenew(false);

			if (bSurfRenew)
				surfRenewTick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"hsv") == 0)
	{
		if (bPicOn)
		{
			pLiveBmp->RGB2HSV();
			bSurfRenew = surfer.SurfRenew(false);

			if (bSurfRenew)
				surfRenewTick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"diff") == 0)
	{
		if (bPicOn)
		{
			BMP *pre = GetPrevPic();
			if (pre != NULL)
			{
				pLiveBmp->Diff(pre);
				bSurfRenew = surfer.SurfRenew(false);

				if (bSurfRenew)
					surfRenewTick = GetTickCount();
			}
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
			if (bPicOn && num > ZOOM_MIN)
			{
				surfer.SurfSetZoomPR(num, true);
				SF_SR(surfer.SurfRenew(false));

				bOnZoom = true;
				zoomTick = GetTickCount();
			}
		}
	}
	//获取 surface 信息（图片更新后，surface 信息也立即更新）
	UpdateSurfaceInfo();

	QueryPerformanceCounter(&e);
	procTime = 1000.0f*(e.QuadPart - s.QuadPart) / (float)freq.QuadPart;
}

//void CALLBACK TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
//{
//	if (uID == TimerID_main)
//	{
//		timershot = true;
//	}
//}
