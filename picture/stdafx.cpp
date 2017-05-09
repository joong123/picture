// stdafx.cpp : 只包括标准包含文件的源文件
// picture.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中引用任何所需的附加头文件，
//而不是在此文件中引用


#ifdef USE_GDIGUI

CBitmap *MyDrawPrepareOne(CDC *pDC, memDCBMP *mDC, int x, int y, CRect &r)
{
	mDC->x = x;
	mDC->y = y;
	mDC->pDC = new CDC;
	mDC->pDC->CreateCompatibleDC(pDC);
	mDC->bmp = new CBitmap;
	mDC->bmp->CreateCompatibleBitmap(pDC, r.Width(), r.Height());
	mDC->poldbmp = mDC->pDC->SelectObject(mDC->bmp);
	CRect r0(0, 0, r.Width(), r.Height());
	mDC->pDC->IntersectClipRect(&r0);
	mDC->cx = r.Width();
	mDC->cy = r.Height();

	return mDC->poldbmp;
}



void DeleteMemDCBMP(memDCBMP *mDC)
{
	if (mDC)
	{
		if (mDC->pDC)
		{
			mDC->pDC->SelectObject(mDC->poldbmp);
			mDC->pDC->DeleteDC();
		}
		if (mDC->bmp)
		{
			mDC->bmp->DeleteObject();
		}
	}
}

#endif // USE_GDIGUI