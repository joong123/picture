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
	pd3dwnd = new D3DWnd();
	if (!pd3dwnd->D3DCreateWindow(szTitle, WndProc, hInstance
		, CS_HREDRAW | CS_VREDRAW/* | CS_DROPSHADOW*/
		, WS_OVERLAPPEDWINDOW, WS_EX_ACCEPTFILES
		, WINDOWPOSX_INIT, WINDOWPOSY_INIT, WINDOWWIDTH_INIT, WINDOWHEIGHT_INIT
		, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PICTURE))
		, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL))
		, /*MAKEINTRESOURCEW(IDC_PICTURE)*/0
		, szWindowClass, COLOR_BKG_INIT))
		return FALSE;

	mainwnd = pd3dwnd->GetHWND();//存储主窗口句柄

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
		if (isiconic)//ERROR 有时退出时会崩溃，指向这行！
		{
			Sleep(ICONIC_SLEEP);//最小化状态降低资源消耗
			continue;
		}

		// 帧率限制
		QueryPerformanceCounter(&etime);
		float testfps;
		if (fpslimit)
		{
			if (etime.QuadPart != stime.QuadPart)
				testfps = (float)frequency.QuadPart / (etime.QuadPart - stime.QuadPart);
			if (dragging || onzoom || draggingzoom)//图片需快速更新
			{
				short ftdiff = (short)(normalframetime - 1000.0f / testfps);
				if (/*testfps > normalfps*///可能莫名奇妙降低帧率
					ftdiff >= 1)
				{
					Sleep(1);
					continue;
				}
			}
			else//静止状态，低帧率刷新界面
			{
				if (testfps > staticfps*1.05f)// *1.05控制帧率准确度
				{
					Sleep(1);
					continue;
				}
			}
		}

		// 计数
		loopcount++;
		time(&nowtime);

		// 更新延迟的标志
		DelayFlag();

		//动态调整帧率限制
		/*if (fpslimit)
		{
			if (dragging || onzoom || draggingzoom)
			{
				if (testfps < FPS_ERROR_DOWN)
				{
					normalfps += 0.2f;
					normalframetime = 1000.0f / normalfps;
				}
				else if (testfps > FPS_ERROR_UP)
				{
					if (normalfps > 0)
						normalfps -= 0.2f;
					normalframetime = 1000.0f / normalfps;
				}
				else
				{
					if (normalfps > NORMAL_FPS)
						normalfps -= 0.2f;
					normalframetime = 1000.0f / normalfps;
				}
			}
		}*/

		// 每秒执行(帧率等)
		if (nowtime != lasttime)
		{
			lasttime = nowtime;

			// 计算内存占用 
			PROCESS_MEMORY_COUNTERS pmc;
			GetProcessMemoryInfo(hprocess, &pmc, sizeof(pmc));
			memoryin = pmc.WorkingSetSize / B_TO_MB;//内存占用量
			memoryout = pmc.PagefileUsage / B_TO_MB;//虚拟内存占用量

			// 计算fps
			fpscount++;
			if (fpslimit)							//当前帧率
			{
				fps = testfps;
			}
			else
			{
				fps = (float)frequency.QuadPart / (etime.QuadPart - stime.QuadPart);
			}
			
			frametime = 1000.0f / fps;				//帧时间
			if (avgfps < 0)							//平均帧率
				avgfps = fps;
			else
				avgfps = avgfps*0.9f + fps*0.1f;

			if (cvgfps < 0)							//收敛帧率
				cvgfps = fps;
			else
				cvgfps = (cvgfps*fpscount + fps) / (fpscount + 1);
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
//   //mainwnd = hWnd;
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
		rgnrect.right = WIDTHOF(wrect) - rgnrect.left + 1/*WIDTHOF(clientrect) + 9*/;
		rgnrect.bottom = HEIGHTOF(wrect) - rgnrect.top + 1/*HEIGHTOF(clientrect) + 22*/;
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
				if(mainpicpack)
					SaveFileWin(mainpicpack->GetFileName());
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
			if (!isiconic)
			{
				Render();//使得调整窗口尺寸时自动重绘（调整窗口尺寸时不进入主循环）

				//不可以去掉下面3行(尽管没有添加绘图代码)，会导致帧率降低
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);

				// TODO: 在此处添加使用 hdc 的任何绘图代码...
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
		if (haspic)
		{
			if (ondrag)//*****************************************高效*******************************************
			{
				/*mainbmp->TestInc();
				surfer.SurfRenew(false);*/
				SF_SRFR(surfer.OnMove_Custom(MINUSPOINT(cursor, lastcursor)));
				SF_DR();
			}
			else if (ondragzoom)//*****************************************高效********************************
			{
				surfer.SurfAdjustZoom_dragPR(cursor.x - lastcursor.x);//调整放大倍率
				surfrenew = surfer.SurfZoomRenew(NULL, ISALTDOWN, !ISKEYDOWN('V'));

				needforcerenew = true;

				// 标志
				if (surfrenew)
					surfrenewtick = GetTickCount();
				draggingzoom = true;
				dragzoomtick = GetTickCount();
			}
			else
			{
				g_gui->HandleMouse(LMBdown, cursor.x - clientrect.left, cursor.y - clientrect.top);
			}
		}
		else
			g_gui->HandleMouse(LMBdown, cursor.x - clientrect.left, cursor.y - clientrect.top);
		// 获取信息
		surfer.GetCurInfo(&cursor, &clientrect);
		//pd3dwnd->ChangeVBColor_tail(&colorblock
		//	, surfer.pixelcolorvalid ? SETALPHA(surfer.picpixelcolor, COLOR_BLOCKALPHA) : backcolor
		//	, sizeof(FVF2));

		lastcursor = cursor;//保存鼠标位置
		break;
	//	WM_MOUSEWHEEL	- 鼠标滚轮，需要高效
	case WM_MOUSEWHEEL:
		if (haspic)
		{
			//缩放
			POINT base;
			base.x = cursor.x - clientrect.left;
			base.y = cursor.y - clientrect.top;
			surfer.SurfAdjustZoom_wheelPR((short)HIWORD(wParam));
			surfrenew = surfer.SurfZoomRenew(&base, ISALTDOWN, !ISKEYDOWN('V'));

			needforcerenew = true;

			// 获取信息
			surfer.GetCurInfo(&cursor, &clientrect);
			//pd3dwnd->ChangeVBColor_tail(&colorblock
			//	, surfer.pixelcolorvalid ? SETALPHA(surfer.picpixelcolor, COLOR_BLOCKALPHA) : backcolor
			//	, sizeof(FVF2));

			// 标志
			if(surfrenew)
				surfrenewtick = GetTickCount();
			onzoom = true;
			zoomtick = GetTickCount();
		}
		break;
	//	WM_SIZE	- 窗口拉伸，需要高效
	case WM_SIZE:
		isiconic = IsIconic(mainwnd) > 0;
		if (!isiconic)
		{
			GetCursorPos(&cursor);//size时不进入主循环，需获取鼠标位置
			OnWinChange();//更新窗口区域，如果改为size结束动作时做，TODO：设置另一窗口区域信息实时更新
			MaintainWindowStyle();//维持窗口样式

			// 重置设备
			ResetDevice();

			// surface更新
			surfrenew = surfer.OnWinsize_Custom();
			needforcerenew |= surfrenew;

			// 标志
			if (surfrenew)
				surfrenewtick = GetTickCount();
			onsize = true;
			sizetick = GetTickCount();
		}
		else
		{
			;
		}
		break;
	//	WM_MOVE	- 窗口移动，需要高效
	case WM_MOVE:
		GetCursorPos(&cursor);//WM_MOVE时不进入主循环，需要更新鼠标位置
		if (!onsize)
		{
			//更新窗口区域
			OnWinChange();//size时不做,否则重复执行
		}
		break;
	//	WM_SETCURSOR	- 设置鼠标样式
	case WM_SETCURSOR:
		//不同窗口模式，不同处理
		if (winmode == WINMODE_ROUND)
		{
			if (iswindowedfullscreen)
				::SetCursor(LoadCursor(NULL, IDC_ARROW));
			else
				SetCursor_Custom();
		}
		else
			DefWindowProc(hWnd, message, wParam, lParam);
		break;
	//	WM_LBUTTONDBLCLK	- 鼠标左键双击
	case WM_LBUTTONDBLCLK:
		if (iswindowedfullscreen)
		{
			FullScreen_Windowed(!iswindowedfullscreen);
		}
		else
		{
			PostMessage(mainwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		}
		break;
	//	WM_LBUTTONDOWN	- 鼠标左键按下
	case WM_LBUTTONDOWN:
		LMBdown = true;
		
		//自定义客户区内拉伸窗口
		if (ISCONTROLDOWN)
		{
			if (iswindowedfullscreen)
				FullScreen_Windowed(!iswindowedfullscreen, false);//退出全屏
			
			BeginDragWindow_Custom(cursor.x, cursor.y);
			break;
		}
		//无边框窗口拉伸
		else if (sizeEnable && winmode == WINMODE_ROUND
			&& GetSizeType(cursor) != 0 && !iswindowedfullscreen)
		{
			PostMessage(mainwnd, WM_NCLBUTTONDOWN, GetSizeType(cursor), MAKELPARAM(cursor.x, cursor.y));
			break;
		}
		//拖动图片
		else
		{
			if (haspic)
			{
				BeginDragPic();//开始拖动
			}
		}

		g_gui->HandleMouse(LMBdown, cursor.x - clientrect.left, cursor.y - clientrect.top);

		break;
	//	WM_LBUTTONUP	- 鼠标左键抬起
	case WM_LBUTTONUP:
		//ResetDevice();
		LMBdown = false;
		
		if (ondrag)
		{
			EndDragPic();
		}

		ReleaseCapture();
		g_gui->HandleMouse(LMBdown, cursor.x - clientrect.left, cursor.y - clientrect.top);

		break;
	//	WM_RBUTTONDOWN	- 鼠标右键按下
	case WM_RBUTTONDOWN:
		if (haspic)
		{
			BeginDragZoomPic();//开始拖动
		}
		break;
	//	WM_RBUTTONDOWN	- 鼠标右键抬起
	case WM_RBUTTONUP:
		if (ondragzoom)
		{
			EndDragZoomPic();
		}
		break;
	case WM_NCLBUTTONDOWN:
		DefWindowProc(hWnd, message, wParam, lParam); 
		//防止发送NCLBUTTONDOWN后鼠标抬起不响应
		PostMessage(mainwnd, WM_LBUTTONUP, GetSizeType(cursor), MAKELPARAM(cursor.x, cursor.y));
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
		if (!ondrag && !onzoom && !ondragzoom && !onsize)
		{
			SF_SR(surfer.SurfRenew(false));
			needforcerenew = false;//清除需要强制刷新的标志
			
			surfer.GetCurInfo(&cursor, &clientrect);//获取信息
			//pd3dwnd->ChangeVBColor_tail(&colorblock
			//	, SETALPHA(surfer.picpixelcolor, COLOR_BLOCKALPHA), sizeof(FVF2));//改变colorblock模型颜色

		}
		break;
	case WM_MOUSEWHEELEND:
		if (needforcerenew/* && !ondrag && !onzoom && !ondragzoom && !onsize*/)
			PostMessage(mainwnd, WM_SURFFORCERENEW, 0, 1);
		break;
	//	WM_ACTIVATE	- 激活窗口
	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE)
		{
			//处理没有更新的Surface
			if (needforcerenew)
				PostMessage(mainwnd, WM_SURFFORCERENEW, 0, 1);

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
		FullScreen_Windowed(!iswindowedfullscreen);
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
		pnccsp->rgrc[0].top = wndrect.top + 8;
		pnccsp->rgrc[0].bottom = pnccsp->rgrc[0].top + HEIGHTOF(wndrect)-16;
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

	if (point.x > clientrect.left + SIZE_NEAR_PIXEL && point.x < clientrect.right - SIZE_NEAR_PIXEL
		&& point.y > wndrect.top + 12 && point.y < clientrect.bottom - SIZE_NEAR_PIXEL)
		sizetype = HTNOWHERE;
	else if (point.x <= clientrect.left + SIZE_NEAR_PIXEL && point.x >= clientrect.left - SIZE_NEAR_PIXEL
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
	else if (point.y <= wndrect.top + 12 + SIZE_NEAR_PIXEL && point.y >= wndrect.top + 12 - SIZE_NEAR_PIXEL)
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
	// 状态信息显示区域
	textrect_flag.left = TEXTMARGIN_SIDE;
	textrect_flag.top = HEIGHTOF(clientrect) - TEXTMARGIN_BOTTOM - 16;
	textrect_flag.right = textrect_flag.left + 600;
	textrect_flag.bottom = textrect_flag.top + 16;

	// 图片状态显示区域
	textrect_picnum.right = WIDTHOF(clientrect) - 10;
	textrect_picnum.top = TEXTMARGIN_TOP;
	textrect_picnum.left = textrect_picnum.right - 100;
	textrect_picnum.bottom = textrect_picnum.top + 60;
}

bool MYCALL1 Init()
{
	//窗口信息&状态
	OnWinChange();

	GetCursorPos(&cursor);
	lastcursor = cursor;

	wlimit = WINDOWWIDTH_INIT;
	hlimit = WINDOWHEIGHT_INIT;

	hprocess = GetCurrentProcess();
	//状态标志
	purewnd = false;
	iswindowedfullscreen = false;
	isiconic = false;
	ondrag = false;
	ondragzoom = false;
	dragging = false;
	draggingzoom = false;
	onzoom = false;
	onsize = false;
	surfrenew = false;
	needforcerenew = false;
	//选项标志
	mode = MODE_PIC;
	SetWindowMode(WINMODE_INIT);//WINMODE_NORMALWINDOWS
	colorblockon = false;
	backcolor = COLOR_BKG_INIT;
	if(winmode == WINMODE_ROUND)
		sizeEnable = true;
	else
		sizeEnable = false;
	easymoveEnable = true;
	flagshow = true;
	infoshow = true;
	debuginfoshow = false;
	fpslimit = false;
	screencoloron = false;
	screencolor = 0;

	//计时
	loopcount = 0;
	lastloopcount = 0;
	fps = 0.0f;
	staticfps = STATIC_FPS_NORMAL;
	staticframetime = 1000.0f / staticfps;
	normalfps = NORMAL_FPS;
	normalframetime = 1000.0f / normalfps;
	avgfps = -1.0f;
	cvgfps = -1.0f;
	fpscount = 0;
	frametime = 0.0f;
	proctime = 0.0f;

	//D3D
	maindevice = NULL;

	//信息显示
	textrect_pic = RECT(TEXTMARGIN_SIDE, TEXTMARGIN_TOP + 60, 400, 100);
	textrect_surface = RECT(TEXTMARGIN_SIDE, textrect_pic.bottom, 400, 500);

	RefreshTextRect();

	return true;
}

bool MYCALL1 D3DInit()
{
	if (!pd3dwnd->CreateDevice(D3DFMT_A8R8G8B8, 1))
		return false;

	// DEVICE
	maindevice = pd3dwnd->GetDevice();
	// BUFFER SIZE CACHE
	pbufferw = pd3dwnd->GetPBufferWidth();
	pbufferh = pd3dwnd->GetPBufferHeight();

	// SURFER
	surfer.BindDevice(maindevice);
	surfer.BindBuf(pbufferw, pbufferh);
	surfer.SetBackcolor(backcolor);

	// FONT
	pd3dwnd->DXCreateFont(&font, L"Arial Rounded MT Bold"
		, 13, 5, FW_NORMAL, CLEARTYPE_NATURAL_QUALITY);//DEFAULT_QUALITY
	pd3dwnd->DXCreateFont(&font2, L"Consolas"
		, 15, 0, FW_NORMAL, CLEARTYPE_NATURAL_QUALITY);
	pd3dwnd->DXCreateFont(&fontpic, L"苹方 常规"
		, 48, 0, FW_NORMAL, PROOF_QUALITY, 1U, false, 1UL, OUT_TT_PRECIS);
	pd3dwnd->DXCreateFont(&fontcmd, L"Consolas"
		, 15, 0, FW_NORMAL, CLEARTYPE_NATURAL_QUALITY);//PixelSix10,20,10
	// CD3DFont
	/*d3dfont1 = new CD3DFont(L"Arial Rounded MT Bold", 12, 0);
	d3dfont1->InitDeviceObjects(maindevice);
	d3dfont1->RestoreDeviceObjects();*/
	picinfostr[0] = L'\0';
	
	// SPRITE
	/*D3DXCreateSprite(maindevice, &m_sprite);
	D3DXMATRIX matTransform;
	D3DXMatrixIdentity(&matTransform);
	m_sprite->SetTransform(&matTransform);*/

	// OBJECT
	//pd3dwnd->CreateMesh_Custom1(&backdecorate);//自创模型并保存
	D3DXLoadMeshFromXW(L"crystal.x", D3DXMESH_MANAGED, maindevice, NULL, NULL, NULL, NULL, &backdecorate);
	D3DXMatrixIdentity(&basemat);
	D3DXMatrixRotationX(&basemat, PI_F / 2);

	/*pd3dwnd->CreateVertexBuffer_Custom1(&colorblock, COLORBLOCK_X, COLORBLOCK_Y, COLORBLOCK_RADIUS);
	pd3dwnd->CreateVertexBuffer_Custom1(&colorblockback
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
	g_gui = new CD3DGUISystem(maindevice);
	g_gui->Bind(pd3dwnd);
	g_gui->SetEventProc(GUICallback);
	g_gui->AddDXFont(L"Consolas", &fontID1, 15, 0, 0, CLEARTYPE_NATURAL_QUALITY);

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
	if (!maindevice)
	{
		return false;
	}

	PicPack *newpp = new PicPack;
	HRESULT hr = newpp->LoadFile(maindevice, file);//尝试加载新图片
	if (SUCCEEDED(hr))
	{
		// 图片列表扩充 & 更新当前图片
		piclist.Add(newpp);
		SetTailPic();

		// TODO重复renew settailpic中也有
		surfer.SurfCenterPR(*pbufferw, *pbufferh);
		surfrenew = surfer.SurfRenew(false);
		if (surfrenew)
			surfrenewtick = GetTickCount();

		return true;
	}
	else
	{
		delete newpp;//释放

		SetForegroundWindow(mainwnd);
		D3DErrorShow(hr, L"Load File", mainwnd);// 显示错误信息

		return false;
	}
}

void OnSave()
{
	if (haspic)
	{
		if (mainpicpack)
			SaveFileWin(mainpicpack->GetFileName());
	}
}

void OnSaveAs()
{
	if (haspic)
	{
		if (mainpicpack)
			SaveFileWin(mainpicpack->GetFileName());
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

	if (onzoom)
	{
		if (nowtick - zoomtick > FLAGDELAY_WHEEL || nowtick < zoomtick)
		{
			onzoom = false;
			PostMessage(mainwnd, WM_MOUSEWHEELEND, 0, 0);
		}
	}
	if (onsize)
	{
		if (nowtick - sizetick > FLAGDELAY_SIZE || nowtick < sizetick)
			onsize = false;
	}
	if (draggingzoom)
	{
		if (nowtick - dragzoomtick > FLAGDELAY_DRAGZOOM || nowtick < dragzoomtick)
			draggingzoom = false;
	}
	if (dragging)
	{
		if (nowtick - dragtick > FLAGDELAY_DRAG || nowtick < dragtick)
			dragging = false;
	}
	if (surfrenew)
	{
		if (nowtick - surfrenewtick > FLAGDELAY_SURFREFRESH || nowtick < surfrenewtick)
			surfrenew = false;
	}
}

void MYCALL1 ClearFlag()
{
	ReleaseCapture();//统一释放
	if (LMBdown)
	{
		LMBdown = false;
	}
	if (ondrag)
	{
		ondrag = false;
	}
	if (ondragzoom)
	{
		ondragzoom = false;
	}
	draggingzoom = false;
	dragging = false;
	onzoom = false;
	onsize = false;
	surfrenewtick = false;
	needforcerenew = false;
}

void InitNonPic()
{
	mainpicpack = NULL;
	mainbmp = NULL;

	haspic = HasPic();
}

bool SetPic(short picidx)
{
	PicPack *oldpicpack = mainpicpack;
	piclist.SetPicPack(picidx);

	// 更新本地
	mainpicpack = piclist.pLivePicPack;
	mainbmp = piclist.pLiveBMP;

	SetSurface(mainpicpack, oldpicpack);
	SetPicInfo();
	haspic = HasPic();

	return true;
}

bool SetTailPic()
{
	PicPack *oldpicpack = mainpicpack; 
	piclist.SetTailPicPack();

	// 更新本地
	mainpicpack = piclist.pLivePicPack;
	mainbmp = piclist.pLiveBMP;

	SetSurface(mainpicpack, oldpicpack);
	SetPicInfo();
	haspic = HasPic();

	return true;
}

bool SetPrevPic()
{
	PicPack *oldpicpack = mainpicpack;
	piclist.SetPrev();

	// 更新本地
	mainpicpack = piclist.pLivePicPack;
	mainbmp = piclist.pLiveBMP;

	SetSurface(mainpicpack, oldpicpack);
	SetPicInfo();
	haspic = HasPic();

	return true;
}

bool SetNextPic()
{
	PicPack *oldpicpack = mainpicpack;
	piclist.SetNext();

	// 更新本地
	mainpicpack = piclist.pLivePicPack;
	mainbmp = piclist.pLiveBMP;

	SetSurface(mainpicpack, oldpicpack);
	SetPicInfo();
	haspic = HasPic();

	return true;
}

void Drop()
{
	piclist.Drop();

	// 更新本地
	mainpicpack = piclist.pLivePicPack;
	mainbmp = piclist.pLiveBMP;

	SetSurface(mainpicpack, NULL);// 不解绑
	SetPicInfo();
	haspic = HasPic();
}

void SetSurface(PicPack *newpicpack, PicPack *oldpicpack)
{
	if (oldpicpack)
		surfer.DeBindPic(oldpicpack);// 解绑
	surfer.BindPic(newpicpack);// 捆绑

	//标志
	surfrenew = true;//TODO强制设置为已更新？
	surfrenewtick = GetTickCount();
}

void SetPicInfo()
{
	if (mainpicpack)
	{
		wcscpy_s(picinfostr, mainpicpack->GetPicInfoStrW());// 图片信息字符串更新
		SetWindowTextW(mainwnd, mainpicpack->GetFileName());// 窗口标题
	}
	else
	{
		picinfostr[0] = L'\0';
		SetWindowTextW(mainwnd, L"");
	}
	surfer.GetCurInfo(&cursor, &clientrect);//获取信息
}

void PicMove(int dx, int dy)
{
	SF_SRFR(surfer.OnMove_Custom({ dx, dy }));

	surfer.GetCurInfo(&cursor, &clientrect);
}

void PicRestore()
{
	POINT base;
	base.x = cursor.x - clientrect.left;
	base.y = cursor.y - clientrect.top;

	surfer.SurfSetZoomPR(1);
	SF_SR(surfer.SurfZoomRenew(&base, true, false));
	SF_OZ();

	surfer.GetCurInfo(&cursor, &clientrect);
}

void PicClipWindow()
{
	SF_SRFR(surfer.OnMove_Custom(surfer.GetSrc()));

	wndrect.right = wndrect.left + surfer.GetZoomWidth() + wbias;
	wndrect.bottom = wndrect.top + surfer.GetZoomHeight() + hbias;
	SetWindowPos(mainwnd, HWND_TOP, wndrect.left, wndrect.top
		, wndrect.right - wndrect.left, wndrect.bottom - wndrect.top
		, 0);

	surfer.GetCurInfo(&cursor, &clientrect);
}

void BeginDragWindow_Custom(int cursorx, int cursory)
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
		if (ISCONTROLDOWN)
		{
			switch (wParam)
			{
			case 'F':	// 设置标准窗口尺寸
				OpenFileWin();
				break;
			case 'I':
				debuginfoshow = !debuginfoshow;
				break;
			case 'M':
			//multisample导致图片不显示
				pd3dwnd->ChangeMultiSample();
				ResetDevice();
				break;
			case 'S':
				OnSave();
				break;
			case 'W':	// 改变窗口模式
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
			case 'A':
			case VK_LEFT:
				if (haspic)
					PicMove(-1, 0);
				break;
			case 'B':	// 图片恢复原始像素
				if (haspic)
					PicRestore();
				break;
			case 'C':	// 窗口调整到正好包括图片
				if (haspic)
					PicClipWindow();
				break;
			case 'D':
			case VK_RIGHT:
				if (haspic)
					PicMove(1, 0);
				break;
			case 'E':	// 切换信息显示
				infoshow = !infoshow;
				break;
			case 'F':	// 设置标准窗口尺寸
				if (haspic)
					PicFitWnd();
				break;
			case 'I':
				colorblockon = !colorblockon;
				break;
			case 'M':	// 图片居中
				if (haspic)
					PicCenter();
				break;
			case 'N':
				ToggleNight();
				break;
			case 'Q':	// 图片放回左上角
				if (haspic)
					PicDock();
				break;
			case 'R':	// 手动渲染一次
				Render();
				break;
			case 'S':
			case VK_DOWN:
				if (haspic)
					PicMove(0, 1);
				break;
			case 'T':
				if (haspic)
					PicFit();
				break;
			case 'V':	// 手动重新生成surface一次
				if (haspic)
				{
					if (!ondragzoom && !onzoom)//drazoom时按v在mousemove中刷新surface
					{
						SF_SR(surfer.SurfRenew(false));

						surfer.GetCurInfo(&cursor, &clientrect);
					}
				}
				break;
			case 'W':	// 改变窗口模式
			case VK_UP:
				if (haspic)
					PicMove(0, -1);
				break;
			case 'X':	// 重置收敛帧率值（重新开始计算）
				fpscount = 0;
				cvgfps = -1;
				break;
			case VK_F1:	// 显示显卡信息			
				pd3dwnd->DisplayAdapter();
				break;
			case VK_F3:
				PureWindow(!purewnd);
				break;
			case VK_F4:
				PostMessage(mainwnd, WM_TOGGLEFULLSCREEN, 0, 1);
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
	opfn.hwndOwner = mainwnd;//模态
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
	svfn.hwndOwner = mainwnd;//模态
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
		//	if (IDYES == MessageBoxW(mainwnd, L"是否覆盖？", L"文件已存在", MB_YESNO | MB_APPLMODAL))
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
	if (!mainpicpack)
	{
		return false;
	}

	if (!mainpicpack->SaveFile(maindevice, file))
	{
		MessageBoxW(mainwnd, L"Save File FAILED!", L"ERROR", MB_OK | MB_APPLMODAL);
		return false;
	}
	else
	{
		SetWindowTextW(mainwnd, file);//窗口标题

		wcscpy_s(picinfostr, mainpicpack->GetPicInfoStrW());

		return true;
	}
}

void SetWindowMode(UINT8 wmode)
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
		rgnrect.right = wndrect.right - clientrect.left + 1;//wndrect.right - clientrect.left + 1
		rgnrect.bottom = HEIGHTOF(wndrect) - rgnrect.top + 1;//HEIGHTOF(wndrect) - rgnrect.top + 1
		if (iswindowedfullscreen)
			hrgn = CreateRectRgn(rgnrect.left, rgnrect.top
				, rgnrect.right, rgnrect.bottom);
		else
			hrgn = CreateRoundRectRgn(rgnrect.left, rgnrect.top
				, rgnrect.right, rgnrect.bottom, 2, 2);
		SetWindowRgn(mainwnd, hrgn, TRUE);

		DeleteObject(hrgn);
	}
}

void MYCALL1 BeginDragPic()
{
	SetCapture(mainwnd);//允许鼠标在窗口外拖动

	lastcursor = cursor;

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

	surfer.SetBasePoint({ cursor.x - clientrect.left,cursor.y - clientrect.top });

	lastcursor = cursor;

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
	SF_SRFR(surfer.OnMove_Custom(surfer.GetSrc()));

	wndrect.right = wndrect.left + wlimit + wbias;
	wndrect.bottom = wndrect.top + wlimit * mainbmp->width / mainbmp->height + hbias;
	if (HEIGHTOF(wndrect) > hlimit)
	{
		wndrect.bottom = wndrect.top + hlimit + hbias;
		wndrect.right = wndrect.left + hlimit * mainbmp->width / mainbmp->height + wbias;
	}

	SetWindowPos(mainwnd, HWND_TOP, wndrect.left, wndrect.top
		, WIDTHOF(wndrect), HEIGHTOF(wndrect)
		, 0);

	surfer.GetCurInfo(&cursor, &clientrect);
}

void PicDock()
{
	SF_SRFR(surfer.OnMove_Custom(surfer.GetSrc()));

	surfer.GetCurInfo(&cursor, &clientrect);
}

void PicCenter()
{
	surfer.SurfCenterPR(*pbufferw, *pbufferh);
	SF_SR(surfer.SurfRenew(false));

	surfer.GetCurInfo(&cursor, &clientrect);
}

void PicFit()
{
	SF_SR(surfer.SurfSuit(*pbufferw, *pbufferh));

	surfer.GetCurInfo(&cursor, &clientrect);
}

bool FullScreen_Windowed(bool tofull, bool winchange)
{
	if (iswindowedfullscreen == tofull)
		return false;
	iswindowedfullscreen = tofull;
	if (iswindowedfullscreen)// 全屏化
	{
		originwndrect = wndrect;
		RECT fullScreenRect;
		fullScreenRect.left = wndrect.left - clientrect.left;
		fullScreenRect.top = wndrect.top - clientrect.top + 22;
		if (purewnd)
			fullScreenRect.top -= 22;
		fullScreenRect.right = wndrect.right
			- clientrect.right + GetSystemMetrics(SM_CXSCREEN) + 12;
		fullScreenRect.bottom = wndrect.bottom
			- clientrect.bottom + GetSystemMetrics(SM_CYSCREEN);
		loopcount = GetSystemMetrics(SM_CXSCREEN);

		// 隐藏任务栏
		/*HWND taskwnd, startbutton;
		taskwnd = FindWindow(L"Shell_TrayWnd", NULL);
		ShowWindow(taskwnd, SW_HIDE);
		startbutton = FindWindow(_T("Button"), NULL);
		ShowWindow(startbutton, SW_HIDE);
		taskwnd=GetDlgItem(FindWindow(L"Shell_TrayWnd", NULL), 0x130);
		ShowWindow(taskwnd, SW_HIDE);*/

		// 去除边框，顺利进入全屏
		LONG tmp = GetWindowLong(mainwnd, GWL_STYLE);
		tmp &= ~WS_BORDER;
		tmp |= WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		SetWindowLong(mainwnd, GWL_STYLE, tmp);
		purewnd = true;

		SetWindowPos(mainwnd, HWND_TOP, fullScreenRect.left, fullScreenRect.top
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

		LONG tmp = GetWindowLong(mainwnd, GWL_STYLE);
		tmp |= WS_POPUPWINDOW;
		tmp |= WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		SetWindowLong(mainwnd, GWL_STYLE, tmp);
		purewnd = false;

		if (winchange)
		{
			//SetWindowPos比MoveWindow发送更少的消息，加速，减缓图片闪烁
			SetWindowPos(mainwnd, HWND_TOP, originwndrect.left, originwndrect.top
				, WIDTHOF(originwndrect), HEIGHTOF(originwndrect)
				, SWP_FRAMECHANGED);
		}
		g_gui->SetControlText(BUTTON_ID_3, L"full");
	}

	return true;
}

bool PureWindow(bool topure)
{
	if (purewnd == topure)
		return false;
	purewnd = topure;

	// 更改窗口属性
	LONG oldwindowlong = GetWindowLong(mainwnd, GWL_STYLE);
	if (purewnd)
	{
		oldwindowlong &= ~WS_BORDER;
	}
	else
	{
		oldwindowlong |= WS_POPUPWINDOW;
	}
	SetWindowLong(mainwnd, GWL_STYLE, oldwindowlong);

	// 使得窗口属性更改后，外观立刻更新
	SetWindowPos(mainwnd, HWND_TOP, wndrect.left, wndrect.top
		, WIDTHOF(wndrect), HEIGHTOF(wndrect)
		, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOSENDCHANGING);

	return false;
}

void SetStaticFps(float sfps)
{
	if (sfps > 0)
	{
		staticfps = (short)sfps;
		staticframetime = 1000.0f / sfps;
	}
}

void ToggleNight()
{
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
}

inline void MYCALL1 SetRenderState()
{
	// 标配：D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA
	maindevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	maindevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	maindevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	maindevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	maindevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	maindevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	maindevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	maindevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	maindevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	maindevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);

	maindevice->SetRenderState(D3DRS_LIGHTING, TRUE); 
	maindevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	maindevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	//maindevice->SetRenderState(D3DRS_AMBIENT,
	//	D3DCOLOR_COLORVALUE(0.3f, 0.3f, 0.3f, 1.0f));

	//maindevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);//不需要
	maindevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);

	maindevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
	//maindevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	maindevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

inline void SetView()
{
	// mesh方法：动态调整视角
	if (backdecorate)
	{
		D3DXVECTOR3 eye;
		D3DXVECTOR3 at;
		float eyeradius = 40.0f;
		eye = D3DXVECTOR3(0.0f, -eyeradius, 0.0f);
		at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DXMATRIXA16 matView;
		D3DXMatrixLookAtLH(&matView, &eye, &at, &D3DXVECTOR3(0.0f, 0.0f, -1.0f));
		maindevice->SetTransform(D3DTS_VIEW, &matView);
		D3DXMATRIXA16 proj;
		D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 3
			, (float)WIDTHOF(clientrect) / HEIGHTOF(clientrect)
			, 0.02f, 100.0f);
		maindevice->SetTransform(D3DTS_PROJECTION, &proj);

		time_t nowtick = GetTickCount();
		const float angle = nowtick > decoratetick ? 0.0014f*(nowtick - decoratetick) : 0;
		decoratetick = nowtick;
		D3DXMATRIX rotnew;
		D3DXMatrixRotationZ(&rotnew, angle);
		D3DXMatrixMultiply(&basemat, &basemat, &rotnew);
		maindevice->SetTransform(D3DTS_WORLD, &basemat);
	}
}

