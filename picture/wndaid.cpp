#include "stdafx.h"
#include "wndaid.h"

int DoModal(HWND * pHWnd, HWND hWndParent)
{
	void EndModal(int nCode);
	//void MYCALL1 Render();

	if (pHWnd == NULL)
		return -1;
	HWND hWnd = *pHWnd;

	if (hWnd == NULL || !IsWindow(hWnd))
		return -1;

	//标识处于模态状态中   
	//g_isModaling = TRUE;
	//显示自己   
	ShowWindow(hWnd, SW_SHOW);
	BringWindowToTop(hWnd);
	//disable掉父窗口   
	HWND hParentWnd = hWndParent;
	while (hParentWnd != NULL)
	{
		//EnableWindow(hParentWnd, FALSE);
		hParentWnd = GetParent(hParentWnd);
	}
	//接管消息循环   
	MSG msg;
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.hwnd == hWnd)
			{
				if (msg.message == WM_CLOSE || 
					msg.message == WM_SYSCOMMAND && msg.wParam == SC_CLOSE)
				{
					EndModal(0);
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
				}
			}
			else if (msg.hwnd == hWndParent)
			{
				if (msg.message == WM_CLOSE ||
					//(msg.message == WM_ACTIVATE && msg.wParam == WA_ACTIVE) ||//TODOW无效
					(msg.message == WM_SYSCOMMAND && msg.wParam == SC_CLOSE) ||
					msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN ||
					msg.message == WM_NCLBUTTONDOWN || msg.message == WM_NCRBUTTONDOWN)
				{
					EndModal(0);
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
				}
			}
		}
		//Render();

		Sleep(16);
	}
	//模态已经退出   
	//恢复父窗口的enable状态   
	hParentWnd = hWndParent;
	while (hParentWnd != NULL)
	{
		EnableWindow(hParentWnd, TRUE);
		hParentWnd = GetParent(hParentWnd);
	}

	return 0;//g_nModalCode;
}

void EndModal(int nCode)
{
	//g_nModalCode = nCode;
	//g_isModaling = FALSE;
	PostMessage(NULL, WM_NULL, 0, 0);
}

D3DWnd *startup;
HWND hWndStartup = NULL;
bool bDrag;
POINT ptCurStartup;
//CD3DGUISystem *pGuiStartup;

HWND hWndAid;
PROCESS_INFORMATION pi;
STARTUPINFO si;//用于指定新进程的主窗口特性的一个结构


LRESULT CALLBACK StartWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		POINT clienttl;
		POINT ptCurNew/*, ptBias*/;
		RECT rcClient, rcWindow;
	case WM_CREATE:
		GetCursorPos(&ptCurStartup);

		clienttl = { 0, 0 };
		GetClientRect(hWnd, &rcClient);//得到client区域尺寸
		ClientToScreen(hWnd, &clienttl);//获得client区域左上角的屏幕坐标
										//得到client真实屏幕区域
		rcClient.left = clienttl.x;
		rcClient.top = clienttl.y;
		rcClient.right += clienttl.x;
		rcClient.bottom += clienttl.y;
		//得到窗口区域
		GetWindowRect(hWnd, &rcWindow);

		HRGN hrgn;

		RECT rgnrect;
		rgnrect.left = rcClient.left - rcWindow.left;
		rgnrect.top = 30;
		rgnrect.right = WIDTHOF(rcWindow) - rgnrect.left + 1/*WIDTHOF(rcClient) + 9*/;
		rgnrect.bottom = HEIGHTOF(rcWindow) - rgnrect.top + 1/*HEIGHTOF(rcClient) + 22*/;
		hrgn = CreateRoundRectRgn(rgnrect.left, rgnrect.top
			, rgnrect.right, rgnrect.bottom, 2, 2);//尺寸，要+1
		SetWindowRgn(hWnd, hrgn, TRUE);
		/*GetWindowRect(hWnd, &wrect);
		SetWindowPos(hWnd, HWND_TOP
		, wrect.left, wrect.top, wrect.right, wrect.bottom, 0);*/

		bDrag = false;


		::SetWindowLong(hWnd, GWL_EXSTYLE,
			::GetWindowLongPtr(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hWnd, 0, 200, LWA_ALPHA);

		break;
		//case WM_PAINT:
		//	break;
	case WM_MOUSEMOVE:
		GetCursorPos(&ptCurNew);
		//ptBias = MINUSPOINT(ptCurNew, ptCurStartup);
		//if(bDrag)
		//{
		//	GetWindowRect(hWnd, &rcWindow);
		//	MoveWindow(hWnd, rcWindow.left + ptBias.x, rcWindow.top + ptBias.y
		//		, WIDTHOF(rcWindow), HEIGHTOF(rcWindow), TRUE);
		//	//PostMessageW(hWnd, WM_MOVE, 0, MAKELPARAM(ptBias.x, ptBias.y));//0x001D0003
		//}
		ptCurStartup = ptCurNew;
		break;
	case WM_LBUTTONDOWN:
		//不使用PostMessage，HTCAPTION是为了和主窗口分开，新窗口依赖于主窗口主循环频率
		/*bDrag = true;
		SetCapture(hWnd);
		GetCursorPos(&ptCurStartup);*/
		PostMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		break;
	case WM_LBUTTONUP:
		if (bDrag)
		{
			bDrag = false;
			ReleaseCapture();
		}
		break;
	case WM_KILLFOCUS:
		if (bDrag)
		{
			bDrag = false;
			ReleaseCapture();
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
