// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料

//定义GUI系统的使用
#define USE_D3DGUI
//#define USE_GDIGUI//是否采用GDI式GUI

#ifdef USE_GDIGUI
// Windows 头文件
//afxwin.h is OK before windows.h
#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展
#endif // USE_GDIGUI

#include <windows.h>
#include <atltypes.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO:  在此处引用程序需要的其他头文件
#ifdef USE_GDIGUI
struct memDCBMP
{
	CBitmap *poldbmp;

	CDC *pDC;
	CBitmap *bmp;
	int x, y, cx, cy;
};

CBitmap *MyDrawPrepareOne(CDC *pDC, memDCBMP *mDC, int x, int y, CRect &r);
void DeleteMemDCBMP(memDCBMP *mDC);
#endif // USE_GDIGUI