inline void SetLight()
{
	if (backdecorate)
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
		maindevice->SetLight(0, &light);
		maindevice->LightEnable(0, TRUE);
	}
}

bool InfoRender()
{
	WCHAR infowstr[512] = { 0 };

	// 图片状态信息显示
	swprintf_s(infowstr, L"%d/%d", piclist.GetCurPos(), piclist.GetCount());
	fontpic->DrawTextW(NULL, infowstr, -1, &textrect_picnum, DT_RIGHT | DT_NOCLIP, COLOR_TEXT4);

	// 第1部分信息显示（图片信息）
	// d3dfont1->DrawTextW(100, 200, COLOR_TEXT1, infowstr0, 0);//测试高速字体绘制
	font->DrawTextW(NULL, picinfostr, -1, &textrect_pic, DT_LEFT | DT_NOCLIP, COLOR_TEXT1);

	// 窗口信息 & surfer信息
	swprintf_s(infowstr, L"MODE: %d\n\
		FPS: %.2f/%.2f (%.2f)  %.3fms   %lld\n\
		MEM: %.1fMB,  %.1fMB\n\
		BUF: %d × %d\n\
		CLIENT: %d, %d\n\
		PROC: %.3fms\n"
		, mode
		, fps, avgfps, cvgfps, frametime, loopcount
		, memoryin, memoryout
		, *pbufferw, *pbufferh
		, cursor.x - clientrect.left, cursor.y - clientrect.top
		, proctime);
	wcscat_s(infowstr, L"   -    -    -    -   \n");
	wcscat_s(infowstr, surfer.GetInfoStr());
	font->DrawTextW(NULL, infowstr, -1, &textrect_surface, DT_LEFT | DT_NOCLIP, COLOR_TEXT1 );
	//font->PreloadText();

	// 附加信息
	//const string cursorposshow[3] = { "PIC", "BLANK", "OUTSIDE CLIENT" };
	//	"backcolor: %08X.ARGB\n
	//	screen color: %02X.%06X.ARGB
	//	cursor: %d, %d\n\"
	//	, (screencolor >> 24), (screencolor & 0xFFFFFF)
	//	, cursor.x, cursor.y

	// 标志信息
	if (debuginfoshow)
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
			, yesno2[haspic]
			, yesno2[surfer.IsSurfNotNull()], yesno2[onzoom]
			, yesno2[dragging], yesno2[draggingzoom]
			, yesno2[onsize], yesno2[surfer.clip]
			, yesno2[surfer.surfclipped], yesno2[surfer.picclipped]
			, yesno2[surfer.outsideclient], yesno2[surfrenew]);

		// 第3部分信息显示
		font2->DrawTextW(NULL, infowstr, -1, &textrect_flag
			, DT_LEFT | DT_TOP | DT_NOCLIP, COLOR_TEXT3);
	}

	return true;
}

