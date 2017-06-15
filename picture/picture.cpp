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
LRESULT CALLBACK    StartWndProc(HWND, UINT, WPARAM, LPARAM);
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

	// 启动界面
	/*D3DWnd *startup = new D3DWnd();
	HWND startwnd = NULL;
	if (startup->D3DCreateWindow(L"", StartWndProc, 0
		, 0, WS_OVERLAPPEDWINDOW, 0
		, WINDOWPOSX_INIT + 280, WINDOWPOSY_INIT + 140, 240, 260
		, 0, 0, 0, 0, 0x11D0BBFF))
	{
		startwnd = startup->GetHWND();
	}*/

	// 初始化参数
	Init();
	D3DInit();
	//TimerID_main = timeSetEvent(16, 0, TimerProc, 0, TIME_PERIODIC);//多媒体定时器

	// 初始窗口启动参数
	OnWininitFile(GetCommandLine());

	// 销毁启动窗口
	//if(startwnd)
	//	SendMessage(startwnd, WM_CLOSE, 0, 0);
	//delete startup;

	// 舍弃的方法
    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PICTURE));

	// 优先级
	//SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	// 循环参数初始化
	time_t nowtime = 0, lasttime = 0;//计时
	LARGE_INTEGER frequency, stime, etime;
	QueryPerformanceFrequency(&frequency);

    // 主消息循环: 
	timeBeginPeriod(1);//精度设置
	MSG msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
	while (msg.message != WM_QUIT)
	{
		/********************* 发布消息前做的动作 *********************/
		// 更新鼠标坐标,在发布消息前更新鼠标位置，防止消息处理函数中数据错误(图片拖动问题)
		GetCursorPos(&cursor);

		// 消息
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		/********************* 发布消息后做的动作 *********************/
		// 最小化状态拦截
		if (bIconic)//ERROR 有时退出时会崩溃，指向这行！
		{
			Sleep(ICONIC_SLEEP);//最小化状态降低资源消耗
			continue;
		}

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
			else//静止状态，低帧率刷新界面
			{
				if (testfps > staticFps*1.05f)// *1.05控制帧率准确度
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

		//动态调整帧率限制
		/*if (bFpsLimited)
		{
			if (bDragging || bOnZoom || bDragzooming)
			{
				if (testfps < FPS_ERROR_DOWN)
				{
					normalFps += 0.2f;
					normalFrameTime = 1000.0f / normalFps;
				}
				else if (testfps > FPS_ERROR_UP)
				{
					if (normalFps > 0)
						normalFps -= 0.2f;
					normalFrameTime = 1000.0f / normalFps;
				}
				else
				{
					if (normalFps > NORMAL_FPS)
						normalFps -= 0.2f;
					normalFrameTime = 1000.0f / normalFps;
				}
			}
		}*/

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
			if (bFpsLimited)							//当前帧率
			{
				fps = testfps;
			}
			else
			{
				fps = (float)frequency.QuadPart / (etime.QuadPart - stime.QuadPart);
			}
			
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
LRESULT CALLBACK StartWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		POINT clienttl;
		RECT crect, wrect;
	case WM_CREATE:
		/*GetWindowRect(hWnd, &wrect);
		SetWindowPos(hWnd, HWND_TOPMOST
			, wrect.left, wrect.top, wrect.right, wrect.bottom, 0);*/
		break;
	case WM_PAINT:
		clienttl = { 0, 0 };
		GetClientRect(hWnd, &crect);//得到client区域尺寸
		ClientToScreen(hWnd, &clienttl);//获得client区域左上角的屏幕坐标
										   //得到client真实屏幕区域
		crect.left = clienttl.x;
		crect.top = clienttl.y;
		crect.right += clienttl.x;
		crect.bottom += clienttl.y;

		//得到窗口区域
		GetWindowRect(hWnd, &wrect);

		HRGN hrgn;

		RECT rgnrect;
		rgnrect.left = crect.left - wrect.left;
		rgnrect.top = 30;
		rgnrect.right = WIDTHOF(wrect) - rgnrect.left + 1/*WIDTHOF(rcClient) + 9*/;
		rgnrect.bottom = HEIGHTOF(wrect) - rgnrect.top + 1/*HEIGHTOF(rcClient) + 22*/;
		hrgn = CreateRoundRectRgn(rgnrect.left, rgnrect.top
			, rgnrect.right, rgnrect.bottom, 2, 2);//尺寸，要+1
		SetWindowRgn(hWnd, hrgn, TRUE);

		break;
	case WM_LBUTTONDOWN:
		PostMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
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
				if(pLivePicpack)
					SaveFileWin(pLivePicpack->GetFileName());
				break;
			case IDM_CLEAR:
				Drop();
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	//	WM_PAINT	- 处理窗口重绘，需要高效
    case WM_PAINT:
        {
			if (!bIconic)
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
		//不采取动作，防止窗口重绘时闪烁
		break;
	//	WM_MOUSEMOVE	- 鼠标移动，需要高效
	case WM_MOUSEMOVE:
		if (bPicOn)
		{
			if (bOnDrag)//*****************************************高效*******************************************
			{
				/*mainbmp->TestInc();
				surfer.SurfRenew(false);*/
				SF_SRFR(surfer.OnMove_Custom(MINUSPOINT(cursor, lastCursor)));
				SF_DR();
			}
			else if (bOnDragzoom)//*****************************************高效********************************
			{
				surfer.SurfAdjustZoom_DragPR(cursor.x - lastCursor.x);//调整放大倍率
				bSurfRenew = surfer.SurfZoomRenew(NULL, ISALTDOWN, !ISKEYDOWN('V'));

				bNeedForceRenew = true;

				// 标志
				if (bSurfRenew)
					surfRenewTick = GetTickCount();
				bDragzooming = true;
				dragzoomTick = GetTickCount();
			}
			else
			{
				g_gui->HandleMouse(bLMBDown, cursor.x - rcClient.left, cursor.y - rcClient.top);
			}
		}
		else
			g_gui->HandleMouse(bLMBDown, cursor.x - rcClient.left, cursor.y - rcClient.top);
		// 获取信息
		surfer.GetCurInfo(&cursor, &rcClient);
		//pD3DWnd->ChangeVBColor_tail(&colorblock
		//	, surfer.bCursorOnPic ? SETALPHA(surfer.cursorColor, COLOR_BLOCKALPHA) : BackgroundColor
		//	, sizeof(FVF2));

		lastCursor = cursor;//保存鼠标位置
		break;
	//	WM_MOUSEWHEEL	- 鼠标滚轮，需要高效
	case WM_MOUSEWHEEL:
		if (bPicOn)
		{
			//缩放
			POINT base;
			base.x = cursor.x - rcClient.left;
			base.y = cursor.y - rcClient.top;
			surfer.SurfAdjustZoom_WheelPR((short)HIWORD(wParam));
			bSurfRenew = surfer.SurfZoomRenew(&base, ISALTDOWN, !ISKEYDOWN('V'));

			bNeedForceRenew = true;

			// 获取信息
			surfer.GetCurInfo(&cursor, &rcClient);
			//pD3DWnd->ChangeVBColor_tail(&colorblock
			//	, surfer.bCursorOnPic ? SETALPHA(surfer.cursorColor, COLOR_BLOCKALPHA) : BackgroundColor
			//	, sizeof(FVF2));

			// 标志
			if(bSurfRenew)
				surfRenewTick = GetTickCount();
			bOnZoom = true;
			zoomTick = GetTickCount();
		}
		break;
	//	WM_SIZE	- 窗口拉伸，需要高效
	case WM_SIZE:
		bIconic = IsIconic(hWndMain) > 0;
		if (!bIconic)
		{
			GetCursorPos(&cursor);//size时不进入主循环，需获取鼠标位置
			OnWinChange();//更新窗口区域。如果使用size结束重置设备，这句可不执行。TODO：设置另一窗口区域信息实时更新
			MaintainWindowStyle();//维持窗口样式

			//如果使用size结束重置设备，ResetDevice和surface更新都不执行
			// 重置设备
			ResetDevice();

			// surface更新
			bSurfRenew = surfer.OnWinsize_Custom();
			bNeedForceRenew |= bSurfRenew;

			// 标志
			if (bSurfRenew)
				surfRenewTick = GetTickCount();
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
		GetCursorPos(&cursor);//WM_MOVE时不进入主循环，需要更新鼠标位置
		if (!bOnSize)
		{
			//更新窗口区域
			OnWinChange();//size时不做,否则是重复执行
		}
		break;
	//	WM_SETCURSOR	- 设置鼠标样式
	case WM_SETCURSOR:
		//不同窗口模式，不同处理
		if (winMode == WINMODE_ROUND)
		{
			if (bWindowedFullscreen)
				::SetCursor(LoadCursor(NULL, IDC_ARROW));
			else
				SetCursor_Custom();
		}
		else
			DefWindowProc(hWnd, message, wParam, lParam);
		break;
	//	WM_LBUTTONDBLCLK	- 鼠标左键双击
	case WM_LBUTTONDBLCLK:
		if (bWindowedFullscreen)
		{
			FullScreen_Windowed(!bWindowedFullscreen);
		}
		else
		{
			PostMessage(hWndMain, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		}
		break;
	//	WM_LBUTTONDOWN	- 鼠标左键按下
	case WM_LBUTTONDOWN:
		bLMBDown = true;
		
		//自定义客户区内拉伸窗口
		if (ISCONTROLDOWN)
		{
			if (bWindowedFullscreen)
				FullScreen_Windowed(!bWindowedFullscreen, false);//退出全屏
			
			BeginDragWindow_Inner(cursor.x, cursor.y);
			break;
		}
		//无边框窗口拉伸
		else if (bSizeEnable && winMode == WINMODE_ROUND
			&& GetSizeType(cursor) != 0 && !bWindowedFullscreen)
		{
			BeginDragWindow_Custom(cursor.x, cursor.y);
			break;
		}
		//拖动图片
		else
		{
			if (bPicOn)
			{
				BeginDragPic();//开始拖动
			}
		}

		g_gui->HandleMouse(bLMBDown, cursor.x - rcClient.left, cursor.y - rcClient.top);

		break;
	//	WM_LBUTTONUP	- 鼠标左键抬起
	case WM_LBUTTONUP:
		//ResetDevice();
		bLMBDown = false;
		
		if (bOnDrag)
		{
			EndDragPic();
		}

		ReleaseCapture();
		g_gui->HandleMouse(bLMBDown, cursor.x - rcClient.left, cursor.y - rcClient.top);

		break;
	//	WM_RBUTTONDOWN	- 鼠标右键按下
	case WM_RBUTTONDOWN:
		if (bPicOn)
		{
			BeginDragZoomPic();//开始拖动
		}
		break;
	//	WM_RBUTTONDOWN	- 鼠标右键抬起
	case WM_RBUTTONUP:
		if (bOnDragzoom)
		{
			EndDragZoomPic();
		}
		break;
	case WM_NCLBUTTONDOWN:
		DefWindowProc(hWnd, message, wParam, lParam); 
		//防止发送NCLBUTTONDOWN后鼠标抬起不响应
		PostMessage(hWndMain, WM_LBUTTONUP, GetSizeType(cursor), MAKELPARAM(cursor.x, cursor.y));
		break;
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
		g_gui->HandleKeyboard(GUI_KEYEVENT_KEYDOWN, wParam);
		break;
	//	WM_RESETDEVICE	- 处理设备丢失
	case WM_RESETDEVICE:
		ResetDevice();
		break;
	//	WM_SURFFORCERENEW	- 强制刷新surface（用合适的生成方法）
	case WM_SURFFORCERENEW:
		if (!bOnDrag && !bOnZoom && !bOnDragzoom && !bOnSize)
		{
			SF_SR(surfer.SurfRenew(false));
			bNeedForceRenew = false;//清除需要强制刷新的标志
			
			surfer.GetCurInfo(&cursor, &rcClient);//获取信息
			//pD3DWnd->ChangeVBColor_tail(&colorblock
			//	, SETALPHA(surfer.cursorColor, COLOR_BLOCKALPHA), sizeof(FVF2));//改变colorblock模型颜色

		}
		break;
	case WM_MOUSEWHEELEND:
		if (bNeedForceRenew/* && !bOnDrag && !bOnZoom && !bOnDragzoom && !bOnSize*/)
			PostMessage(hWndMain, WM_SURFFORCERENEW, 0, 1);
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
				PostMessage(hWndMain, WM_SURFFORCERENEW, 0, 1);

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
		g_gui->HandleKeyboard(GUI_KEYEVENT_IMECHAR, wParam);
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
		pnccsp->rgrc[0].top = rcWnd.top + 8;
		pnccsp->rgrc[0].bottom = pnccsp->rgrc[0].top + HEIGHTOF(rcWnd)-16;
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

short MYCALL1 GetSizeType(CPoint point)
{
	short sizetype = HTNOWHERE;

	if (point.x > rcClient.left + SIZE_NEAR_PIXEL && point.x < rcClient.right - SIZE_NEAR_PIXEL
		&& point.y > rcWnd.top + 12 && point.y < rcClient.bottom - SIZE_NEAR_PIXEL)
		sizetype = HTNOWHERE;
	else if (point.x <= rcClient.left + SIZE_NEAR_PIXEL && point.x >= rcClient.left - SIZE_NEAR_PIXEL
		&& point.y <= rcWnd.top + 12 && point.y >= rcWnd.top - SIZE_NEAR_PIXEL)
		sizetype = HTTOPLEFT;
	else if (point.x <= rcClient.right + SIZE_NEAR_PIXEL && point.x >= rcClient.right - SIZE_NEAR_PIXEL
		&& point.y <= rcClient.bottom + SIZE_NEAR_PIXEL && point.y >= rcClient.bottom - SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOMRIGHT;
	else if (point.x <= rcClient.right + SIZE_NEAR_PIXEL && point.x >= rcClient.right - SIZE_NEAR_PIXEL
		&& point.y <= rcWnd.top + 12 && point.y >= rcWnd.top - SIZE_NEAR_PIXEL)
		sizetype = HTTOPRIGHT;
	else if (point.x <= rcClient.left + SIZE_NEAR_PIXEL && point.x >= rcClient.left - SIZE_NEAR_PIXEL
		&& point.y <= rcClient.bottom + SIZE_NEAR_PIXEL && point.y >= rcClient.bottom - SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOMLEFT;
	else if (point.x <= rcClient.left + SIZE_NEAR_PIXEL && point.x >= rcClient.left - SIZE_NEAR_PIXEL)
		sizetype = HTLEFT;
	else if (point.x <= rcClient.right + SIZE_NEAR_PIXEL && point.x >= rcClient.right - SIZE_NEAR_PIXEL)
		sizetype = HTRIGHT;
	else if (point.y <= rcWnd.top + 12 + SIZE_NEAR_PIXEL && point.y >= rcWnd.top + 12 - SIZE_NEAR_PIXEL)
		sizetype = HTTOP;
	else if (point.y <= rcClient.bottom + SIZE_NEAR_PIXEL && point.y >= rcClient.bottom - SIZE_NEAR_PIXEL)
		sizetype = HTBOTTOM;
	else
		sizetype = HTNOWHERE;

	return sizetype;
}

inline void MYCALL1 SetCursor_Custom()
{
	if (bSizeEnable && winMode == WINMODE_ROUND)
	{
		switch (GetSizeType(cursor))
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
	GetClientRect(hWndMain, &rcClient);//得到client区域尺寸
	POINT clienttl = { 0, 0 };
	ClientToScreen(hWndMain, &clienttl);//获得client区域左上角的屏幕坐标
	//得到client真实屏幕区域
	rcClient.left = clienttl.x;
	rcClient.top = clienttl.y;
	rcClient.right += clienttl.x;
	rcClient.bottom += clienttl.y;

	//得到窗口区域
	GetWindowRect(hWndMain, &rcWnd);
	//客户区和窗口的长宽差
	WinDiffW = WIDTHOF(rcWnd) - WIDTHOF(rcClient);
	WinDiffH = HEIGHTOF(rcWnd) - HEIGHTOF(rcClient);

	//更新文字信息显示区域
	RefreshTextRect();
}

inline void MYCALL1 RefreshTextRect()
{
	// 状态信息显示区域
	rcFlag.left = TEXTMARGIN_SIDE;
	rcFlag.top = HEIGHTOF(rcClient) - TEXTMARGIN_BOTTOM - 16;
	rcFlag.right = rcFlag.left + 600;
	rcFlag.bottom = rcFlag.top + 16;

	// 图片状态显示区域
	rcPicState.right = WIDTHOF(rcClient) - 10;
	rcPicState.top = TEXTMARGIN_TOP;
	rcPicState.left = rcPicState.right - 100;
	rcPicState.bottom = rcPicState.top + 60;
}

bool MYCALL1 Init()
{
	//窗口信息&状态
	OnWinChange();

	GetCursorPos(&cursor);
	lastCursor = cursor;

	WinStdW = WINDOWWIDTH_INIT;
	WinStdH = WINDOWHEIGHT_INIT;

	hProcess = GetCurrentProcess();
	//状态标志
	bPureWnd = false;
	bWindowedFullscreen = false;
	bIconic = false;
	bOnDrag = false;
	bOnDragzoom = false;
	bDragging = false;
	bDragzooming = false;
	bOnZoom = false;
	bOnSize = false;
	bSurfRenew = false;
	bNeedForceRenew = false;
	//选项标志
	mode = MODE_PIC;
	SetWindowMode(WINMODE_INIT);//WINMODE_NORMALWINDOWS
	bColorblockOn = false;
	BackgroundColor = COLOR_BKG_INIT;
	bSizeEnable = true;
	bFlagsShow = true;
	bInfoShow = true;
	bFpsLimited = true;
	bScreenColorOn = false;
	screenColor = 0;

	//计时
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

	//D3D
	mainDevice = NULL;

	//信息显示
	rcPic = RECT(TEXTMARGIN_SIDE, TEXTMARGIN_TOP + 60, 400, 100);
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
	surfer.SetBackcolor(BackgroundColor);

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
	picInfoStr[0] = L'\0';
	
	// SPRITE
	/*D3DXCreateSprite(mainDevice, &m_sprite);
	D3DXMATRIX matTransform;
	D3DXMatrixIdentity(&matTransform);
	m_sprite->SetTransform(&matTransform);*/

	// OBJECT
	//pD3DWnd->CreateMesh_Custom1(&decorate);//自创模型并保存
	D3DXLoadMeshFromXW(L"crystal.x", D3DXMESH_MANAGED, mainDevice, NULL, NULL, NULL, NULL, &decorate);
	D3DXMatrixIdentity(&matWorld);
	D3DXMatrixRotationX(&matWorld, PI_F / 2);

	/*pD3DWnd->CreateVertexBuffer_Custom1(&colorblock, COLORBLOCK_X, COLORBLOCK_Y, COLORBLOCK_RADIUS);
	pD3DWnd->CreateVertexBuffer_Custom1(&colorblockback
		, COLORBLOCK_X, COLORBLOCK_Y, COLORBLOCK_RADIUS + 2, COLOR_BLOCKBACK);*/

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

	// RENDERSTATE
	SetRenderState();

	// GUI
	g_gui = new CD3DGUISystem(mainDevice);
	g_gui->Bind(pD3DWnd);
	g_gui->SetCallbackEvent(GUICallback);
	g_gui->AddDXFont(L"Consolas", &fontID1, 15, 0, 0, CLEARTYPE_NATURAL_QUALITY);//PixelSix10,20,10

	float bX = 6, bY = 6, bW = 58, bH = 20;
	float bVM = 23;
	g_gui->AddButton(BUTTON_ID_1, bX, bY, bW, bH, L"file");
	bY += bVM;
	g_gui->AddButton(BUTTON_ID_2, bX, bY, bW, bH, L"save");
	g_gui->AddButton(BUTTON_ID_3, bX, 50, bW, bH, L"full", 0xEEAAEEFF, 0, GUI_WINDOCK_RIGHT);
	g_gui->AddEdit(INPUT_IN_1, 0, 18.0f, 0, 20.0f, CMDRECT_COLOR_USING, COLOR_CMD_INIT, fontID1, GUI_WINDOCK_BOTTOMHSPAN);
	g_gui->HideControl(INPUT_IN_1);
	g_gui->AddBackdrop(L"test.jpg", 0.1f, 0.3f, 0.2f, 0.2f, GUI_WINDOCK_SCALE);

	return true;
}

bool MYCALL1 LoadFile(WCHAR file[])
{
	if (!mainDevice)
	{
		return false;
	}

	PicPack *newpp = new PicPack;
	HRESULT hr = newpp->LoadFile(mainDevice, file);//尝试加载新图片
	if (SUCCEEDED(hr))
	{
		// 图片列表扩充 & 更新当前图片
		picList.Add(newpp);
		SetNewPic();

		return true;
	}
	else
	{
		delete newpp;//释放

		SetForegroundWindow(hWndMain);
		D3DErrorShow(hr, L"Load File", hWndMain);// 显示错误信息

		return false;
	}
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

bool MYCALL1 OnWininitFile(LPWSTR cmdline)
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
	ReleaseCapture();//统一释放
	if (bLMBDown)
	{
		bLMBDown = false;
	}
	if (bOnDrag)
	{
		bOnDrag = false;
	}
	if (bOnDragzoom)
	{
		bOnDragzoom = false;
	}
	bDragzooming = false;
	bDragging = false;
	bOnZoom = false;
	bOnSize = false;
	surfRenewTick = false;
	bNeedForceRenew = false;
}

void InitNonPic()
{
	pLivePicpack = NULL;
	pLiveBmp = NULL;

	bPicOn = HasPic();
}

bool SetPic(short picidx)
{
	PicPack *pOldPicpack = pLivePicpack;
	picList.SetPicPack(picidx);

	// 更新本地
	pLivePicpack = picList.pLivePicPack;
	pLiveBmp = picList.pLiveBMP;

	SetSurface(pLivePicpack, pOldPicpack);
	SetPicInfo();
	bPicOn = HasPic();

	return true;
}

bool SetNewPic()
{
	PicPack *oldpicpack = pLivePicpack;
	picList.SetTailPicPack();

	// 更新本地
	pLivePicpack = picList.pLivePicPack;
	pLiveBmp = picList.pLiveBMP;

	SetSurface(pLivePicpack, oldpicpack, false);

	// 居中
	surfer.SurfCenterPR(*pBufferW, *pBufferH);
	bSurfRenew = surfer.SurfRenew(false);
	if (bSurfRenew)
		surfRenewTick = GetTickCount();

	SetPicInfo();
	bPicOn = HasPic();

	return true;
}

bool SetTailPic()
{
	PicPack *oldpicpack = pLivePicpack;
	picList.SetTailPicPack();

	// 更新本地
	pLivePicpack = picList.pLivePicPack;
	pLiveBmp = picList.pLiveBMP;

	SetSurface(pLivePicpack, oldpicpack);
	SetPicInfo();
	bPicOn = HasPic();

	return true;
}

bool SetPrevPic()
{
	PicPack *oldpicpack = pLivePicpack;
	picList.SetPrev();

	// 更新本地
	pLivePicpack = picList.pLivePicPack;
	pLiveBmp = picList.pLiveBMP;

	SetSurface(pLivePicpack, oldpicpack);
	SetPicInfo();
	bPicOn = HasPic();

	return true;
}

bool SetNextPic()
{
	PicPack *oldpicpack = pLivePicpack;
	picList.SetNext();

	// 更新本地
	pLivePicpack = picList.pLivePicPack;
	pLiveBmp = picList.pLiveBMP;

	SetSurface(pLivePicpack, oldpicpack);
	SetPicInfo();
	bPicOn = HasPic();

	return true;
}

void Drop()
{
	picList.Drop();

	// 更新本地
	pLivePicpack = picList.pLivePicPack;
	pLiveBmp = picList.pLiveBMP;

	SetSurface(pLivePicpack, NULL);// 不解绑
	SetPicInfo();
	bPicOn = HasPic();
}

void SetSurface(PicPack *newpicpack, PicPack *oldpicpack, bool renew)
{
	if (oldpicpack)
		surfer.DeBindPic(oldpicpack);// 解绑
	surfer.BindPic(newpicpack, renew);// 捆绑

	//标志
	bSurfRenew = true;//TODO强制设置为已更新？
	surfRenewTick = GetTickCount();
}

void SetPicInfo()
{
	if (pLivePicpack)
	{
		wcscpy_s(picInfoStr, pLivePicpack->GetPicInfoStrW());// 图片信息字符串更新
		SetWindowTextW(hWndMain, pLivePicpack->GetFileName());// 窗口标题
	}
	else
	{
		picInfoStr[0] = L'\0';
		SetWindowTextW(hWndMain, L"");
	}
	surfer.GetCurInfo(&cursor, &rcClient);//获取信息
}

void PicMove(int dx, int dy)
{
	SF_SRFR(surfer.OnMove_Custom({ dx, dy }));

	surfer.GetCurInfo(&cursor, &rcClient);
}

void PicRestore()
{
	POINT base;
	base.x = cursor.x - rcClient.left;
	base.y = cursor.y - rcClient.top;

	surfer.SurfSetZoomPR(1);
	SF_SR(surfer.SurfZoomRenew(&base, true, false));
	SF_OZ();

	surfer.GetCurInfo(&cursor, &rcClient);
}

void PicClipWindow()
{
	SF_SRFR(surfer.OnMove_Custom(surfer.GetBase()));

	rcWnd.right = rcWnd.left + surfer.GetZoomWidth() + WinDiffW;
	rcWnd.bottom = rcWnd.top + surfer.GetZoomHeight() + WinDiffH;
	SetWindowPos(hWndMain, HWND_TOP, rcWnd.left, rcWnd.top
		, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top
		, 0);

	surfer.GetCurInfo(&cursor, &rcClient);
}

void BeginDragWindow_Inner(int cursorx, int cursory)
{
	if (cursorx < MIDDLEXOF(rcClient) + WIDTHOF(rcClient) / 5
		&& cursorx > MIDDLEXOF(rcClient) - WIDTHOF(rcClient) / 5
		&& cursory < MIDDLEYOF(rcClient) + HEIGHTOF(rcClient) / 5
		&& cursory > MIDDLEYOF(rcClient) - HEIGHTOF(rcClient) / 5)
	{
		PostMessage(hWndMain, WM_NCLBUTTONDOWN, HTCAPTION, 0);
	}
	else
	{
		if (cursory < MIDDLEYOF(rcClient))
		{
			if (cursorx < MIDDLEXOF(rcClient))
				PostMessage(hWndMain, WM_NCLBUTTONDOWN, HTTOPLEFT, 0);
			else
				PostMessage(hWndMain, WM_NCLBUTTONDOWN, HTTOPRIGHT, 0);
		}
		else
		{
			if (cursorx < MIDDLEXOF(rcClient))
				PostMessage(hWndMain, WM_NCLBUTTONDOWN, HTBOTTOMLEFT, 0);
			else
				PostMessage(hWndMain, WM_NCLBUTTONDOWN, HTBOTTOMRIGHT, 0);
		}
	}
}

void BeginDragWindow_Custom(int cursorx, int cursory)
{
	PostMessage(hWndMain, WM_NCLBUTTONDOWN, GetSizeType(cursor)
		, MAKELPARAM(cursorx, cursory));

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
			//multisample导致图片不显示
				pD3DWnd->ChangeMultiSample();
				ResetDevice();
				break;
			case 'S':
				OnSave();
				break;
			case 'W':	// 改变窗口模式
				if (winMode == WINMODE_ROUND)
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
			case 'F':	// 设置标准窗口尺寸
				if (bPicOn)
					PicFitWnd();
				break;
			case 'I':
				bColorblockOn = !bColorblockOn;
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
					if (!bOnDragzoom && !bOnZoom)//drazoom时按v在mousemove中刷新surface
					{
						SF_SR(surfer.SurfRenew(false));

						surfer.GetCurInfo(&cursor, &rcClient);
					}
				}
				break;
			case 'W':	// 改变窗口模式
			case VK_UP:
				if (bPicOn)
					PicMove(0, -1);
				break;
			case 'X':	// 重置收敛帧率值（重新开始计算）
				fpsCount = 0;
				cvgFps = -1;
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
				break;
			case VK_OEM_PLUS:
			case VK_TAB:
				SetNextPic();
				break;
			case VK_DELETE:
				Drop();
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

void SaveFileWin(WCHAR file[])
{
	OPENFILENAME svfn;
	WCHAR savefilename[MAX_PATH] = { 0 };//存放文件名 
	if(file)
		wcscpy_s(savefilename, file);

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
	svfn.Flags = OFN_OVERWRITEPROMPT ;//标志位:覆盖提醒   OFN_SHOWHELP？
	svfn.hwndOwner = hWndMain;//模态
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
		//已设置OFN_OVERWRITEPROMPT！
		//if (_waccess(savefilename, 0) == 0)//判断文件存在
		//{
		//	if (IDYES == MessageBoxW(hWndMain, L"是否覆盖？", L"文件已存在", MB_YESNO | MB_APPLMODAL))
		//		OnSaveFile(savefilename);
		//	else
		//		;
		//}
		//else
		SaveFile(savefilename);
	}
}

bool SaveFile(WCHAR file[])
{
	if (!pLivePicpack)
	{
		return false;
	}

	if (!pLivePicpack->SaveFile(mainDevice, file))
	{
		MessageBoxW(hWndMain, L"Save File FAILED!", L"ERROR", MB_OK | MB_APPLMODAL);
		return false;
	}
	else
	{
		SetWindowTextW(hWndMain, file);//窗口标题

		wcscpy_s(picInfoStr, pLivePicpack->GetPicInfoStrW());

		return true;
	}
}

void SetWindowMode(UINT8 wmode)
{
	//设置矩形区域
	winMode = wmode;
	if (winMode == WINMODE_ROUND)
	{
		HRGN hrgn;

		RECT rgnrect;
		rgnrect.left = rcClient.left - rcWnd.left;
		rgnrect.top = 8;
		rgnrect.right = WIDTHOF(rcWnd) - rgnrect.left + 1/*WIDTHOF(rcClient) + 9*/;
		rgnrect.bottom = HEIGHTOF(rcWnd) - rgnrect.top + 1/*HEIGHTOF(rcClient) + 22*/;
		hrgn = CreateRoundRectRgn(rgnrect.left, rgnrect.top
			, rgnrect.right, rgnrect.bottom, 2, 2);//尺寸，要+1
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
		rgnrect.left = rcClient.left - rcWnd.left;
		rgnrect.top = 8;
		rgnrect.right = rcWnd.right - rcClient.left + 1;//rcWnd.right - rcClient.left + 1
		rgnrect.bottom = HEIGHTOF(rcWnd) - rgnrect.top + 1;//HEIGHTOF(rcWnd) - rgnrect.top + 1
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

	lastCursor = cursor;

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

	surfer.SetBasePoint({ cursor.x - rcClient.left,cursor.y - rcClient.top });

	lastCursor = cursor;

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

inline void MYCALL1 EnterCMDMode()
{
	mode = MODE_CMD;
	g_gui->ShowControl(INPUT_IN_1);
	g_gui->SetFocus(INPUT_IN_1);
}

inline void MYCALL1 ExitCMDMode()
{
	mode = MODE_PIC;
	g_gui->SetControlText(INPUT_IN_1, L"");
	g_gui->HideControl(INPUT_IN_1);
}

void MYCALL1 PicFitWnd()
{
	SF_SRFR(surfer.OnMove_Custom(surfer.GetBase()));

	rcWnd.right = rcWnd.left + WinStdW + WinDiffW;
	rcWnd.bottom = rcWnd.top + WinStdW * pLiveBmp->width / pLiveBmp->height + WinDiffH;
	if (HEIGHTOF(rcWnd) > WinStdH)
	{
		rcWnd.bottom = rcWnd.top + WinStdH + WinDiffH;
		rcWnd.right = rcWnd.left + WinStdH * pLiveBmp->width / pLiveBmp->height + WinDiffW;
	}

	SetWindowPos(hWndMain, HWND_TOP, rcWnd.left, rcWnd.top
		, WIDTHOF(rcWnd), HEIGHTOF(rcWnd)
		, 0);

	surfer.GetCurInfo(&cursor, &rcClient);
}

void PicDock()
{
	SF_SRFR(surfer.OnMove_Custom(surfer.GetBase()));

	surfer.GetCurInfo(&cursor, &rcClient);
}

void PicCenter()
{
	surfer.SurfCenterPR(*pBufferW, *pBufferH);
	SF_SR(surfer.SurfRenew(false));

	surfer.GetCurInfo(&cursor, &rcClient);
}

void PicFit()
{
	SF_SR(surfer.SurfSuit(*pBufferW, *pBufferH));

	surfer.GetCurInfo(&cursor, &rcClient);
}

bool FullScreen_Windowed(bool tofull, bool restore)
{
	if (bWindowedFullscreen == tofull)
		return false;
	bWindowedFullscreen = tofull;
	if (bWindowedFullscreen)// 全屏化
	{
		rcOriginalWnd = rcWnd;
		RECT fullScreenRect;
		fullScreenRect.left = rcWnd.left - rcClient.left;
		fullScreenRect.top = rcWnd.top - rcClient.top + 22;
		if (bPureWnd)
			fullScreenRect.top -= 22;
		fullScreenRect.right = rcWnd.right
			- rcClient.right + GetSystemMetrics(SM_CXSCREEN) + 12;
		fullScreenRect.bottom = rcWnd.bottom
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

		g_gui->SetControlText(BUTTON_ID_3, L"small");
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
		g_gui->SetControlText(BUTTON_ID_3, L"full");
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
	SetWindowPos(hWndMain, HWND_TOP, rcWnd.left, rcWnd.top
		, WIDTHOF(rcWnd), HEIGHTOF(rcWnd)
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
	// 标配：D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA
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

	mainDevice->SetRenderState(D3DRS_LIGHTING, TRUE); 
	mainDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	mainDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	//mainDevice->SetRenderState(D3DRS_AMBIENT,
	//	D3DCOLOR_COLORVALUE(0.3f, 0.3f, 0.3f, 1.0f));

	mainDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);//不需要
	mainDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);

	mainDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
	//mainDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	mainDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

inline void SetView()
{
	// mesh方法：动态调整视角
	if (decorate)
	{
		D3DXVECTOR3 eye;
		D3DXVECTOR3 at;
		float eyeradius = 40.0f;
		eye = D3DXVECTOR3(0.0f, -eyeradius, 0.0f);
		at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DXMATRIXA16 matView;
		D3DXMatrixLookAtLH(&matView, &eye, &at, &D3DXVECTOR3(0.0f, 0.0f, -1.0f));
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
		D3DXMatrixRotationZ(&rotnew, angle);
		D3DXMatrixMultiply(&matWorld, &matWorld, &rotnew);
		mainDevice->SetTransform(D3DTS_WORLD, &matWorld);
	}
}

inline void SetLight()
{
	if (decorate)
	{
		light.Type = D3DLIGHT_POINT;
		light.Position = D3DXVECTOR3(-12.0f, -20.0f, -4.0f);
		light.Range = 30.0f;
		light.Diffuse.r = 1.0f;
		light.Diffuse.g = 1.0f;
		light.Diffuse.b = 1.0f;
		light.Ambient.r = 0.6f;
		light.Ambient.g = 0.6f;
		light.Ambient.b = 0.6f;
		light.Attenuation0 = 0.5f;
		light.Attenuation1 = 0.04f;
		mainDevice->SetLight(0, &light);
		mainDevice->LightEnable(0, TRUE);
	}
}

bool InfoRender()
{
	WCHAR infowstr[512] = { 0 };

	// 图片状态信息显示
	swprintf_s(infowstr, L"%d/%d", picList.GetCurPos(), picList.GetCount());
	pFontPicState->DrawTextW(NULL, infowstr, -1, &rcPicState, DT_RIGHT | DT_NOCLIP, COLOR_TEXT4);

	// 第1部分信息显示（图片信息）
	// d3dfont1->DrawTextW(100, 200, COLOR_TEXT1, infowstr0, 0);//测试高速字体绘制
	pFontPic->DrawTextW(NULL, picInfoStr, -1, &rcPic, DT_LEFT | DT_NOCLIP, COLOR_TEXT1);

	// 窗口信息 & surfer信息
	swprintf_s(infowstr, L"MODE: %d\n\
		FPS: %.2f/%.2f (%.2f)  %.3fms   %lld\n\
		MEM: %.1fMB,  %.1fMB\n\
		BUF: %d × %d\n\
		CLIENT: %d, %d\n\
		PROC: %.3fms\n"
		, mode
		, fps, avgFps, cvgFps, frameTime, nLoops
		, memoryIn, memoryOut
		, *pBufferW, *pBufferH
		, cursor.x - rcClient.left, cursor.y - rcClient.top
		, procTime);
	wcscat_s(infowstr, L"   -    -    -    -   \n");
	wcscat_s(infowstr, surfer.GetInfoStr());
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
		swprintf_s(infowstr,
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
			, yesno2[surfer.bSurfClipped], yesno2[surfer.bPicClipped]
			, yesno2[surfer.bOutClient], yesno2[bSurfRenew]);

		// 第3部分信息显示
		pFontFlags->DrawTextW(NULL, infowstr, -1, &rcFlag
			, DT_LEFT | DT_TOP | DT_NOCLIP, COLOR_TEXT3);
	}

	return true;
}

void MYCALL1 Render()
{
	static HRESULT hr;

	mainDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, BackgroundColor, 1.0f, 0);
	mainDevice->BeginScene();

	g_gui->RenderBack();

	// 装饰绘制
	if (decorate)
	{
		SetView();
		mainDevice->SetMaterial(&material);
		mainDevice->SetTexture(NULL, NULL);
		mainDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		mainDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		decorate->DrawSubset(0);
	}
	mainDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	mainDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	// 图片绘制
	hr = surfer.Render();

	if (bInfoShow)//信息显示
		InfoRender();

	// D3DGUI
	g_gui->Render();//D3DBLEND_INVSRCCOLOR,D3DBLEND_INVSRCCOLOR;D3DBLEND_ZERO,D3DBLEND_INVDESTCOLOR

	// 颜色块
	/*if (colorblockon && colorblock)
	{
		mainDevice->SetTexture(NULL, NULL);

		mainDevice->SetStreamSource(0, colorblockback, 0, sizeof(FVF2));
		mainDevice->SetFVF(FVF_2CPD);
		mainDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 20);
		mainDevice->SetStreamSource(0, colorblock, 0, sizeof(FVF2));
		mainDevice->SetFVF(FVF_2CPD);
		mainDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 20);
	}*/

	mainDevice->EndScene();
	hr = mainDevice->Present(NULL, NULL, NULL, NULL);

	// 处理设备丢失
	if (hr == D3DERR_DEVICELOST)
	{
		if (mainDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			PostMessage(hWndMain, WM_RESETDEVICE, 0, 0);
		}
	}
}

bool ResetDevice()
{
	if (!mainDevice)
		return false; 

	// 修改device长宽
	OnLostDevice();
	bool result = OnResetDevice2(WIDTHOF(rcClient), HEIGHTOF(rcClient));
	//bool result = OnResetDevice();

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
	// D3D更新,设备丢失后必做！yeah
	SetLight();
	SetView();
	SetRenderState();
}

void CALLBACK GUICallback(int ID, WPARAM wp, LPARAM lp)
{
	switch (ID)
	{
	case BUTTON_ID_1:
		if (wp == GUI_EVENT_UP)
		{
			PostMessage(hWndMain, WM_COMMAND, IDM_OPEN, 0);
		}
		break;
	case BUTTON_ID_2:
		if (wp == GUI_EVENT_UP)
		{
			PostMessage(hWndMain, WM_COMMAND, IDM_SAVE, 0);
		}
		break;
	case BUTTON_ID_3:
		if (wp == GUI_EVENT_UP)
		{
			PostMessage(hWndMain, WM_TOGGLEFULLSCREEN, 0, 1);
		}
		break;
	case INPUT_IN_1:
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
	else if (_wcsicmp(wstr, L"maxwin") == 0)
	{
		SendMessage(hWndMain, WM_SYSCOMMAND, SC_MAXIMIZE, 0);//SC_RESTORE
	}
	else if (_wcsicmp(wstr, L"rstwin") == 0)
	{
		SendMessage(hWndMain, WM_SYSCOMMAND, SC_RESTORE, 0);
	}
	else if (_wcsicmp(wstr, L"minwin") == 0)
	{
		SendMessage(hWndMain, WM_SYSCOMMAND, SC_MINIMIZE, 0);
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
	else if (_wcsicmp(wstr, L"inva") == 0)
	{
		if (bPicOn)
		{
			pLiveBmp->InverseAlpha();
			bSurfRenew = surfer.SurfRenew(false);

			if (bSurfRenew)
				surfRenewTick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"inv4") == 0)
	{
		if (bPicOn)
		{
			pLiveBmp->InverseAll();
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
				POINT base;
				base.x = cursor.x - rcClient.left;
				base.y = cursor.y - rcClient.top;
				surfer.SurfSetZoomPR(num);
				bSurfRenew = surfer.SurfZoomRenew(&base, true, false);//放大，调整surface位置
				
				//标志
				if (bSurfRenew)
					surfRenewTick = GetTickCount();
				bOnZoom = true;
				zoomTick = GetTickCount();
			}
		}
	}
	//获取surface信息（图片更新后，surface信息也立即更新）
	surfer.GetCurInfo(&cursor, &rcClient);

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
