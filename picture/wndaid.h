#pragma once

#include "d3dwnd.h"
//bool g_isModaling;

int DoModal(HWND *pHWnd, HWND hWndParent = NULL);

void EndModal(int nCode);

LRESULT CALLBACK    StartWndProc(HWND, UINT, WPARAM, LPARAM);