void MYCALL1 Render()
{
	static HRESULT hr;

	maindevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, backcolor, 1.0f, 0);
	maindevice->BeginScene();

	g_gui->RenderBack();

	// 装饰绘制
	if (backdecorate)
	{
		SetView();
		maindevice->SetMaterial(&material);
		maindevice->SetTexture(NULL, NULL);
		maindevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		maindevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		backdecorate->DrawSubset(0);
	}
	maindevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	maindevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	// 图片绘制
	hr = surfer.Render();

	if (infoshow)//信息显示
		InfoRender();

	// D3DGUI
	g_gui->Render();//D3DBLEND_INVSRCCOLOR,D3DBLEND_INVSRCCOLOR;D3DBLEND_ZERO,D3DBLEND_INVDESTCOLOR

	// 颜色块
	/*if (colorblockon && colorblock)
	{
		maindevice->SetTexture(NULL, NULL);

		maindevice->SetStreamSource(0, colorblockback, 0, sizeof(FVF2));
		maindevice->SetFVF(FVF_2CPD);
		maindevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 20);
		maindevice->SetStreamSource(0, colorblock, 0, sizeof(FVF2));
		maindevice->SetFVF(FVF_2CPD);
		maindevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 20);
	}*/

	maindevice->EndScene();
	hr = maindevice->Present(NULL, NULL, NULL, NULL);

	// 处理设备丢失
	if (hr == D3DERR_DEVICELOST)
	{
		if (maindevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			PostMessage(mainwnd, WM_RESETDEVICE, 0, 0);
		}
	}
}

bool ResetDevice()
{
	if (!maindevice)
		return false; 

	// 修改device长宽
	OnLostDevice();
	bool result = OnResetDevice2(WIDTHOF(clientrect), HEIGHTOF(clientrect));

	PostResetDevice();

	return result;
}

void OnLostDevice()
{
	pd3dwnd->OnLostDevice();

	g_gui->OnLostDevice();

	//D3D
	font->OnLostDevice();
	font2->OnLostDevice();
	fontpic->OnLostDevice();
	fontcmd->OnLostDevice();
}

bool OnResetDevice2(int clientw, int clienth)
{
	if (!pd3dwnd->OnResetDevice(clientw, clienth))
	{
		return false;
	}

	g_gui->OnResetDevice();

	fontcmd->OnResetDevice();
	font2->OnResetDevice();
	fontpic->OnResetDevice();
	font->OnResetDevice();

	return true;
}

bool OnResetDevice()
{
	if (!pd3dwnd->OnResetDevice())
	{
		return false;
	}

	//g_gui->OnResetDevice(maindevice);
	fontcmd->OnResetDevice();
	font2->OnResetDevice();
	font->OnResetDevice();

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
			PostMessage(mainwnd, WM_COMMAND, IDM_OPEN, 0);
		}
		break;
	case BUTTON_ID_2:
		if (wp == GUI_EVENT_UP)
		{
			PostMessage(mainwnd, WM_COMMAND, IDM_SAVE, 0);
		}
		break;
	case BUTTON_ID_3:
		if (wp == GUI_EVENT_UP)
		{
			PostMessage(mainwnd, WM_TOGGLEFULLSCREEN, 0, 1);
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
		if (haspic)
		{
			mainbmp->Gray();
			surfrenew = surfer.SurfRenew(false);

			if (surfrenew)
				surfrenewtick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"loser") == 0)
	{
		if (haspic)
		{
			mainbmp->LOSE_R();
			surfrenew = surfer.SurfRenew(false);

			if (surfrenew)
				surfrenewtick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"loseg") == 0)
	{
		if (haspic)
		{
			mainbmp->LOSE_G();
			surfrenew = surfer.SurfRenew(false);

			if (surfrenew)
				surfrenewtick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"loseb") == 0)
	{
		if (haspic)
		{
			mainbmp->LOSE_B();
			surfrenew = surfer.SurfRenew(false);

			if (surfrenew)
				surfrenewtick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"inv") == 0)
	{
		if (haspic)
		{
			mainbmp->Inverse();
			surfrenew = surfer.SurfRenew(false);

			if (surfrenew)
				surfrenewtick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"inva") == 0)
	{
		if (haspic)
		{
			mainbmp->InverseAlpha();
			surfrenew = surfer.SurfRenew(false);

			if (surfrenew)
				surfrenewtick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"inv4") == 0)
	{
		if (haspic)
		{
			mainbmp->InverseAll();
			surfrenew = surfer.SurfRenew(false);

			if (surfrenew)
				surfrenewtick = GetTickCount();
		}
	}
	else if (_wcsicmp(wstr, L"hsv") == 0)
	{
		if (haspic)
		{
			mainbmp->RGB2HSV();
			surfrenew = surfer.SurfRenew(false);

			if (surfrenew)
				surfrenewtick = GetTickCount();
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
			if (haspic && num > ZOOM_MIN)
			{
				POINT base;
				base.x = cursor.x - clientrect.left;
				base.y = cursor.y - clientrect.top;
				surfer.SurfSetZoomPR(num);
				surfrenew = surfer.SurfZoomRenew(&base, true, false);//放大，调整surface位置
				
				//标志
				if (surfrenew)
					surfrenewtick = GetTickCount();
				onzoom = true;
				zoomtick = GetTickCount();
			}
		}
	}
	//获取surface信息（图片更新后，surface信息也立即更新）
	surfer.GetCurInfo(&cursor, &clientrect);

	QueryPerformanceCounter(&e);
	proctime = 1000.0f*(e.QuadPart - s.QuadPart) / (float)freq.QuadPart;
}

//void CALLBACK TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
//{
//	if (uID == TimerID_main)
//	{
//		timershot = true;
//	}
//}
