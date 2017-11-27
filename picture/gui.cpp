#include "stdafx.h"
#include "gui.h"

LARGE_INTEGER CGUIControl::frequency = { 0 };

#ifdef USE_GDIGUI
float CGDIGUISystem::time = 0;
#endif // USE_GDIGUI

#ifdef USE_D3DGUI
void D3DErrorShow(HRESULT hr, WCHAR * msg, HWND hwnd, WCHAR * title)
{
	WCHAR errormsg[256] = { 0 };
	if (msg)
	{
		wcscpy_s(errormsg, msg);
		wcscat_s(errormsg, L": ");
	}

	switch (hr)
	{
	case E_OUTOFMEMORY:
		wcscat_s(errormsg, L"OUT OF MEMORY.");
		break;
	case E_FAIL:
		wcscat_s(errormsg, L"E_FAIL");
		break;
	case D3DERR_INVALIDCALL:
		wcscat_s(errormsg, L"D3DERR_INVALIDCALL");
		break;
	default:
		WCHAR resStr[32] = { 0 };
		StringCchPrintfW(resStr, 32, L"0x%8x", hr);
		wcscat_s(errormsg, resStr);
		break;
	}
	MessageBoxW(hwnd, errormsg, title, MB_OK | MB_APPLMODAL);
}

void ChangeAlpha(LPDIRECT3DVERTEXBUFFER9 pbuf, byte alpha)
{
	if (!pbuf)
		return;

	D3DVERTEXBUFFER_DESC vbd;
	pbuf->GetDesc(&vbd);

	void *ptr;
	if (FAILED(pbuf->Lock(0, vbd.Size, (void**)&ptr, 0)))
		return;
	*((byte*)ptr + sizeof(D3DXVECTOR4) + 3) = alpha;
	*((byte*)ptr + sizeof(D3DGUIVertex) + sizeof(D3DXVECTOR4) + 3) = alpha;
	*((byte*)ptr + 2 * sizeof(D3DGUIVertex) + sizeof(D3DXVECTOR4) + 3) = alpha;
	*((byte*)ptr + 3 * sizeof(D3DGUIVertex) + sizeof(D3DXVECTOR4) + 3) = alpha;

	pbuf->Unlock();
}
#endif // USE_D3DGUI

bool mywcscpy(WCHAR ** dest, WCHAR * src)
{
	if (!dest || !src)
		return false;

	if (*dest)
		delete[] * dest;

	int len = wcslen(src);
	(*dest) = new WCHAR[len + 1];
	if (!*dest)
		return false;

	//方法1
	memcpy((*dest), src, len * 2);
	(*dest)[len] = (WCHAR)'\0';
	//方法2
	/*(*dest)[len] = 0;
	wcscpy_s(*dest, len + 1, src);*/

	return true;
}

#ifdef USE_GDIGUI
#define DECIMALPART(F)		((F) - (int)(F))
#define PIXELOFPOS(FP)		/*((int)((FP) + 0.5f))*/(ceilf(FP - 0.5f))
#define CEILPIXELBOUND(FP)	(PIXELOFPOS(FP) + 0.5f)
bool SmoothLine(CDC * pDC, const POINT & start, const POINT & end, const DWORD & color)
{
	if (pDC == NULL)
		return false;

	// smooth part
	if ((start.x != end.x) || (start.y != end.y))
	{
		POINT ps, pe;
		ASIGNPOINT(ps, start);
		ASIGNPOINT(pe, end);

		//垂直线
		if (end.x == start.x)
		{
			pDC->MoveTo(start);
			pDC->LineTo(end);
			return true;
		}

		float slope = (pe.y - ps.y) / (float)(pe.x - ps.x);
		bool bSlopePositive = slope >= 0;
		float hproj;
		float hspan;
		int delta;
		float param1, param2;
		int lstart, lend;
		if ((pe.y - ps.y > pe.x - ps.x) || (pe.y - ps.y < ps.x - pe.x))
		{
			hproj = abs(0.5f / slope);
			delta = (pe.y > ps.y) ? 1 : -1;
			param1 = 1.0f / slope;
			param2 = ps.x - ps.y / slope;
			lstart = ps.y, lend = pe.y;
		}
		else
		{
			hproj = abs(0.5f * slope);
			delta = (pe.x > ps.x) ? 1 : -1;
			param1 = slope;
			param2 = ps.y - ps.x*slope;
			lstart = ps.x, lend = pe.x;
		}
		hspan = sqrtf(0.25f + hproj*hproj);

		if ((pe.y - ps.y > pe.x - ps.x) || (pe.y - ps.y < ps.x - pe.x))//偏倾斜直线
		{
			float lPos = lstart*param1 + param2;
			for (int i = lstart; i != lend; i += delta, lPos += param1*delta)
			{
				float lBias = lPos - (int)lPos;
				/*if (lBias < 0)
				lBias += 1;*/
				DWORD blendColor = 0;//混合颜色
				byte *pDest = ((byte*)&blendColor);
				DWORD backColor = pDC->GetPixel({ (LONG)lPos + 1,i });//原背景色
				byte *pBack = (byte*)&backColor;
				byte *pSrc = (byte*)&color;//线条颜色

				float alpha = lBias;
				float alpha2 = 1 - alpha;
				pDest[2] = (byte)(pSrc[2] * alpha + pBack[2] * alpha2);
				pDest[1] = (byte)(pSrc[1] * alpha + pBack[1] * alpha2);
				pDest[0] = (byte)(pSrc[0] * alpha + pBack[0] * alpha2);
				pDC->SetPixel({ (LONG)lPos + 1,i }, blendColor);

				backColor = pDC->GetPixel({ (LONG)lPos,i });
				pBack = (byte*)&backColor;
				alpha = 1 - alpha;
				alpha2 = 1 - alpha;
				pDest[2] = (byte)(pSrc[2] * alpha + pBack[2] * alpha2);
				pDest[1] = (byte)(pSrc[1] * alpha + pBack[1] * alpha2);
				pDest[0] = (byte)(pSrc[0] * alpha + pBack[0] * alpha2);
				pDC->SetPixel({ (LONG)lPos,i }, blendColor);
			}
		}
		else//偏水平直线
		{
			float lPos = lstart*param1 + param2;
			for (int i = lstart; i != lend; i += delta, lPos += param1*delta)
			{
				float lBias = lPos - (int)lPos;
				/*if (lBias < 0)
				lBias += 1;*/
				DWORD blendColor = 0;
				byte *pDest = ((byte*)&blendColor);
				DWORD backColor = pDC->GetPixel({ i,(LONG)lPos });
				byte *pBack = (byte*)&backColor;
				byte *pSrc = (byte*)&color;

				float alpha = (1 - lBias);
				float alpha2 = 1 - alpha;
				pDest[2] = (byte)(pSrc[2] * alpha + pBack[2] * alpha2);
				pDest[1] = (byte)(pSrc[1] * alpha + pBack[1] * alpha2);
				pDest[0] = (byte)(pSrc[0] * alpha + pBack[0] * alpha2);
				pDC->SetPixel({ i,(LONG)lPos }, blendColor);

				backColor = pDC->GetPixel({ i,(LONG)lPos + 1 });
				pBack = (byte*)&backColor;
				alpha = 1 - alpha;
				alpha2 = 1 - alpha;
				pDest[2] = (byte)(pSrc[2] * alpha + pBack[2] * alpha2);
				pDest[1] = (byte)(pSrc[1] * alpha + pBack[1] * alpha2);
				pDest[0] = (byte)(pSrc[0] * alpha + pBack[0] * alpha2);
				pDC->SetPixel({ i,(LONG)lPos + 1 }, blendColor);
			}
		}
	}

	return true;
}
inline bool SmoothLine2(CDC * pDC, const POINT & start, const POINT & end, const DWORD & color)
{
	if (pDC == NULL)
		return false;

	// smooth part
	if ((start.x != end.x) || (start.y != end.y))
	{
		POINT ps, pe;
		ASIGNPOINT(ps, start);
		ASIGNPOINT(pe, end);

		//垂直线
		if (end.x == start.x)
		{
			pDC->MoveTo(start);
			pDC->LineTo(end);
			return true;
		}

		float slope = (pe.y - ps.y) / (float)(pe.x - ps.x);
		bool bSlopePositive = slope >= 0;
		float hproj;
		float hspan;
		int delta;
		float param1, param2;
		int lstart, lend;
		if ((pe.y - ps.y > pe.x - ps.x) || (pe.y - ps.y < ps.x - pe.x))
		{
			hproj = abs(0.5f / slope);
			delta = (pe.y > ps.y) ? 1 : -1;
			param1 = 1.0f / slope;
			param2 = ps.x - ps.y / slope;
			lstart = ps.y, lend = pe.y;
		}
		else
		{
			hproj = abs(0.5f * slope);
			delta = (pe.x > ps.x) ? 1 : -1;
			param1 = slope;
			param2 = ps.y - ps.x*slope;
			lstart = ps.x, lend = pe.x;
		}
		hspan = sqrtf(0.25f + hproj*hproj);

		if ((pe.y - ps.y > pe.x - ps.x) || (pe.y - ps.y < ps.x - pe.x))//偏倾斜直线
		{
			float lPos = lstart*param1 + param2;
			for (int i = lstart; i != lend; i += delta, lPos += param1*delta)
			{
				float lBias = lPos - (int)lPos;
				/*if (lBias < 0)
				lBias += 1;*/
				float wstart = lPos - hspan - hproj;
				float wstartbias = DECIMALPART(wstart);
				float wend = lPos + hspan + hproj;
				float wendbias = DECIMALPART(wend);
				int wstartpointx = PIXELOFPOS(wstart);
				int wendpointx = PIXELOFPOS(wend);

				float wstart2 = wstart + 2 * hproj;
				float wstartbias2 = DECIMALPART(wstart2);
				float wend2 = wend - 2 * hproj;
				float wendbias2 = DECIMALPART(wend2);
				int wstartpointx2 = PIXELOFPOS(wstart2);
				int wendpointx2 = PIXELOFPOS(wend2);

				float alpha = 0;
				float alpha2 = 0;

				int curpx = wstartpointx;
				float ws1 = wstart, ws2 = wstart2, we1 = ws1, we2 = ws2;
				bool scross = (wstartpointx != wstartpointx2), ecross = false;
				while (true)
				{
					bool end1 = false, end2 = false;
					if (CEILPIXELBOUND(ws1) < wend2)
					{
						we1 = CEILPIXELBOUND(ws1);
					}
					else
					{
						we1 = wend2;
						end1 = true;
					}
					if (CEILPIXELBOUND(ws2) < wend)
					{
						we2 = CEILPIXELBOUND(ws2);
					}
					else
					{
						we2 = wend;
						end2 = true;
					}
					ecross = end1 && (!end2);

					if (!ecross && !scross)
					{
						alpha = 1.0f;
					}
					else if (!ecross)
					{
						;
					}
					alpha2 = 1 - alpha;

					POINT dp = { curpx,i };
					DWORD blendColor = 0;//混合颜色
					byte *pDest = ((byte*)&blendColor);
					DWORD backColor = pDC->GetPixel(dp);//原背景色
					byte *pBack = (byte*)&backColor;
					byte *pSrc = (byte*)&color;//线条颜色
					if (wstartpointx != wstartpointx2)
					{
						alpha = 1 - DECIMALPART(wstart + 0.5f);
						alpha *= 0.5f*alpha / (alpha + DECIMALPART(wstart2 + 0.5f));
					}
					else
					{
						alpha = 1 - DECIMALPART(wstart + 0.5f) - hproj;
					}
					alpha2 = 1 - alpha;

					pDest[2] = (byte)(pSrc[2] * alpha + pBack[2] * alpha2);
					pDest[1] = (byte)(pSrc[1] * alpha + pBack[1] * alpha2);
					pDest[0] = (byte)(pSrc[0] * alpha + pBack[0] * alpha2);
					pDC->SetPixel(dp, blendColor);
				}
			}
		}
		else
		{

		}
	}

	return true;
}

bool AlphaBlendLine(CDC * pDC, const POINT & start, int len, const DWORD & color, byte alpha, bool bVertical = true)
{
	if (pDC == NULL || len <= 0)
		return false;

	if (bVertical)
	{
		for (int i = start.y; i < start.y + len; i++)
		{
			byte *pSrc = (byte*)&color;
			DWORD blendColor = 0;
			byte *pDest = ((byte*)&blendColor);
			DWORD backColor = pDC->GetPixel({ start.x,i });
			byte *pBack = (byte*)&backColor;
			byte alpha2 = 1 - alpha;
			pDest[2] = (byte)((pSrc[2] * alpha + pBack[2] * alpha2) / 255.0f);
			pDest[1] = (byte)((pSrc[1] * alpha + pBack[1] * alpha2) / 255.0f);
			pDest[0] = (byte)((pSrc[0] * alpha + pBack[0] * alpha2) / 255.0f);
			pDC->SetPixel({ start.x,i }, blendColor);
		}
	}
	else
	{
		for (int i = start.x; i < start.x + len; i++)
		{
			byte *pSrc = (byte*)&color;
			DWORD blendColor = 0;
			byte *pDest = ((byte*)&blendColor);
			DWORD backColor = pDC->GetPixel({ i,start.y });
			byte *pBack = (byte*)&backColor;
			byte alpha2 = 1 - alpha;
			pDest[2] = (byte)(pSrc[2] * alpha + pBack[2] * alpha2);
			pDest[1] = (byte)(pSrc[1] * alpha + pBack[1] * alpha2);
			pDest[0] = (byte)(pSrc[0] * alpha + pBack[0] * alpha2);
			pDC->SetPixel({ i,start.y }, blendColor);
		}
	}

	return true;
}
#endif // USE_GDIGUI

void CGUIControl::UpdateRects()
{
	SetRect(&rcBoundingBox, posX + dx, posY + dy, posX + dx + width, posY + dy + height);
	SetRect(&rcText, posX + dx, posY + dy, posX + dx + width, posY + dy + height);
}

CGUIControl::CGUIControl()
{
	type = GUI_CONTROL_NULL;
	ID = 0;

	color = 0x00000000;
	dockMode = GUI_WINDOCK_NORMAL;
	dockX = 0;
	dockY = 0;
	dx = 0;
	dy = 0;
	width = 0;
	height = 0;
	posX = 0;
	posY = 0;

	inputPos = 0;
	strText = NULL;
	textColor = COLOR_WHITE;
	UpdateRects();

	bDisabled = false;
	bVisible = true;

	state = GUI_STATE_OUT;

	displayEvent = GUI_EVENT_NULL;
	lastTick = { 0 };
	alpha = 255;
	overdx = 0;
	overdy = 0;
	downdx = 0;
	downdy = 0;
}

CGUIControl::~CGUIControl()
{
	Release();
}

bool CGUIControl::Release()
{
	SAFE_DELETE_LIST(strText);

	return false;
}

byte CGUIControl::GetType() const
{
	return type;
}

int CGUIControl::GetID() const
{
	return ID;
}

void CGUIControl::SetID(int ID)
{
	this->ID = ID;
}

bool CGUIControl::SetText(WCHAR * str)
{
	if (str == NULL)
	{
		return false;
	}

	mywcscpy(&strText, str);
	inputPos = wcslen(str);
	return true;
}

void CGUIControl::SetLocation(int x, int y)
{
	posX = x; posY = y; UpdateRects();
}

void CGUIControl::SetSize(int width, int height)
{
	this->width = width; this->height = height; UpdateRects();
}

void CGUIControl::SetFormat(UINT nFormat)
{
	textFormat = nFormat;
}

void CGUIControl::KillFocus()
{
	POINT cursor;
	GetCursorPos(&cursor);

	Translation(0 - dx, 0 - dy);

	if (PtInRect(&rcBoundingBox, cursor))
	{
		state = GUI_STATE_OVER;
	}
	else
	{
		state = GUI_STATE_OUT;
	}

	displayEvent = GUI_EVENT_NULL;
	alpha = 0;
}

void CGUIControl::Translation(int dx, int dy)
{
	this->dx += dx;
	this->dy += dy;

	UpdateRects();
}

void CGUIControl::Dock(UINT * pbufw, UINT * pbufh)
{
	UINT bufw = SAFE_POINTERVALUE_0(pbufw);
	UINT bufh = SAFE_POINTERVALUE_0(pbufh);

	switch (dockMode)
	{
	case GUI_WINDOCK_NORMAL:
		posX = ROUNDF(dockX);
		posY = ROUNDF(dockY);
		break;
	case GUI_WINDOCK_RIGHT:
		posX = ROUNDF((float)bufw - width - dockX);
		posY = ROUNDF(dockY);
		break;
	case GUI_WINDOCK_BOTTOM:
		posX = ROUNDF(dockX);
		posY = ROUNDF((float)bufh - height - dockY);
		break;
	case GUI_WINDOCK_BOTTOMRIGHT:
		posX = ROUNDF((float)bufw - width - dockX);
		posY = ROUNDF((float)bufh - height - dockY);
		break;
	case GUI_WINDOCK_BOTTOMHSPAN:
		width = (int)bufw;
		posX = 0;
		posY = ROUNDF((float)bufh - height - dockY);
		break;
	case GUI_WINDOCK_FULLSCREEN:
		width = (int)bufw;
		height = (int)bufh;
		posX = 0;
		posY = 0;
		break;
	case GUI_WINDOCK_SCALE:
		width = ROUNDF(bufw*dockW);
		height = ROUNDF(bufh*dockH);
		posX = ROUNDF(dockX*bufw);
		posY = ROUNDF(dockY*bufh);
		break;
	}
	UpdateRects();
}

bool CGUIControl::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (bDisabled || !bVisible)
		return false;

	switch (uMsg)
	{
	case WM_KEYDOWN:
	case WM_IME_CHAR:
		if (wParam == '\b')
		{
			if (inputPos > 0)
			{
				strText[--inputPos] = 0;
			}
		}
		else if (wParam == '\r')
		{
			return true;
		}
		else if (inputPos < GUI_DEFAULTTEXTLEN)
		{
			if (uMsg == WM_IME_CHAR)
			{
				strText[inputPos++] = wParam;
				strText[inputPos] = 0;
			}
			else
			{
				if (wParam >= 'A' && wParam <= 'Z'
					|| wParam >= 'a' && wParam <= 'z'
					|| wParam >= '0' && wParam <= '9'
					|| wParam == ' ')
				{
					if (ISLOWERCHAR(wParam) && CAPS_STATUS_ON)
						TOUPPERCHAR_UNSAFE(wParam)
					else if (ISUPPERCHAR(wParam) && !CAPS_STATUS_ON)
						TOLOWERCHAR_UNSAFE(wParam)

						strText[inputPos++] = wParam;
					strText[inputPos] = 0;
				}
				else if (wParam == 190)
				{
					strText[inputPos++] = '.';
					strText[inputPos] = 0;
				}
			}
		}
	}
	return false;
}

byte CGUIControl::HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
{
	if (bDisabled || !bVisible)
		return GUI_EVENT_NULL;

	byte theevent = GUI_EVENT_NULL;
	GUI_CONTROL_STATE newstate = state;
	RECT rcLiveBox = rcBoundingBox;
	OffsetRect(&rcLiveBox, -dx, -dy);
	bool inside = PtInRect(&rcLiveBox, pt) > 0;

	switch (uMsg) {
	case WM_LBUTTONDOWN:
		if (inside/* && state != GUI_STATE_DOWN*/)
		{
			theevent = GUI_EVENT_LBUTTONDOWN;
			newstate = GUI_STATE_DOWN;
		}
		break;
	case WM_LBUTTONUP:
		if (state == GUI_STATE_DOWN)
		{
			if (inside)
			{
				theevent = GUI_EVENT_LBUTTONUP;
				newstate = GUI_STATE_OVER;
			}
			else
			{
				theevent = GUI_EVENT_LBUTTONOUTUP;
				newstate = GUI_STATE_OUT;
			}
		}
		break;
	case WM_MOUSEMOVE:
		// 鼠标事件处理
		if (inside)
		{
			if (state == GUI_STATE_OUT)
			{
				theevent = GUI_EVENT_MOUSEENTER;
				newstate = GUI_STATE_OVER;
			}
			else
			{
				theevent = GUI_EVENT_MOUSEMOVE;
			}
		}
		else
		{
			if (state == GUI_STATE_OVER)
			{
				theevent = GUI_EVENT_MOUSELEAVE;
				newstate = GUI_STATE_OUT;
			}
		}
		break;
	case WM_MOUSEWHEEL:
		theevent = GUI_EVENT_MOUSEWHEEL;
		break;
	case WM_RBUTTONDOWN:
		if (inside)
			theevent = GUI_EVENT_RBUTTONDOWN;
		break;
	case WM_RBUTTONUP:
		if (inside)
			theevent = GUI_EVENT_RBUTTONUP;
		break;
	}

	// 状态转移
	state = newstate;

	return theevent;
}

void CGUIControl::Invalidate()
{
	if (displayEvent == GUI_EVENT_NULL)
		displayEvent = GUI_EVENT_REFRESH;
}


#ifdef USE_D3DGUI
D3DGUIControl::D3DGUIControl()
{
	font = NULL;

	model = NULL;

	tex = NULL;
	overdx = -1;
	overdy = -2;
	downdx = 1;
	downdy = 1;
}

D3DGUIControl::~D3DGUIControl()
{
	Release();// 崩溃ERROR!

	CGUIControl::~CGUIControl();
}

bool D3DGUIControl::Release()
{
	SAFE_RELEASE(font);

	SAFE_RELEASE(model);

	SAFE_RELEASE(tex);

	CGUIControl::Release();

	return true;
}

void D3DGUIControl::Translation(int dx, int dy)
{
	CGUIControl::Translation(dx, dy);

	RefreshVertexBuffer();
	//if (!model) 
	//	return;

	//D3DVERTEXBUFFER_DESC vbd;
	//model->GetDesc(&vbd);

	//void *ptr;
	//if (FAILED(model->Lock(0, vbd.Size, (void**)&ptr, 0)))
	//	return;
	//*(float*)((byte*)ptr) += dx;//改坐标
	//*(float*)((byte*)ptr + sizeof(float)) += dy;
	//*(float*)((byte*)ptr + sizeof(D3DGUIVertex)) += dx;
	//*(float*)((byte*)ptr + sizeof(D3DGUIVertex) + sizeof(float)) += dy;
	//*(float*)((byte*)ptr + 2 * sizeof(D3DGUIVertex)) += dx;
	//*(float*)((byte*)ptr + 2 * sizeof(D3DGUIVertex) + sizeof(float)) += dy;
	//*(float*)((byte*)ptr + 3 * sizeof(D3DGUIVertex)) += dx;
	//*(float*)((byte*)ptr + 3 * sizeof(D3DGUIVertex) + sizeof(float)) += dy;
	//model->Unlock();
}

void D3DGUIControl::KillFocus()
{
	CGUIControl::KillFocus();

	//Translation(0 - displaydx, 0 - displaydy);
}

bool D3DGUIControl::RefreshVertexBuffer()
{
	if (model)
	{
		D3DGUIVertex obj[] =
		{
			{ (float)width + posX + dx, (float)posY + dy, 0.0f, 1.0f, color, 1.0f, 0.0f },
			{ (float)width + posX + dx, (float)height + posY + dy, 0.0f, 1.0f, color, 1.0f, 1.0f },
			{ (float)posX + dx, (float)posY + dy, 0.0f, 1.0f, color, 0.0f, 0.0f },
			{ (float)posX + dx, (float)height + posY + dy, 0.0f, 1.0f, color, 0.0f, 1.0f },
		};
		// Fill the vertex buffer.
		void *ptr;
		if (FAILED(model->Lock(0, sizeof(obj), (void**)&ptr, 0)))
		{
			return false;
		}
		memcpy(ptr, obj, sizeof(obj));
		model->Unlock();

		return true;
	}
	return false;
}

bool D3DGUIControl::Render(LPDIRECT3DDEVICE9 dev)
{
	if (!dev || !bVisible)
		return false;

	return true;
}

CD3DGUISystem::CD3DGUISystem()
{
	VarInit();
}

CD3DGUISystem::CD3DGUISystem(LPDIRECT3DDEVICE9 dev)
{
	VarInit();
	device = dev;

	if (device)
	{
		HRESULT hr = D3DXCreateFontW(
			device,
			17, 0, FW_NORMAL, 1, FALSE,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLEARTYPE_NATURAL_QUALITY,	//CLEARTYPE_NATURAL_QUALITY, DEFAULT_QUALITY
			DEFAULT_PITCH | FF_DONTCARE,
			L"Tw Cen MT",
			&fontDefault);
	}
}

CD3DGUISystem::~CD3DGUISystem()
{
	Shutdown();
}

bool CD3DGUISystem::Bind(D3DWnd * pd3dwnd)
{
	if (pd3dwnd)
	{
		pD3DWnd = pd3dwnd;
		pBufferW = pd3dwnd->GetPBufferWidth();
		pBufferH = pd3dwnd->GetPBufferHeight();

		return true;
	}
	else
		return false;
}

void CD3DGUISystem::Shutdown()
{
	// 字体清除
	for (int i = 0; i < nFonts; i++)
	{
		SAFE_RELEASE(fonts[i]);
	}
	SAFE_DELETE_LIST(fonts);
	nFonts = 0;

	// 背景控件清除
	SAFE_RELEASEDELETE(pBackdrop);

	for (int i = 0; i < controls.GetSize(); i++)
	{
		D3DGUIControl* pControl = controls.GetAt(i);
		SAFE_DELETE(pControl);
	}

	controls.RemoveAll();
	nControls = 0;
}

void CD3DGUISystem::OnLostDevice()
{
	for (int i = 0; i < nControls; i++)
	{
		// 字体
		fontDefault->OnLostDevice();
		for (int i = 0; i < nFonts; i++)
		{
			if (fonts[i])
			{
				fonts[i]->OnLostDevice();
			}
		}

		// Take action depending on what type it is.
		switch (controls[i]->GetType())
		{
		case GUI_CONTROL_STATIC:
			break;
		case GUI_CONTROL_BUTTON:
			break;
		}
	}
}

void CD3DGUISystem::OnResetDevice()
{
	for (int i = 0; i < nControls; i++)
	{
		// 字体
		fontDefault->OnResetDevice();
		for (int i = 0; i < nFonts; i++)
		{
			if (fonts[i])
			{
				fonts[i]->OnResetDevice();
			}
		}

		// 背景重置
		if (bUseBackdrop && pBackdrop)
		{
			pBackdrop->Dock(pBufferW, pBufferH);
			pBackdrop->RefreshVertexBuffer();//刷新模型
		}

		// 其他控件重置
		controls[i]->Dock(pBufferW, pBufferH);// 控件重新停靠
		switch (controls[i]->GetType())
		{
		case GUI_CONTROL_STATIC:
			break;
		case GUI_CONTROL_BUTTON:
		case GUI_CONTROL_EDIT:
			//case GUI_CONTROL_BACKDROP:
			controls[i]->RefreshVertexBuffer();//刷新模型
			break;
		}
	}
}

bool CD3DGUISystem::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		HandleMouse(uMsg, { LOWORD(lParam),HIWORD(lParam) }, wParam, lParam);
		break;
	case WM_KEYDOWN:
		HandleKeyboard(uMsg, wParam, lParam);
		break;
	}
	return true;
}

bool CD3DGUISystem::AddDXFont(WCHAR *fontName, int *fontID
	, INT Height, INT Width, INT Weight
	, DWORD Quality
	, UINT MipLevels, bool Italics
	, DWORD Charset, DWORD OutputPrecision
	, DWORD PitchAndFamily)
{
	if (!device) return false;

	if (!fonts)
	{
		fonts = new LPD3DXFONT[1];
		if (!fonts) return false;
	}
	else
	{
		LPD3DXFONT *temp;
		temp = new LPD3DXFONT[nFonts + 1];
		if (!temp) return false;
		memcpy(temp, fonts, sizeof(LPD3DXFONT) * nFonts);
		delete[] fonts;
		fonts = temp;
	}

	if (FAILED(D3DXCreateFontW(device,
		Height, Width, Weight, MipLevels, Italics,
		Charset, OutputPrecision, Quality,
		PitchAndFamily, fontName,
		&fonts[nFonts])))
		return false;

	if (!fonts[nFonts])
		return false;

	if (fontID)
		*fontID = nFonts + 1;

	nFonts++;

	return true;
}

void CD3DGUISystem::VarInit()
{
	CGUIControl::GetFrequency();//控件获取系统频率

	device = NULL;
	pD3DWnd = NULL;
	pBufferW = NULL;
	pBufferH = NULL;

	fontDefault = NULL;
	fonts = NULL;
	pBackdrop = NULL;
	bUseBackdrop = true;

	nFonts = 0;
	nControls = 0;

	pFocusControl = NULL;
	bBlock = false;

	pCallbackEvent = NULL;
}

bool CD3DGUISystem::AddControl(D3DGUIControl * pControl)
{
	HRESULT hr = S_OK;

	hr = controls.Add(pControl);
	if (FAILED(hr))
	{
		return false;
	}
	nControls++;

	return true;
}

bool CD3DGUISystem::AddBackdrop(WCHAR *TexFileName, float x, float y, float width, float height, GUI_WINDOCK_MODE dock)
{
	NULL_RETURN_FALSE(device);

	if (pBackdrop)
		SAFE_RELEASEDELETE(pBackdrop);
	pBackdrop = new D3DGUIBack;
	bool succeed = true;

	pBackdrop->color = COLOR_WHITE;// 创建vertexbuffer用
	pBackdrop->dockMode = dock;
	pBackdrop->dockX = x;
	pBackdrop->dockY = y;
	pBackdrop->dockW = width;
	pBackdrop->dockH = height;
	pBackdrop->width = ROUNDF(width);
	pBackdrop->height = ROUNDF(height);
	pBackdrop->Dock(pBufferW, pBufferH);

	pBackdrop->state = GUI_STATE_OUT;

	D3DXCreateTextureFromFile(device, TexFileName, &pBackdrop->tex);
	if (FAILED(device->CreateVertexBuffer(sizeof(D3DGUIVertex) * 4, 0, D3DFVF_GUI,
		D3DPOOL_SYSTEMMEM, &pBackdrop->model, NULL)))
		succeed = false;
	pBackdrop->RefreshVertexBuffer();

	if (succeed)
	{
		return true;
	}
	else
	{
		SAFE_DELETE(pBackdrop);
		return false;
	}
}

bool CD3DGUISystem::AddStatic(int ID, float x, float y, float width, float height, WCHAR *text, DWORD color, int fontID, GUI_WINDOCK_MODE dock)
{
	NULL_RETURN_FALSE(device);

	D3DGUIStatic *pStatic = new D3DGUIStatic;

	if (pStatic == NULL)
		return false;

	if (!AddControl(pStatic))
	{
		SAFE_DELETE(pStatic);
		return false;
	}

	pStatic->SetID(ID);

	pStatic->color = color;
	pStatic->dockMode = dock;
	pStatic->dockX = x;
	pStatic->dockY = y;
	pStatic->dockW = width;
	pStatic->dockH = height;
	pStatic->SetSize(ROUNDF(width), ROUNDF(height));
	pStatic->Dock(pBufferW, pBufferH);

	pStatic->SetText(text);
	pStatic->textColor = color;
	pStatic->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : fontDefault;

	pStatic->state = GUI_STATE_OUT;
}

bool CD3DGUISystem::AddButton(int ID, float x, float y, float width, float height, WCHAR *text, DWORD color, int fontID, GUI_WINDOCK_MODE dock, WCHAR *up, WCHAR *over, WCHAR *down)
{
	NULL_RETURN_FALSE(device);

	D3DGUIButton *pButton = new D3DGUIButton;

	if (pButton == NULL)
		return false;

	if (!AddControl(pButton))
	{
		SAFE_DELETE(pButton);
		return false;
	}

	pButton->SetID(ID);

	pButton->color = COLOR_WHITE;// 创建vertexbuffer用
	pButton->dockMode = dock;
	pButton->dockX = x;
	pButton->dockY = y;
	pButton->dockW = width;
	pButton->dockH = height;
	pButton->SetSize(ROUNDF(width), ROUNDF(height));
	pButton->Dock(pBufferW, pBufferH);

	pButton->SetText(text);
	pButton->textColor = color;
	pButton->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : fontDefault;

	pButton->overdx = -1;
	pButton->overdy = -1;
	pButton->downdx = 1;
	pButton->downdy = 1;
	pButton->state = GUI_STATE_OUT;
	pButton->alpha = 0;
	pButton->displayEvent = GUI_EVENT_NULL;

	D3DXCreateTextureFromFile(device, up, &pButton->tex);
	D3DXCreateTextureFromFile(device, over, &pButton->texOver);
	D3DXCreateTextureFromFile(device, down, &pButton->texDown);

	// Create the vertex buffer.
	if (FAILED(device->CreateVertexBuffer(sizeof(D3DGUIVertex) * 4, 0,
		D3DFVF_GUI, D3DPOOL_MANAGED, &pButton->model, NULL)))
	{
		SAFE_DELETE(pButton);
		return false;
	}
	pButton->RefreshVertexBuffer();

	return true;
}

bool CD3DGUISystem::AddEdit(int ID, float x, float y, float width, float height, DWORD color, DWORD txtcolor, int fontID, GUI_WINDOCK_MODE dock)
{
	NULL_RETURN_FALSE(device);

	D3DGUIEdit *pEdit = new D3DGUIEdit;

	if (pEdit == NULL)
		return false;

	if (!AddControl(pEdit))
	{
		SAFE_DELETE(pEdit);
		return false;
	}

	pEdit->SetID(ID);

	pEdit->color = color;
	pEdit->dockMode = dock;
	pEdit->dockX = x;
	pEdit->dockY = y;
	pEdit->dockW = width;
	pEdit->dockH = height;
	pEdit->SetSize(ROUNDF(width), ROUNDF(height));
	pEdit->Dock(pBufferW, pBufferH);
	pEdit->dx = 0;
	pEdit->dy = 0;

	pEdit->strText = new WCHAR[GUI_DEFAULTTEXTLEN + 1];
	pEdit->strText[0] = L'\0';
	pEdit->inputPos = 0;
	pEdit->textColor = txtcolor;
	pEdit->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : fontDefault;

	pEdit->state = GUI_STATE_OUT;
	pEdit->alpha = 0;
	pEdit->displayEvent = GUI_EVENT_NULL;

	// Create the vertex buffer.
	if (FAILED(device->CreateVertexBuffer(sizeof(D3DGUIVertex) * 4, 0,
		D3DFVF_GUI, D3DPOOL_SYSTEMMEM, &pEdit->model, NULL)))
	{
		SAFE_DELETE(pEdit);
		return false;
	}
	pEdit->RefreshVertexBuffer();

	return true;
}

bool CD3DGUISystem::DropControl(int ID)
{
	for (int i = 0; i < controls.GetSize(); i++)
	{
		D3DGUIControl* pControl = controls.GetAt(i);
		if (pControl->GetID() == ID)
		{
			if (pFocusControl == pControl)
			{
				// Clean focus first
				ClearFocus();
			}

			SAFE_DELETE(pControl);
			controls.Remove(i);

			return true;
		}
	}
	/*for (int i = 0; i < nControls; i++)
	{
	if (controls[i]->ID == ID)
	{
	delete controls[i];
	for (int j = i; j < nControls - 1; j++)
	{
	controls[j] = controls[j + 1];
	}
	D3DGUIControl **temp = new D3DGUIControl*[nControls - 1];
	for (int j = 0; j < nControls - 1; j++)
	temp[j] = controls[j];
	delete[] controls;
	controls = temp;
	nControls--;
	}
	}*/
	return false;
}

int CD3DGUISystem::GetState(int ID) const
{
	for (int i = 0; i < nControls; i++)
	{
		if (controls[i]->GetID() == ID)
		{
			return controls[i]->state;
		}
	}
	return -1;
}

bool CD3DGUISystem::SetControlText(int ID, WCHAR * text)
{
	NULL_RETURN_FALSE(text);

	for (int i = 0; i < nControls; i++)
	{
		if (controls[i]->GetID() == ID)
		{
			controls[i]->SetText(text);

			controls[i]->inputPos = wcslen(text);

			return true;
		}
	}
	return false;
}

bool CD3DGUISystem::SetControlVisible(int ID, bool bVisible)
{
	for (int i = 0; i < nControls; i++)
	{
		if (controls[i]->GetID() == ID)
		{
			controls[i]->SetVisible(bVisible);

			if (ISFOCUS(i) && !bVisible)
			{
				BLOCK_OFF;
				FOCUS_OFF;
			}

			return true;
		}
	}
	return false;
}

bool CD3DGUISystem::SetControlEnabled(int ID, bool bEnabled)
{
	for (int i = 0; i < nControls; i++)
	{
		if (controls[i]->GetID() == ID)
		{
			controls[i]->SetEnabled(bEnabled);

			if (ISFOCUS(i) && !bEnabled)
			{
				BLOCK_OFF;
				FOCUS_OFF;
			}

			return true;
		}
	}
}

LPD3DXFONT CD3DGUISystem::GetFont(int ID) const
{
	if (ID < 0 || ID >= nFonts) return NULL;
	return fonts[ID];
}

bool CD3DGUISystem::SetFocus(int ID)
{
	for (int i = 0; i < nControls; i++)
	{
		if (controls[i]->GetID() == ID)
		{
			ClearFocus();
			FOCUS_CONTROL(i);

			return true;
		}
	}

	return false;
}

bool CD3DGUISystem::ClearFocus()
{
	if (pFocusControl)
	{
		bBlock = false;
		pFocusControl->KillFocus();

		return true;
	}

	return false;
}

void CD3DGUISystem::SetCallbackEvent(LPGUIEVENTCALLBACK pevent)
{
	pCallbackEvent = pevent;
}

void CD3DGUISystem::HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
{
	if (!pD3DWnd)
		return;

	bool controldown = false;//是否有控件按下
	for (int i = 0; i < nControls; i++)
	{
		if (!bBlock || ISFOCUS(i))//不阻塞或者第i个控件就是焦点控件
		{
			if (!controls[i]->bDisabled && controls[i]->bVisible)// 控件处于使能状态
			{
				int event = controls[i]->HandleMouse(uMsg, pt, wParam, lParam);

				// 控件系统调度
				if (event == GUI_EVENT_LBUTTONDOWN)//按下的按钮阻塞其他按钮
				{
					if (controls[i]->GetType() == GUI_CONTROL_BUTTON)
					{
						controldown = true;
						BLOCK_ON_CONTROL(i)
					}
					else if (controls[i]->GetType() == GUI_CONTROL_EDIT)
					{
						controldown = true;
						FOCUS_CONTROL(i)
					}
				}
				else if (event == GUI_EVENT_LBUTTONOUTUP || event == GUI_EVENT_LBUTTONUP)
					BLOCK_OFF;

				if (event && pCallbackEvent)//回调
				{
					pCallbackEvent(controls[i]->GetID(), event, 0);
				}
			}
		}
	}
}

void CD3DGUISystem::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < nControls; i++)
	{
		if (controls[i]->GetType() == GUI_CONTROL_EDIT && ISFOCUS(i))
		{
			if (!controls[i]->bDisabled && controls[i]->bVisible)
			{
				bool cmd = controls[i]->HandleKeyboard(uMsg, wParam, lParam);

				if (cmd && pCallbackEvent)
				{
					pCallbackEvent(controls[i]->GetID(), GUI_EVENT_CMD, (LPARAM)controls[i]->strText);
					controls[i]->SetText(L"");
				}
			}
		}
	}
}

void CD3DGUISystem::RenderBack()
{
	if (device)
	{
		if (bUseBackdrop && pBackdrop)
		{
			pBackdrop->Render(device);
		}
	}
}

void CD3DGUISystem::Render()
{
	if (device)
	{
		// 逐个绘制控件
		for (int i = 0; i < nControls; i++)
		{
			controls[i]->Render(device);
		}
	}
}

D3DGUIStatic::D3DGUIStatic()
{
	type = GUI_CONTROL_STATIC;
}

bool D3DGUIStatic::Render(LPDIRECT3DDEVICE9 dev)
{
	if (!dev || !bVisible)
		return false;

	// 计时
	//LARGE_INTEGER curTime = { 0 };
	//QueryPerformanceCounter(&curTime);

	if (font && strText)
		font->DrawTextW(NULL, strText, -1, &rcText, TEXTFORMAT_LEFT, color);

	//lastTick.QuadPart = curTime.QuadPart;

	return true;
}

D3DGUIButton::D3DGUIButton()
{
	type = GUI_CONTROL_BUTTON;

	texOver = NULL;
	texDown = NULL;
}

D3DGUIButton::~D3DGUIButton()
{
	Release();
}

bool D3DGUIButton::Release()
{
	SAFE_RELEASE(texOver);
	SAFE_RELEASE(texDown);

	D3DGUIControl::Release();

	return true;
}

byte D3DGUIButton::HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
{
	byte event = CGUIControl::HandleMouse(uMsg, pt, wParam, lParam);
	if (event)
	{
		int ddx = 0, ddy = 0;
		if (state == GUI_STATE_OUT)
		{
			ddx = 0;
			ddy = 0;
		}
		else if (state == GUI_STATE_OVER)
		{
			ddx = overdx;
			ddy = overdy;
		}
		else if (state == GUI_STATE_DOWN)
		{
			ddx = downdx;
			ddy = downdy;
		}
		Translation(ddx - dx, ddy - dy);
	}

	// 显示
	if (event)
	{
		//存入事件
		displayEvent = event;
		QueryPerformanceCounter(&lastTick);

		//事件起始设定
		if (displayEvent == GUI_EVENT_MOUSELEAVE)
			alpha = 255;
		else if (displayEvent == GUI_EVENT_LBUTTONUP)
			alpha = 0;
	}

	return event;
}

bool D3DGUIButton::Render(LPDIRECT3DDEVICE9 dev)
{
	if (!dev || !bVisible)
		return false;

	// 计时
	LARGE_INTEGER tick = { 0 };
	QueryPerformanceCounter(&tick);
	const float speed = ALPHASPEED_DEFAULT;

	if (!model)
		return false;

	// 底层绘制
	dev->SetTexture(0, tex);
	D3DGUI_RENDER_VBUFFER(dev, model);

	// 第二图层绘制
	// 不用多线程处理渐变
	float dAplha = 1000.0f*(tick.QuadPart - lastTick.QuadPart) / frequency.QuadPart *speed;
	if (displayEvent == GUI_EVENT_MOUSEENTER || displayEvent == GUI_EVENT_MOUSEMOVE)
	{
		if (alpha != 255)
			alpha += min(255 - alpha, dAplha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_MOUSELEAVE)
	{
		if (alpha)
			alpha -= min(alpha, dAplha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_LBUTTONOUTUP)
	{
		if (alpha)
			alpha -= min(alpha, dAplha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_LBUTTONUP)
	{
		if (alpha != 255)
			alpha += min(255 - alpha, dAplha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_LBUTTONDOWN)
	{
		alpha = 255;
	}

	// 设置二层纹理
	if (displayEvent == GUI_EVENT_LBUTTONOUTUP
		|| displayEvent == GUI_EVENT_LBUTTONDOWN)
		dev->SetTexture(0, texDown);
	else
		dev->SetTexture(0, texOver);

	// 绘制第二图层
	ChangeAlpha(model, ROUNDF_BYTE(alpha));//设置实时透明度
	D3DGUI_RENDER_VBUFFER(dev, model);
	ChangeAlpha(model, 255);

	// 按钮文本绘制
	if (strText && font)
	{
		rcText = { (LONG)(posX + dx), LONG(posY + dy - 1) //-1修正
			, LONG(posX + dx + width)
			, LONG(posY + dy + height) };
		//阴影绘制
		/*fontDefault->DrawTextW(NULL, pCnt->strText
		, -1, &textregion, TEXTFORMAT_DEFAULT
		, SETALPHA(~pCnt->textcolor, 255));*/
		/*DWORD alphacolor = D3DCOLOR_ARGB(textcolor >> 24
		, ((textcolor >> 16) & 0xFF)*(255 - alpha) / 255
		, ((textcolor >> 8) & 0xFF)*(255 - alpha) / 255
		, (textcolor & 0xFF)*(255 - alpha) / 255);*/
		DWORD alphacolor = D3DCOLOR_ARGB(textColor >> 24
			, (BYTE_PART(textColor >> 16)*ROUNDF_BYTE(255 - alpha) + ROUNDF_BYTE(alpha)*(255 - BYTE_PART(textColor >> 16))) >> 8
			, (BYTE_PART(textColor >> 8)*ROUNDF_BYTE(255 - alpha) + ROUNDF_BYTE(alpha)*(255 - BYTE_PART(textColor >> 8))) >> 8
			, (BYTE_PART(textColor)*ROUNDF_BYTE(255 - alpha) + ROUNDF_BYTE(alpha)*(255 - BYTE_PART(textColor))) >> 8);
		/*textregion.bottom -= 2;
		textregion.right -= 2;*/
		font->DrawTextW(NULL, strText, -1, &rcText, TEXTFORMAT_DEFAULT, alphacolor);
	}
	lastTick.QuadPart = tick.QuadPart;

	return true;
}

D3DGUIEdit::D3DGUIEdit()
{
	type = GUI_CONTROL_EDIT;
}

bool D3DGUIEdit::Render(LPDIRECT3DDEVICE9 dev)
{
	if (!dev || !bVisible)
		return false;

	dev->SetTexture(0, NULL);
	D3DGUI_RENDER_VBUFFER(dev, model);

	if (strText && font)
		font->DrawTextW(NULL, strText, -1, &rcText, TEXTFORMAT_LEFT, textColor);

	return true;
}

D3DGUIBack::D3DGUIBack()
{
	type = GUI_CONTROL_BACKDROP;
}

bool D3DGUIBack::Render(LPDIRECT3DDEVICE9 dev)
{
	if (!dev || !bVisible)
		return false;

	dev->SetTexture(0, tex);
	D3DGUI_RENDER_VBUFFER(dev, model);

	return true;
}
#endif // USE_D3DGUI


#ifdef USE_GDIGUI
GDIDevice::GDIDevice()
{
	pmDB = NULL;
	pdevDC = NULL;
	rcRedraw = { 0,0,0,0 };
}

GDIDevice::~GDIDevice()
{
	DeleteMemDCBMP(pmDB);
	DeleteMemDCBMP(pmDBBack);
}

HRESULT __stdcall GDIDevice::QueryInterface(const IID & riid, void ** ppvObj)
{
	return S_OK;
}

ULONG __stdcall GDIDevice::AddRef()
{
	return 0;
}

ULONG __stdcall GDIDevice::Release()
{
	return 0;
}

HRESULT __stdcall GDIDevice::SetDevice(CDC * pdc)
{
	if (!pdc)
		return E_INVALIDARG;

	pdevDC = pdc;

	if (pmDB)
	{
		DeleteMemDCBMP(pmDB);
		pmDB = NULL;
	}
	pmDB = new memDCBMP;
	MyDrawPrepareOne(pdevDC, pmDB, 0, 0
		, CRect(0, 0, GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN)));

	if (pmDBBack)
	{
		DeleteMemDCBMP(pmDBBack);
		pmDBBack = NULL;
	}
	pmDBBack = new memDCBMP;
	MyDrawPrepareOne(pdevDC, pmDBBack, 0, 0
		, CRect(0, 0, GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN)));

	return S_OK;
}
GDIGUIControl::GDIGUIControl()
{
	textColor = COLORGDI_DARKGREY;//modified from CGUIControl()

	font = NULL;
	pic = NULL;
}

GDIGUIControl::~GDIGUIControl()
{
	Release();
}

bool GDIGUIControl::Release()
{
	SAFE_DELETE(pic);

	return true;
}

bool GDIGUIControl::DisplayCycle(LPGDIDevice dev)
{
	if (displayEvent != GUI_EVENT_NULL)
	{
		// 重绘
		bool res = Render(dev);

		displayEvent = GUI_EVENT_NULL;
		return res;
	}

	return false;
}

bool GDIGUIControl::RenderText(CDC * pDC, UINT nFormat, DWORD *pNewColor)
{
	if (strText && pDC != NULL)
	{
		CFont *pOldFont = (CFont*)pDC->SelectObject(font);
		if (pNewColor)
			pDC->SetTextColor(*pNewColor);
		else
			pDC->SetTextColor(textColor);

		pDC->SetBkMode(TRANSPARENT);
		test = pDC->DrawTextW(strText, &rcText, nFormat);
		pDC->SelectObject(pOldFont);

		return true;
	}
	return false;
}

bool GDIGUIControl::Render(LPGDIDevice dev)
{
	if (!bVisible)
		return false;

	if (!dev || dev->IsDCNull())
	{
		return false;
	}

	return true;
}

GDIGUIStatic::GDIGUIStatic()
{
	type = GUI_CONTROL_STATIC;

	textFormat = TEXTFORMAT_LEFT;
}

bool GDIGUIStatic::Render(LPGDIDevice dev)
{
	if (!bVisible)
		return false;

	if (!dev || dev->IsDCNull())
		return false;

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();
	CDC *pMemDCBack = dev->GetMemDCBack();
	if (pDestDC == NULL || pMemDC == NULL || pMemDCBack == NULL)
		return false;

	//RECT refreshrect = RECT(posX, posY, width, height);
	//pMemDC->FillSolidRect(&refreshrect, COLORGDI_DEFAULT);
	pMemDC->BitBlt(rcBoundingBox.left, rcBoundingBox.top, rcBoundingBox.right, rcBoundingBox.bottom
		, pMemDCBack, rcBoundingBox.left, rcBoundingBox.top, SRCCOPY);

	// 贴图
	if (pic)
		pic->Show(pMemDC, posX, posY, 255, true, pMemDCBack);//测试

															 // 绘制文本
	RenderText(pMemDC, textFormat);

	// 显示到目标
	pDestDC->BitBlt(posX, posY, width, height
		, pMemDC, posX, posY, SRCCOPY);

	return true;
}

GDIGUIBack::GDIGUIBack()
{
	type = GUI_CONTROL_BACKDROP;

	textFormat = TEXTFORMAT_LEFT;
}

bool GDIGUIBack::Render(LPGDIDevice dev)
{
	if (!bVisible)
		return false;

	if (!dev || dev->IsDCNull())
		return false;

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();
	CDC *pMemDCBack = dev->GetMemDCBack();
	if (pDestDC == NULL || pMemDC == NULL || pMemDCBack == NULL)
		return false;

	RECT refreshrect = RECT(posX, posY, width, height);
	pMemDCBack->FillSolidRect(&refreshrect, COLORGDI_DEFAULT);

	// 贴图
	if (pic)
		pic->Show(pMemDCBack, ROUND(posX), ROUND(posY), ROUNDF_BYTE(alpha), false);//测试

																				  // 绘制文本
																				  //RenderText(pMemDC, textFormat);

																				  // 显示到目标
	pMemDC->BitBlt(posX, posY, width, height
		, pMemDCBack, posX, posY, SRCCOPY);

	return true;
}

GDIGUIButton::GDIGUIButton()
{
	type = GUI_CONTROL_BUTTON;

	picon = NULL;
	picdown = NULL;

	textFormat = TEXTFORMAT_DEFAULT;
}

GDIGUIButton::~GDIGUIButton()
{
	Release();
}

bool GDIGUIButton::Release()
{
	SAFE_DELETE(picon);
	SAFE_DELETE(picdown);

	GDIGUIControl::Release();

	return true;
}

byte GDIGUIButton::HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
{
	if (bDisabled || !bVisible)
		return false;

	byte event = CGUIControl::HandleMouse(uMsg, pt, wParam, lParam);

	// 处理按钮偏移
	if (event)
	{
		int ddx = 0, ddy = 0;
		if (state == GUI_STATE_OUT)
		{
			ddx = 0;
			ddy = 0;
		}
		else if (state == GUI_STATE_OVER)
		{
			ddx = overdx;
			ddy = overdy;
		}
		else if (state == GUI_STATE_DOWN)
		{
			ddx = downdx;
			ddy = downdy;
		}
		Translation(ddx - dx, ddy - dy);
	}

	if (event)
	{
		//存入事件
		displayEvent = event;
		QueryPerformanceCounter(&lastTick);

		//事件起始设定
		if (displayEvent == GUI_EVENT_MOUSELEAVE)
			alpha = 255;
		else if (displayEvent == GUI_EVENT_LBUTTONUP)
			alpha = 0;
	}

	return event;
}

bool GDIGUIButton::DisplayCycle(LPGDIDevice dev)
{
	const float speed = ALPHASPEED_DEFAULT;

	LARGE_INTEGER tick = { 0 };
	QueryPerformanceCounter(&tick);
	float dAlpha = 1000.0f*(tick.QuadPart - lastTick.QuadPart) / frequency.QuadPart *speed;
	if (displayEvent == GUI_EVENT_MOUSEENTER || displayEvent == GUI_EVENT_MOUSEMOVE)
	{
		if (alpha != 255)
			alpha += min(255 - alpha, dAlpha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_MOUSELEAVE)
	{
		if (alpha)
			alpha -= min(alpha, dAlpha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_LBUTTONOUTUP)
	{
		if (alpha)
			alpha -= min(alpha, dAlpha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_LBUTTONUP)
	{
		if (alpha != 255)
			alpha += min(255 - alpha, dAlpha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_LBUTTONDOWN)
	{
		alpha = 255;
	}

	lastTick.QuadPart = tick.QuadPart;

	if (displayEvent != GUI_EVENT_NULL)
		return Render(dev);

	return false;
}

bool GDIGUIButton::Render(LPGDIDevice dev)
{
	if (!bVisible)
		return false;

	if (!dev || dev->IsDCNull())
		return false;

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();
	CDC *pMemDCBack = dev->GetMemDCBack();
	if (pDestDC == NULL || pMemDC == NULL || pMemDCBack == NULL)
		return false;

	int basex = ROUND(posX), basey = ROUND(posY);//待刷新区域坐标
	int basew = ROUND(width), baseh = ROUND(height);//待刷新区域尺寸
	memPic *upperLayer = NULL;//二层贴图(特效)

							  // 确定第二层贴图 & 当前贴图实际位置
	if (state == GUI_STATE_OUT)
	{
		if (displayEvent == GUI_EVENT_MOUSELEAVE)
			upperLayer = picon;
		else if (displayEvent == GUI_EVENT_LBUTTONOUTUP)
			upperLayer = picdown;
	}
	else if (state == GUI_STATE_OVER)
		upperLayer = picon;
	else if (state == GUI_STATE_DOWN)
		upperLayer = picdown;

	//确定需要刷新的区域（当前贴图和上一次状态的贴图并集）
	if (displayEvent == GUI_EVENT_MOUSEENTER || displayEvent == GUI_EVENT_MOUSELEAVE)
	{
		if (overdx < 0)
			basex += overdx;
		if (overdy < 0)
			basey += overdy;
		basew += abs(overdx);
		baseh += abs(overdy);
	}
	else if (displayEvent == GUI_EVENT_LBUTTONOUTUP)
	{
		if (downdx < 0)
			basex += downdx;
		if (downdy < 0)
			basey += downdy;
		basew += abs(downdx);
		baseh += abs(downdy);
	}
	else if (displayEvent == GUI_EVENT_LBUTTONDOWN || displayEvent == GUI_EVENT_LBUTTONUP)
	{
		basex += min(min(overdx, downdx), 0);
		basey += min(min(overdy, downdy), 0);
		basew += max(max(abs(overdx), abs(downdx)), abs(overdx - downdx));
		baseh += max(max(abs(overdy), abs(downdy)), abs(overdy - downdy));
	}

	// 清除背景
	//RECT refreshrect = RECT(basex, basey, basew, baseh);
	//pMemDC->FillSolidRect(&refreshrect, COLORGDI_DEFAULT);

	// 绘制两层贴图
	if (pic)
		pic->Show(pMemDC, posX + dx, posY + dy, 255, true, pMemDCBack);
	if (upperLayer)
		upperLayer->Show(pMemDC, posX + dx, posY + dy, ROUNDF_BYTE(alpha), false);

	// 绘制文本
	DWORD alphacolor = RGB(
		(BYTE_PART(textColor >> 16)*ROUNDF_BYTE(255 - alpha) + ROUNDF_BYTE(alpha)*(255 - BYTE_PART(textColor >> 16))) >> 8
		, (BYTE_PART(textColor >> 8)*ROUNDF_BYTE(255 - alpha) + ROUNDF_BYTE(alpha)*(255 - BYTE_PART(textColor >> 8))) >> 8
		, (BYTE_PART(textColor)*ROUNDF_BYTE(255 - alpha) + ROUNDF_BYTE(alpha)*(255 - BYTE_PART(textColor))) >> 8);
	RenderText(pMemDC, textFormat, &alphacolor);

	// 显示到目标
	pDestDC->BitBlt(basex, basey, basew, baseh
		, pMemDC, basex, basey, SRCCOPY);

	return true;
}

void GDIGUITristate::UpdateRects()
{
	GDIGUIControl::UpdateRects();
	SetRect(&rcText, posX, posY - height - 2, posX + width, posY - 2);//tristate文本框在正上方，控件外侧
}

GDIGUITristate::GDIGUITristate()
{
	type = GUI_CONTROL_TRISTATE;

	picon = NULL;
	picoff = NULL;

	tristate = GUI_TRISTATE_NORMAL;

	textFormat = TEXTFORMAT_BOTTOM;
}

GDIGUITristate::~GDIGUITristate()
{
	Release();
}

bool GDIGUITristate::Release()
{
	SAFE_DELETE(picon);
	SAFE_DELETE(picoff);

	GDIGUIControl::Release();

	return true;
}

void GDIGUITristate::SetOn()
{
	tristate = GUI_TRISTATE_ON;
	displayEvent = GUI_EVENT_REFRESH;
}

void GDIGUITristate::SetOff()
{
	tristate = GUI_TRISTATE_OFF;
	displayEvent = GUI_EVENT_REFRESH;
}

void GDIGUITristate::SetNormal()
{
	tristate = GUI_TRISTATE_NORMAL;
	displayEvent = GUI_EVENT_REFRESH;
}

bool GDIGUITristate::Render(LPGDIDevice dev)
{
	if (!bVisible)
		return false;

	if (!dev || dev->IsDCNull())
		return false;

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();
	CDC *pMemDCBack = dev->GetMemDCBack();
	if (pDestDC == NULL || pMemDC == NULL || pMemDCBack == NULL)
		return false;

	memPic *ppic = NULL;

	// 确定贴图
	if (tristate == GUI_TRISTATE_NORMAL)
	{
		ppic = pic;
	}
	else if (tristate == GUI_TRISTATE_ON)
	{
		ppic = picon;
	}
	else if (tristate == GUI_TRISTATE_OFF)
	{
		ppic = picoff;
	}

	//GDIGUITristate实际区域是参数区域向上扩展一倍，refreshrect和BitBlt中作调整！
	// 清除背景
	RECT rcUnion;
	UnionRect(&rcUnion, &rcBoundingBox, &rcText);//控件区域和文本区域并
	pMemDC->BitBlt(rcUnion.left, rcUnion.top, rcUnion.right, rcUnion.bottom
		, pMemDCBack, rcUnion.left, rcUnion.top, SRCCOPY);

	// 绘制贴图
	if (ppic)
		ppic->Show(pMemDC, posX, posY, ROUNDF_BYTE(alpha), true, pMemDCBack);

	// 绘制文本
	RenderText(pMemDC, textFormat);

	// 显示到目标
	pDestDC->BitBlt(rcUnion.left, rcUnion.top, WIDTHOF(rcUnion), HEIGHTOF(rcUnion)
		, pMemDC, rcUnion.left, rcUnion.top, SRCCOPY);

	return true;
}

void GDIGUISwitch::UpdateRects()
{
	GDIGUIControl::UpdateRects();
	SetRect(&rcText, posX + width + 2, posY, posX + 10 * width + 2, posY + height);
}

GDIGUISwitch::GDIGUISwitch()
{
	type = GUI_CONTROL_SWITCH;

	picon = NULL;
	stateOn = false;

	textFormat = TEXTFORMAT_LEFT;
}

GDIGUISwitch::~GDIGUISwitch()
{
	Release();
}

bool GDIGUISwitch::Release()
{
	SAFE_DELETE(picon);
	return true;
}

bool GDIGUISwitch::GetOn()
{
	return stateOn;
}

void GDIGUISwitch::SetOn()
{
	stateOn = true;
	displayEvent = GUI_EVENT_REFRESH;
}

void GDIGUISwitch::SetOff()
{
	stateOn = false;
	displayEvent = GUI_EVENT_REFRESH;
}

bool GDIGUISwitch::Render(LPGDIDevice dev)
{
	if (!bVisible)
		return false;

	if (!dev || dev->IsDCNull())
		return false;

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();
	CDC *pMemDCBack = dev->GetMemDCBack();
	if (pDestDC == NULL || pMemDC == NULL || pMemDCBack == NULL)
		return false;

	memPic *ppic = NULL;

	// 确定贴图
	if (stateOn == false)
	{
		ppic = pic;
	}
	else if (stateOn == true)
	{
		ppic = picon;
	}

	//GDIGUITristate实际区域是参数区域向上扩展一倍，refreshrect和BitBlt中作调整！
	// 清除背景
	RECT rcUnion;
	UnionRect(&rcUnion, &rcBoundingBox, &rcText);//控件区域和文本区域并
												 //pMemDC->FillSolidRect(&rcUnion, COLORGDI_DEFAULT);//清除背景

												 // 绘制贴图
	if (ppic)
		ppic->Show(pMemDC, posX, posY, ROUNDF_BYTE(alpha), true, pMemDCBack);

	// 绘制文本
	RenderText(pMemDC, textFormat);

	// 显示到目标
	pDestDC->BitBlt(rcUnion.left, rcUnion.top, WIDTHOF(rcUnion), HEIGHTOF(rcUnion)
		, pMemDC, rcUnion.left, rcUnion.top, SRCCOPY);

	return true;
}

bool GDIGUIWave::SetEmptyBackground()
{
	if (waveBuf == NULL)
		return false;
	if (waveBuf->pDC == NULL)
		return false;

	// 清除背景
	RECT refreshrect = RECT(0, 0, width, height);
	waveBuf->pDC->FillSolidRect(&refreshrect, color);//清除背景

	CPen pen(PS_SOLID, 2, COLORGDI_LIGHTGREY);
	CPen* pOldPen = (CPen*)waveBuf->pDC->SelectObject(&pen);
	waveBuf->pDC->MoveTo({ 0,(LONG)(height / 2) });
	waveBuf->pDC->LineTo({ (LONG)width,(LONG)(height / 2) });
	waveBuf->pDC->SelectObject(pOldPen);

	return true;
}

GDIGUIWave::GDIGUIWave()
{
	type = GUI_CONTROL_WAVE;

	waveState = WAVE_STATE_STOPED;
	lineColor = COLORGDI_BLACK;

	amp = { 0 };
	waveMoved = 0;
	newWaveMoved = 0;
	startTime = { 0 };
	pauseTime = { 0 };
	secRec = 0;
	lastBeat = 0;
	nRenderd = 0;

	timeSpan = 2000;
	ampSpan = 2000;
	freq = 0.0f;
	nReview = 0;
	waveBuf = NULL;

	textFormat = TEXTFORMAT_TOP;
}

GDIGUIWave::~GDIGUIWave()
{
	Release();
}

bool GDIGUIWave::Release()
{
	if (waveBuf)
	{
		DeleteMemDCBMP(waveBuf);
		waveBuf = NULL;
	}

	GDIGUIControl::Release();

	return true;
}

void GDIGUIWave::PrepareWaveDC(CDC * pdc)
{
	if (pdc)
	{
		if (waveBuf)
			delete waveBuf;

		waveBuf = new memDCBMP;
		MyDrawPrepareOne(pdc, waveBuf, 0, 0, CRect(0, 0, width, height));

		SetEmptyBackground();
	}
}

byte GDIGUIWave::HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
{
	if (bDisabled || !bVisible)
		return false;

	byte event = CGUIControl::HandleMouse(uMsg, pt, wParam, lParam);

	if (event == GUI_EVENT_MOUSEMOVE)
	{
		if (waveState == WAVE_STATE_PAUSED)
		{
			if (samples.GetSize() == 0)
				return event;
			displayEvent = event;

			nReview = 0;
			sample3D *pSample = samples.GetPAt(nRenderd - 1);
			if (pSample == NULL)
				return event;
			int xDiff = posX + width - pt.x;
			//鼠标位置对应时间与最后一个绘制的样本时间差
			LONGLONG timeCursor = curTime.QuadPart - startTime.QuadPart
				- (LONGLONG)(timeSpan / 1000.0f*(float)xDiff / width*frequency.QuadPart);
			LONGLONG timeDiff = pSample->time - timeCursor;
			if (timeDiff < 0)
			{
				nReview = nRenderd;
				return event;
			}
			int idxDiff = (int)(timeDiff / frequency.QuadPart * freq);
			int idx = max(0, nRenderd - 1 - idxDiff);

			pSample = samples.GetPAt(idx);
			if (pSample == NULL)
				return event;
			LONGLONG timeDist = abs(pSample->time - timeCursor);
			nReview = idx + 1;
			idx--;
			int i;
			for (i = 0; i < 50 && idx >= 0; i++, idx--)
			{
				pSample = samples.GetPAt(idx);
				if (pSample == NULL)
					return event;
				LONGLONG newTimeDist = abs(pSample->time - timeCursor);
				if (newTimeDist < timeDist)
				{
					timeDist = newTimeDist;
					nReview = idx + 1;
				}
				else
					break;
			}
			if (i == 0)
			{
				idx++;
				for (i = 0; i < 200 && idx < nRenderd; i++)
				{
					idx++;
					pSample = samples.GetPAt(idx);
					if (pSample == NULL)
						return event;
					LONGLONG newTimeDist = abs(pSample->time - timeCursor);
					if (newTimeDist < timeDist)
					{
						timeDist = newTimeDist;
						nReview = idx + 1;
					}
					else
						break;
				}
			}
		}
	}
	else if (event == GUI_EVENT_MOUSELEAVE)
	{
		if (waveState == WAVE_STATE_PAUSED)
		{
			if (nReview > 0)
			{
				//使鼠标移出控件时更新review
				//nReview = 0;
				//displayEvent = event;
			}
		}
	}

	return event;
}

bool GDIGUIWave::DisplayCycle(LPGDIDevice dev)
{
	if (waveState == WAVE_STATE_RUNNING)
	{
		QueryPerformanceCounter(&curTime);

		//计算波形移动量
		sec = (curTime.QuadPart - startTime.QuadPart) / (double)frequency.QuadPart;
		int roundXBias = ROUND(width * 1000 * sec / timeSpan - waveMoved);

		newWaveMoved = roundXBias;//当前移动量
		waveMoved += roundXBias;//总移动量
		lastX -= newWaveMoved;//上一个样本位置偏移

		if (waveBuf)
		{
			if (waveBuf->pDC)
			{
				// 波形移动
				TRANSLATION_PDC(waveBuf->pDC, width, height, -roundXBias, 0);

				// 填充背景
				RECT refreshrect = RECT(width - roundXBias, 0, roundXBias, height);
				waveBuf->pDC->FillSolidRect(&refreshrect, color);
				// 画基准线
				CPen pen0(PS_SOLID, 2, COLORGDI_GREY);
				CPen* pOldPen = waveBuf->pDC->SelectObject(&pen0);
				if (newWaveMoved > 0)
				{
					waveBuf->pDC->MoveTo({ width, (LONG)(height / 2) });
					waveBuf->pDC->LineTo({ width - newWaveMoved - 1, (LONG)(height / 2) });
				}
				//画读秒线
				if (sec >= secRec)
				{
					CPen penSec(PS_SOLID, 1, COLORGDI_MIDGREY);
					CPen penSecBegin(PS_SOLID, 1, COLORGDI_RED2);
					if (secRec>0)
						waveBuf->pDC->SelectObject(&penSec);
					else
						waveBuf->pDC->SelectObject(&penSecBegin);

					int secX = ROUND(width - waveMoved + (float)width * 1000 * secRec / timeSpan) - 1;
					waveBuf->pDC->MoveTo({ secX, 1 });
					waveBuf->pDC->LineTo({ secX, height });

					//文本显示
					/*WCHAR strSec[16] = L"";
					StringCchPrintfW(strSec, 16, L"%d", secRec);
					CFont *pOldFont = waveBuf->pDC->SelectObject(font);
					waveBuf->pDC->SetTextColor(0xA0A0A0);

					RECT rcSec = { secX - 10, height / 2, secX - 2, height / 2 + 20 };
					waveBuf->pDC->SetBkMode(TRANSPARENT);
					waveBuf->pDC->DrawTextW(strSec, &rcSec, TEXTFORMAT_RIGHTTOP);
					waveBuf->pDC->SelectObject(pOldFont);*/

					secRec = (LONGLONG)sec + 1;
				}

				waveBuf->pDC->SelectObject(pOldPen);
			}
		}
		if (lastTick.QuadPart == 0)
			lastTick.QuadPart = startTime.QuadPart;

		//if (displayEvent != GUI_EVENT_NULL)
		bool res = Render(dev);

		lastTick.QuadPart = curTime.QuadPart;
		return res;
	}
	else if (displayEvent != GUI_EVENT_NULL)
	{
		bool res = Render(dev);

		displayEvent = GUI_EVENT_NULL;
		return res;
	}

	return false;
}

void GDIGUIWave::HandleCMD(UINT cmd)
{
	switch (cmd)
	{
	case GUI_CMD_START:
		if (waveState == WAVE_STATE_STOPED)
		{
			Start();
		}
		else if (waveState == WAVE_STATE_PAUSED)
		{
			Resume();
		}
		break;
	case GUI_CMD_PAUSE:
		if (waveState == WAVE_STATE_RUNNING)
		{
			Pause();
		}
		else if (waveState == WAVE_STATE_PAUSED)
		{
			Resume();
		}
		break;
	case GUI_CMD_STOP:
		if (waveState == WAVE_STATE_RUNNING)
		{
			Pause();
		}
		else if (waveState != WAVE_STATE_STOPED)
		{
			Stop();
		}
		break;
	}
}

void GDIGUIWave::AddSample(SAMPLE_TYPE ampval1, SAMPLE_TYPE ampval2, SAMPLE_TYPE ampval3, LONGLONG tick)
{
	if (waveState == WAVE_STATE_RUNNING)
	{
		//计算样本频率
		if (lastBeat == 0)
		{
			;
		}
		else
		{
			const float rate = 0.1f;
			if (freq == -1)
				freq = (float)frequency.QuadPart / (tick - lastBeat);
			else
				freq = freq*(1 - rate) + rate*frequency.QuadPart / (tick - lastBeat);
		}
		lastBeat = tick;

		//添加样本
		sample3D temp = { ampval1, ampval2, ampval3, tick };
		samples.Add(temp);
	}
}

void GDIGUIWave::AddTimeStamp(LONGLONG peakTime, TIMETYPE type)
{
	times.Add({ peakTime, type });
}

const sample3D *GDIGUIWave::GetLastSample() const
{
	return samples.GetPAt(samples.GetSize() - 1);
}

int GDIGUIWave::GetSampleCount() const
{
	return samples.GetSize();
}

void GDIGUIWave::SetTimeSpan(float span)
{
	timeSpan = span;
}

void GDIGUIWave::SetAmpSpan(float span)
{
	ampSpan = span;
}

void GDIGUIWave::RestoreFlag()
{
	samples.RemoveAll();
	times.RemoveAll();
	freq = 0.0f;
	nHandled = 0;
	nRenderd = 0;
	nReview = 0;

	amp = { 0,0,0,0 };
	lastX = width;
	lastYx = 0;
	lastYy = 0;
	lastYz = 0;
	waveMoved = 0;
	newWaveMoved = 0;

	startTime = { 0 };
	pauseTime = { 0 };
	lastTick = { 0 };
	lastBeat = 0;
	secRec = 0;
	sec = 0.0;
}

void GDIGUIWave::Start()
{
	RestoreFlag();

	QueryPerformanceCounter(&startTime);
	waveState = WAVE_STATE_RUNNING;
}

void GDIGUIWave::Pause()
{
	QueryPerformanceCounter(&pauseTime);

	waveState = WAVE_STATE_PAUSED;
}

void GDIGUIWave::Resume()
{
	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);

	//暂停后时间修正
	startTime.QuadPart += tick.QuadPart - pauseTime.QuadPart;
	lastTick.QuadPart += tick.QuadPart - pauseTime.QuadPart;

	// 复位样本查看
	if (nReview > 0)
		nReview = 0;

	waveState = WAVE_STATE_RUNNING;
}

void GDIGUIWave::Stop()
{
	waveState = WAVE_STATE_STOPED;

	SetEmptyBackground();
	Invalidate();// 强制更新
}

bool GDIGUIWave::Render(LPGDIDevice dev)
{
	if (!bVisible)
		return false;

	if (!dev || dev->IsDCNull())
		return false;

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();
	if (pDestDC == NULL || pMemDC == NULL/* || pMemDCBack == NULL*/)
		return false;

	CPen penLineX(PS_SOLID, 1, lineColor);
	CPen penLineY(PS_SOLID, 1, lineColorY);
	CPen penLineZ(PS_SOLID, 1, lineColorZ);
	// 绘制波形
	if (waveState == WAVE_STATE_RUNNING)
	{
		if (waveBuf != NULL && waveBuf->pDC != NULL)
		{
			CPen* pOldPen = waveBuf->pDC->SelectObject(&penLineX);
			CPen penAux(PS_SOLID, 1, COLORGDI_HEAVYGREY);

			while (samples.GetSize() > nRenderd)
			{
				// 计算采样点在波形图中位置
				// GetAt获取引用，线程不安全，列表插入时指针可能改变
				amp = samples.GetAtSafe(nRenderd);
				int ampYx = (int)(amp.ampX / ampSpan*height / 2);
				int ampYy = (int)(amp.ampY / ampSpan*height / 2);
				int ampYz = (int)(amp.ampZ / ampSpan*height / 2);
				//int ampX = width - newWaveMoved
				//	+ (int)(width*1000*(amp.time - lastTick.QuadPart + startTime.QuadPart)
				/// (float)frequency.QuadPart / timeSpan);
				int ampX = width - waveMoved
					+ (int)(amp.time * 1000 / (float)frequency.QuadPart*width / timeSpan);

				// 防止画线画到buffer外，造成画面上下一帧波形产生断裂
				if (ampX >= width)
					break;

				//波形断裂控制（1、防止绘制超长一段线 2、防止连接长时间间隔的波形）
				const float msTimeInterupt = 200.0f;//大于这个毫秒时间间隔的数据间不绘制波形
				if (lastX < 0 || (ampX - lastX) > width*msTimeInterupt / timeSpan)
				{
					;
				}
				else
				{
					// 画线
					waveBuf->pDC->SelectObject(&penLineX);
					SmoothLine(waveBuf->pDC
						, { (LONG)(lastX), (LONG)(height / 2 - lastYx) }
						, { ampX, (LONG)(height / 2 - ampYx) }
					, lineColor);
					waveBuf->pDC->SelectObject(&penLineY);
					SmoothLine(waveBuf->pDC
						, { (LONG)(lastX), (LONG)(height / 2 - lastYy) }
						, { ampX, (LONG)(height / 2 - ampYy) }
					, lineColorY);
					waveBuf->pDC->SelectObject(&penLineZ);
					SmoothLine(waveBuf->pDC
						, { (LONG)(lastX), (LONG)(height / 2 - lastYz) }
						, { ampX, (LONG)(height / 2 - ampYz) }
					, lineColorZ);
				}

				// 采样频度辅助线
				waveBuf->pDC->SelectObject(&penAux);
				waveBuf->pDC->MoveTo({ ampX, 16 });
				waveBuf->pDC->LineTo({ ampX, 18 });

				// 样本列表删除 & 数据更新
				nRenderd++;

				lastX = ampX;
				lastYx = (float)ampYx;
				lastYy = (float)ampYy;
				lastYz = (float)ampYz;
			}

			//绘制时间戳
			CPen penTime1(PS_SOLID, 2, lineColor);
			CPen penTime2(PS_SOLID, 2, lineColorZ);
			while (times.GetSize() > 0)
			{
				int endY = 2;
				timeStamp time = times.GetAtSafe(0);
				if (time.type == TIMETYPE_1)
				{
					waveBuf->pDC->SelectObject(&penTime1);
					endY = 34;
				}
				else if (time.type == TIMETYPE_2)
				{
					waveBuf->pDC->SelectObject(&penTime2);
					endY = 18;
				}
				int peakX = width - waveMoved
					+ (int)(time.time * 1000 / (float)frequency.QuadPart*width / timeSpan);
				waveBuf->pDC->MoveTo({ peakX, 2 });
				waveBuf->pDC->LineTo({ peakX, endY });
				times.Remove(0);
			}
			waveBuf->pDC->SelectObject(pOldPen);
		}
	}
	//else if (waveState == WAVE_STATE_STOPED)
	//	SetEmptyBackground();

	if (waveBuf)
	{
		if (waveBuf->pDC)
		{
			pMemDC->BitBlt(posX, posY, width, height
				, waveBuf->pDC, 0, 0, SRCCOPY);
		}
	}

	// 绘制文本
	// 当前波形值
	WCHAR txt[64] = { 0 };
	StringCchPrintfW(txt, 64, L"(%d/%d) %4d, %4d, %4d"
		, samples.GetSize(), samples.GetCapacity()
		, amp.ampX, amp.ampY, amp.ampZ);
	SetText(txt);
	RenderText(pMemDC, textFormat);
	// 振幅刻度
	CFont *pold = (CFont*)pMemDC->SelectObject(font);
	pMemDC->SetTextColor(COLORGDI_HEAVYGREY);
	StringCchPrintfW(txt, 64, L"%.1f", ampSpan);
	pMemDC->DrawTextW(txt, &rcText, TEXTFORMAT_LEFTTOP);
	StringCchPrintfW(txt, 64, L"-%.1f", ampSpan);
	pMemDC->DrawTextW(txt, &rcText, TEXTFORMAT_LEFTBOTTOM);
	// 频率
	StringCchPrintfW(txt, 64, L"%.1fhz", freq);
	pMemDC->DrawTextW(txt, &rcText, TEXTFORMAT_RIGHTTOP);
	// 计时
	pMemDC->SetTextColor(COLORGDI_DARKGREY);
	TimeString(txt, 64, sec);
	pMemDC->DrawTextW(txt, &rcText, TEXTFORMAT_BOTTOM);

	// 波形值查看
	if (waveState == WAVE_STATE_PAUSED && nReview > 0)
	{
		CPen* pOldPen = pMemDC->SelectObject(&penLineX);

		sample3D sam = samples.GetAtSafe(nReview - 1);
		pMemDC->SetTextColor(COLORGDI_GREEN3);
		StringCchPrintfW(txt, 64, L"查看数据: %4d, %4d, %4d(%.1f) "
			, sam.ampX, sam.ampY, sam.ampZ
			, sqrtf(SQ(sam.ampX) + SQ(sam.ampY) + SQ(sam.ampZ))
		);
		WCHAR strTime[32];
		TimeString(strTime, 32, sam.time / (double)frequency.QuadPart);
		StringCchCat(txt, 64, strTime);
		pMemDC->DrawTextW(txt, &rcText, TEXTFORMAT_RIGHTBOTTOM);

		int reviewX = width - waveMoved
			+ (int)(sam.time * 1000 / (float)frequency.QuadPart*width / timeSpan);

		AlphaBlendLine(pMemDC, { posX + reviewX, posY }, height, COLORGDI_GREEN2, 200);

		/*CBrush brX, brY, brZ;
		brX.CreateSolidBrush(lineColor);
		brY.CreateSolidBrush(lineColorY);
		brZ.CreateSolidBrush(lineColorZ);
		CBrush *oldBr = pMemDC->SelectObject(&brX);
		pMemDC->Ellipse(
		CRect(posX + reviewX - 1, posY + height / 2 - sam.ampX / ampSpan*height / 2
		, posX + reviewX + 2, posY + height / 2 - sam.ampX / ampSpan*height / 2 + 3));
		pMemDC->SelectObject(&brY);
		pMemDC->Ellipse(
		CRect(posX + reviewX - 1, posY + height / 2 - sam.ampY / ampSpan*height / 2
		, posX + reviewX + 2, posY + height / 2 - sam.ampY / ampSpan*height / 2 + 3));
		pMemDC->SelectObject(&brZ);
		pMemDC->Ellipse(
		CRect(posX + reviewX - 1, posY + height / 2 - sam.ampZ / ampSpan*height / 2
		, posX + reviewX + 2, posY + height / 2 - sam.ampZ / ampSpan*height / 2 + 3));
		pMemDC->SelectObject(oldBr);*/
		pMemDC->SelectObject(&pOldPen);
	}
	pMemDC->SelectObject(pold);

	// 绘制图例
	/*int iX = (int)(posX + width / 2 * 1.1f);
	int iY = (int)(posY + 0.9f*height);
	CPen* pOldPen = pMemDC->SelectObject(&penLineX);
	pMemDC->MoveTo(iX, iY); iX += 8;
	pMemDC->LineTo(iX, iY); iX += 20;
	pMemDC->SelectObject(&penLineY);
	pMemDC->MoveTo(iX, iY); iX += 8;
	pMemDC->LineTo(iX, iY); iX += 20;
	pMemDC->SelectObject(&penLineZ);
	pMemDC->MoveTo(iX, iY); iX += 8;
	pMemDC->LineTo(iX, iY); iX += 20;
	pMemDC->SelectObject(pOldPen);*/

	// 显示到目标
	pDestDC->BitBlt(posX, posY, width, height
		, pMemDC, posX, posY, SRCCOPY);

	return true;
}

int CGDIGUISystem::countLoop = 0;

void CGDIGUISystem::ThreadGUI(LPVOID lpParam)
{
	threadParam *ppgui = (threadParam*)lpParam;
	CGDIGUISystem *pGUISys = (CGDIGUISystem*)ppgui->pGUISys;//控件系统
	GDIGUIControl **ppctrl;//控件列表指针
	GDIGUIControl **ppback;//背景列表指针
	CDC *pDC;
	if (!pGUISys)
	{
		return;
	}
	else
	{
		if (!pGUISys->device)
			return;

		pDC = pGUISys->device->GetDestDC();

		if (!pDC)
			return;

		ppctrl = pGUISys->controls.GetData();
		ppback = pGUISys->backdrop.GetData();
	}
	LPGDIDevice device = pGUISys->device;

	//释放线程参数
	delete lpParam;

	CDC *pDestDC = device->GetDestDC();
	CDC *pMemDC = device->GetMemDC();
	CDC *pMemDCBack = device->GetMemDCBack();
	if (pMemDC == NULL || pMemDCBack == NULL || pDestDC == NULL)
		return;
	RECT rcFull = { 0,0,0,0 };
	if (!(device->GetRect(&rcFull)))
		return;

	//默认背景
	pMemDC->FillSolidRect(&rcFull, COLORGDI_DEFAULT);
	pMemDCBack->FillSolidRect(&rcFull, COLORGDI_DEFAULT);
	while (true)
	{
		//if (ppctrl != pGUISys->controls.GetData())//更新控件列表指针
		//{
		//	ppctrl = pGUISys->controls.GetData();
		//}
		//if (pGUISys->nControls <= 0)//没有控件
		//{
		//	Sleep(10);
		//	continue;
		//}

		//背景控件
		bool res = false;
		for (int i = 0; i < pGUISys->backdrop.GetSize(); i++)
		{
			GDIGUIControl *pback = ppback[i];

			if ((!pback->bDisabled && pback->bVisible)
				/*|| pback->displayEvent == GUI_EVENT_REFRESH*/)
			{
				res |= pback->DisplayCycle(device);
			}
		}
		if (res)//背景控件更新，所有其他控件更新
		{
			for (int i = 0; i < pGUISys->nControls; i++)
			{
				ppctrl[i]->Invalidate();
			}
		}
		//其他控件
		for (int i = 0; i < pGUISys->nControls; i++)
		{
			GDIGUIControl *pctrl = ppctrl[i];

			if ((!pctrl->bDisabled && pctrl->bVisible)
				/*|| pctrl->displayEvent == GUI_EVENT_REFRESH*/)//控件处于使能状态
			{
				pctrl->DisplayCycle(device);
			}
		}

		if (res)//背景控件更新时，画面整体更新，拷贝到窗口DC一次
		{
			pDestDC->BitBlt(rcFull.left, rcFull.top, WIDTHOF(rcFull), HEIGHTOF(rcFull)
				, pMemDC, rcFull.left, rcFull.top, SRCCOPY);
		}

		countLoop++;
		Sleep(2);
	}
}

void CGDIGUISystem::VarInit()
{
	CGUIControl::GetFrequency();//控件获取系统频率

	device = NULL;
	pBufW = NULL;
	pBufH = NULL;

	fontDefault = NULL;
	fonts = NULL;
	bUseBackdrop = true;

	nFonts = 0;
	nControls = 0;

	//lastLMBdown = false;
	pControlFocus = NULL;
	bBlock = false;

	bInvalidate = false;

	pEventProc = NULL;
	hThreadGUI = NULL;

	if (!fontDefault)
		fontDefault = new CFont;
	fontDefault->CreateFontW(
		14, 0, 0, 0, FW_NORMAL
		, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS
		, PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
}

void CGDIGUISystem::BeginGUIThread()
{
	threadParam *pParam = new threadParam;
	pParam->pGUISys = this;
	hThreadGUI = (HANDLE)_beginthread(ThreadGUI, 0, pParam); // 创建线程
	countLoop = 0;
}

void CGDIGUISystem::ShutDown()
{
	nControls = 0;
	controls.RemoveAll();
}

bool CGDIGUISystem::AddControl(GDIGUIControl * pControl)
{
	HRESULT hr = S_OK;

	hr = controls.Add(pControl);
	if (FAILED(hr))
	{
		return false;
	}
	nControls++;

	return true;
}

bool CGDIGUISystem::AddBack(GDIGUIBack * pBack)
{
	HRESULT hr = S_OK;

	hr = backdrop.Add(pBack);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

CGDIGUISystem::CGDIGUISystem()
{
	VarInit();

	//BeginGUIThread();
}

CGDIGUISystem::CGDIGUISystem(LPGDIDevice dev)
{
	VarInit();
	device = dev;

	//BeginGUIThread();
}

CGDIGUISystem::~CGDIGUISystem()
{
	// 字体清除
	for (int i = 0; i < nFonts; i++)
	{
		if (fonts[i])
			fonts[i]->DeleteObject();//TODO 是否清除不完全？
	}
	nFonts = 0;
	SAFE_DELETE_LIST(fonts);

	// 背景控件清除
	backdrop.RemoveAll();

	// 控件清除
	for (int i = 0; i < controls.GetSize(); i++)
	{
		GDIGUIControl* pControl = controls.GetAt(i);
		SAFE_DELETE(pControl);
	}

	nControls = 0;
	controls.RemoveAll();
}

CFont * CGDIGUISystem::GetFont(int ID) const
{
	if (ID < 0 || ID >= nFonts) return NULL;
	return fonts[ID];
}

bool CGDIGUISystem::SetControlText(int ID, WCHAR * strText)
{
	NULL_RETURN_FALSE(strText);

	for (int i = 0; i < nControls; i++)
	{
		if (controls[i]->GetID() == ID)
		{
			controls[i]->SetText(strText);

			controls[i]->inputPos = wcslen(strText);

			//controls[i]->Render(device);//留到多线程中绘制
			controls[i]->displayEvent = GUI_EVENT_TEXTCHANGE;

			return true;
		}
	}
	return false;
}

bool CGDIGUISystem::AddFont(WCHAR * fontName, int * fontID, INT Height, INT Width, INT Weight, BYTE Quality, int Escapement, int Orientation, bool Italics, bool Underline, bool StrikeOut, BYTE Charset, BYTE OutPrecision, BYTE ClipPrecision, BYTE PitchAndFamily)
{
	if (!fonts)
	{
		fonts = new CFont*[1];
		if (!fonts) return false;
	}
	else
	{
		CFont **temp;
		temp = new CFont *[nFonts + 1];
		if (!temp) return false;
		memcpy(temp, fonts, sizeof(CFont*) * nFonts);
		delete[] fonts;
		fonts = temp;
	}

	fonts[nFonts] = new CFont;
	if (FAILED(fonts[nFonts]->CreateFontW(
		Height, Width, Escapement, Orientation, Weight
		, Italics, Underline, StrikeOut, Charset, OutPrecision, ClipPrecision
		, Quality, PitchAndFamily, fontName)))
		return false;

	if (!fonts[nFonts])
		return false;

	if (fontID)
		*fontID = nFonts + 1;

	nFonts++;

	return true;
}

bool CGDIGUISystem::AddStatic(int ID, float x, float y, float width, float height, WCHAR * strText, DWORD color, int fontID, WCHAR *file, GUI_WINDOCK_MODE dock)
{
	if (!device)
		return false;

	GDIGUIStatic *pStatic = new GDIGUIStatic;

	if (pStatic == NULL)
		return false;

	if (!AddControl(pStatic))
	{
		SAFE_DELETE(pStatic);
		return false;
	}

	pStatic->SetID(ID);

	pStatic->color = color;
	pStatic->dockMode = dock;
	pStatic->dockX = x;
	pStatic->dockY = y;
	pStatic->dockW = width;
	pStatic->dockH = width;
	pStatic->width = ROUND(width);
	pStatic->height = ROUND(height);
	pStatic->Dock(pBufW, pBufH);

	pStatic->SetText(strText);
	pStatic->textColor = color;
	pStatic->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : fontDefault;

	pStatic->state = GUI_STATE_OUT;
	pStatic->alpha = 255;

	//导入图片
	if (!pStatic->pic)
		pStatic->pic = new memPic();
	pStatic->pic->LoadImg(file, ROUND(width), ROUND(height));

	return true;
}

bool CGDIGUISystem::AddBackdrop(int ID, float x, float y, float width, float height, WCHAR * file, GUI_WINDOCK_MODE dock)
{
	if (!device)
		return false;

	GDIGUIBack *pBack = new GDIGUIBack;

	if (pBack == NULL)
		return false;

	if (!AddBack(pBack))
	{
		SAFE_DELETE(pBack);
		return false;
	}

	pBack->SetID(ID);

	pBack->color = COLORGDI_WHITE;
	pBack->dockMode = dock;
	pBack->dockX = x;
	pBack->dockY = y;
	pBack->dockW = width;
	pBack->dockH = width;
	pBack->width = ROUND(width);
	pBack->height = ROUND(height);
	pBack->Dock(pBufW, pBufH);

	pBack->SetText(L"");
	pBack->textColor = COLORGDI_BLACK;
	pBack->font = fontDefault;

	pBack->state = GUI_STATE_OUT;
	pBack->alpha = 255;

	//导入图片
	if (!pBack->pic)
		pBack->pic = new memPic();
	pBack->pic->LoadImg(file, ROUND(width), ROUND(height));

	return true;
}

bool CGDIGUISystem::AddButton(int ID, float x, float y, float width, float height, WCHAR * strText, DWORD color, int fontID, GUI_WINDOCK_MODE dock, WCHAR * up, WCHAR * on, WCHAR * down)
{
	if (!device)
		return false;

	GDIGUIButton *pButton = new GDIGUIButton;

	if (pButton == NULL)
		return false;

	if (!AddControl(pButton))
	{
		SAFE_DELETE(pButton);
		return false;
	}

	pButton->SetID(ID);

	pButton->color = color;
	pButton->dockMode = dock;
	pButton->dockX = x;
	pButton->dockY = y;
	pButton->dockW = width;
	pButton->dockH = height;
	pButton->width = ROUND(width);
	pButton->height = ROUND(height);
	pButton->Dock(pBufW, pBufH);

	pButton->SetText(strText);
	pButton->textColor = color;
	pButton->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : fontDefault;

	pButton->overdx = -1;
	pButton->overdy = -1;
	pButton->downdx = 1;
	pButton->downdy = 1;
	pButton->state = GUI_STATE_OUT;
	pButton->alpha = 0;
	pButton->displayEvent = GUI_EVENT_NULL;

	//导入图片
	if (!pButton->pic)
		pButton->pic = new memPic();
	pButton->pic->LoadImg(up, ROUND(width), ROUND(height));

	if (!pButton->picon)
		pButton->picon = new memPic();
	pButton->picon->LoadImg(on, ROUND(width), ROUND(height));

	if (!pButton->picdown)
		pButton->picdown = new memPic();
	pButton->picdown->LoadImg(down, ROUND(width), ROUND(height));

	return true;
}

bool CGDIGUISystem::AddWave(int ID, float x, float y, float width, float height, DWORD color, DWORD lineColor, GUI_WINDOCK_MODE dock, int fontID)
{
	if (!device)
		return false;

	GDIGUIWave *pWave = new GDIGUIWave;

	if (pWave == NULL)
		return false;

	if (!AddControl(pWave))
	{
		SAFE_DELETE(pWave);
		return false;
	}

	pWave->SetID(ID);

	pWave->color = color;
	pWave->dockMode = dock;
	pWave->dockX = x;
	pWave->dockY = y;
	pWave->dockW = width;
	pWave->dockH = height;
	pWave->width = ROUND(width);
	pWave->height = ROUND(height);
	pWave->Dock(pBufW, pBufH);

	//pWave->textColor = color;
	pWave->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : fontDefault;

	pWave->state = GUI_STATE_OUT;
	pWave->SetLineColor(lineColor);
	pWave->alpha = 255;

	pWave->PrepareWaveDC(device->GetDestDC());

	return true;
}

bool CGDIGUISystem::AddTristate(int ID, float x, float y, float width, float height, WCHAR *strText, DWORD color, int fontID, GUI_WINDOCK_MODE dock, WCHAR * normal, WCHAR * on, WCHAR * off)
{
	if (!device)
		return false;

	GDIGUITristate *pTristate = new GDIGUITristate;

	if (pTristate == NULL)
		return false;

	if (!AddControl(pTristate))
	{
		SAFE_DELETE(pTristate);
		return false;
	}

	pTristate->SetID(ID);

	pTristate->dockMode = dock;
	pTristate->dockX = x;
	pTristate->dockY = y;
	pTristate->dockW = width;
	pTristate->dockH = height;
	pTristate->width = ROUND(width);
	pTristate->height = ROUND(height);
	pTristate->Dock(pBufW, pBufH);

	pTristate->SetText(strText);
	pTristate->textColor = color;
	pTristate->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : fontDefault;

	pTristate->state = GUI_STATE_OUT;
	pTristate->alpha = 255;
	pTristate->displayEvent = GUI_EVENT_NULL;

	//导入图片
	if (!pTristate->pic)
		pTristate->pic = new memPic();
	pTristate->pic->LoadImg(normal, ROUND(width), ROUND(height));

	if (!pTristate->picon)
		pTristate->picon = new memPic();
	pTristate->picon->LoadImg(on, ROUND(width), ROUND(height));

	if (!pTristate->picoff)
		pTristate->picoff = new memPic();
	pTristate->picoff->LoadImg(off, ROUND(width), ROUND(height));

	return true;
}

bool CGDIGUISystem::AddSwitch(int ID, float x, float y, float width, float height, WCHAR * strText, DWORD color, int fontID, GUI_WINDOCK_MODE dock, WCHAR * normal, WCHAR * on)
{
	if (!device)
		return false;

	GDIGUISwitch *pSwitch = new GDIGUISwitch;

	if (pSwitch == NULL)
		return false;

	if (!AddControl(pSwitch))
	{
		SAFE_DELETE(pSwitch);
		return false;
	}

	pSwitch->SetID(ID);

	pSwitch->dockMode = dock;
	pSwitch->dockX = x;
	pSwitch->dockY = y;
	pSwitch->dockW = width;
	pSwitch->dockH = height;
	pSwitch->width = ROUND(width);
	pSwitch->height = ROUND(height);
	pSwitch->Dock(pBufW, pBufH);

	pSwitch->SetText(strText);
	pSwitch->textColor = color;
	pSwitch->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : fontDefault;

	pSwitch->state = GUI_STATE_OUT;
	pSwitch->alpha = 255;
	pSwitch->displayEvent = GUI_EVENT_NULL;

	//导入图片
	if (!pSwitch->pic)
		pSwitch->pic = new memPic();
	pSwitch->pic->LoadImg(normal, ROUND(width), ROUND(height));

	if (!pSwitch->picon)
		pSwitch->picon = new memPic();
	pSwitch->picon->LoadImg(on, ROUND(width), ROUND(height));

	return true;
}

bool CGDIGUISystem::AddScrollBar(int ID, float x, float y, float width, float height, GUI_WINDOCK_MODE dock)
{
	if (!device)
		return false;

	GDIGUIScrollBar *pScrollBar = new GDIGUIScrollBar;

	if (pScrollBar == NULL)
		return false;

	if (!AddControl(pScrollBar))
	{
		SAFE_DELETE(pScrollBar);
		return false;
	}

	pScrollBar->SetID(ID);

	pScrollBar->dockMode = dock;
	pScrollBar->dockX = x;
	pScrollBar->dockY = y;
	pScrollBar->dockW = width;
	pScrollBar->dockH = height;
	pScrollBar->width = ROUND(width);
	pScrollBar->height = ROUND(height);
	pScrollBar->Dock(pBufW, pBufH);

	pScrollBar->font = fontDefault;

	pScrollBar->state = GUI_STATE_OUT;
	pScrollBar->alpha = 255;
	pScrollBar->displayEvent = GUI_EVENT_NULL;

	//导入图片
	pScrollBar->InitPic();

	//test
	//pScrollBar->SetTrackRange(0, 14);
	//pScrollBar->SetPageSize(6);
	return true;
}

bool CGDIGUISystem::AddComboBox(int ID, float x, float y, float width, float height, GUI_WINDOCK_MODE dock)
{
	if (!device)
		return false;

	GDIGUIComboBox *pComboBox = new GDIGUIComboBox;

	if (pComboBox == NULL)
		return false;

	if (!AddControl(pComboBox))
	{
		SAFE_DELETE(pComboBox);
		return false;
	}

	pComboBox->SetID(ID);

	pComboBox->dockMode = dock;
	pComboBox->dockX = x;
	pComboBox->dockY = y;
	pComboBox->dockW = width;
	pComboBox->dockH = height;
	pComboBox->width = ROUND(width);
	pComboBox->height = ROUND(height);
	pComboBox->Dock(pBufW, pBufH);

	pComboBox->font = fontDefault;

	pComboBox->state = GUI_STATE_OUT;
	pComboBox->alpha = 255;
	pComboBox->displayEvent = GUI_EVENT_NULL;

	//导入图片
	pComboBox->InitPic();

	return true;
}

GDIGUIControl * CGDIGUISystem::GetControl(int ID, UINT nControlType) const
{
	// Try to find the control with the given ID
	for (int i = 0; i < controls.GetSize(); i++)
	{
		GDIGUIControl* pControl = controls.GetAt(i);

		if (pControl->GetID() == ID && pControl->GetType() == nControlType)
		{
			return pControl;
		}
	}

	// Not found
	return NULL;
}

void CGDIGUISystem::SetCallbackEvent(LPGDIGUIEVENTCALLBACK pevent)
{
	pEventProc = pevent;
}

int CGDIGUISystem::HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
{
	//byte LMBstate = (byte)lastLMBdown + (byte)(LMBDown << 1);
	//lastLMBdown = LMBDown;
	int retVal = 0;
	bool controldown = false;//是否有控件按下
	for (int i = 0; i < nControls; i++)
	{
		if (!bBlock || ISFOCUS_GDI(i))//不阻塞或者第i个控件就是焦点控件
		{
			if (!controls[i]->bDisabled && controls[i]->bVisible)// 控件处于使能状态
			{
				byte event = controls[i]->HandleMouse(uMsg, pt, wParam, lParam);

				// 控件系统调度
				if (event == GUI_EVENT_LBUTTONDOWN)//按下的按钮阻塞其他按钮
				{
					if (controls[i]->GetType() == GUI_CONTROL_BUTTON)
					{
						controldown = true;
						BLOCK_ON_CONTROL_GDI(i)
					}
					else if (controls[i]->GetType() == GUI_CONTROL_EDIT)
					{
						controldown = true;
						FOCUS_CONTROL_GDI(i)
					}
				}
				else if (event == GUI_EVENT_LBUTTONOUTUP || event == GUI_EVENT_LBUTTONUP)
					BLOCK_OFF_GDI;
				/*if (controls[i]->GetType() == GUI_CONTROL_COMBOBOX)
				{
				if (bBlock)
				{
				if (controls[i]->state != GUI_STATE_DOWN)
				{
				BLOCK_OFF;
				}
				}
				else
				{
				if (controls[i]->state == GUI_STATE_DOWN)
				{
				controldown = true;
				BLOCK_ON_CONTROL(i)
				}
				}
				}*/

				if (event != GUI_EVENT_NULL)
				{
					retVal = 1;
					//重绘(交给多线程)
					/*if (controls[i]->type == GUI_CONTROL_BUTTON)
					{
					controls[i]->Render(device);
					}*/

					//回调
					if (pEventProc)
						pEventProc(controls[i]->GetID(), event, controls[i]);
				}
			}
		}
	}

	return retVal;
}

void CGDIGUISystem::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < nControls; i++)
	{
		if (controls[i]->GetType() == GUI_CONTROL_EDIT && ISFOCUS_GDI(i))
		{
			if (!controls[i]->bDisabled && controls[i]->bVisible)
			{
				bool cmd = controls[i]->HandleKeyboard(uMsg, wParam, lParam);

				if (cmd && pEventProc)
				{
					pEventProc(controls[i]->GetID(), GUI_EVENT_CMD, controls[i]);
					controls[i]->SetText(L"");
				}
			}
		}
	}
}

void CGDIGUISystem::HandleCMD(int ID, UINT cmd)
{
	for (int i = 0; i < nControls; i++)
	{
		if (controls[i]->GetID() == ID)
		{
			controls[i]->HandleCMD(cmd);
			break;
		}
	}
}

void CGDIGUISystem::Invalidate()
{
	//bInvalidate = true;

	//CDC *pMemDC = device->GetMemDC();
	//RECT rcFull;
	//bool res = device->GetRect(&rcFull);
	//if (res && pMemDC != NULL)
	//{
	//	//默认背景
	//	pMemDC->FillSolidRect(&rcFull, COLORGDI_DEFAULT);
	//}

	for (int i = 0; i < backdrop.GetSize(); i++)
	{
		backdrop[i]->Invalidate();
	}
	for (int i = 0; i < nControls; i++)
	{
		controls[i]->Invalidate();
	}
}

void CGDIGUISystem::Render()
{
	LARGE_INTEGER stime, etime, freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&stime);
	// 逐个绘制控件
	for (int i = 0; i < backdrop.GetSize(); i++)
	{
		//backdrop[i]->DisplayCycle(device);
		backdrop[i]->Render(device);
	}
	for (int i = 0; i < nControls; i++)
	{
		//controls[i]->DisplayCycle(device);
		controls[i]->Render(device);
	}
	QueryPerformanceCounter(&etime);
	time = 1000.0f*(etime.QuadPart - stime.QuadPart) / freq.QuadPart;
}

GDIGUIScrollBar::GDIGUIScrollBar()
{
	type = GUI_CONTROL_SCROLLBAR;

	bShowThumb = true;

	SetRect(&rcUpButton, 0, 0, 0, 0);
	SetRect(&rcDownButton, 0, 0, 0, 0);
	SetRect(&rcTrack, 0, 0, 0, 0);
	SetRect(&rcThumb, 0, 0, 0, 0);
	nPosition = 0;
	nPageSize = 1;
	nStart = 0;
	nEnd = 1;
	//m_Arrow = CLEAR;
	//m_dArrowTS = 0.0;

	picTrack = NULL;
	picThumb = NULL;
	picUpButton = NULL;
	picDownButton = NULL;
}

GDIGUIScrollBar::~GDIGUIScrollBar()
{
}

void GDIGUIScrollBar::UpdateRects()
{
	GDIGUIControl::UpdateRects();

	// Make the buttons square
	SetRect(&rcUpButton, rcBoundingBox.left, rcBoundingBox.top,
		rcBoundingBox.right, rcBoundingBox.top + WIDTHOF(rcBoundingBox));
	SetRect(&rcDownButton, rcBoundingBox.left, rcBoundingBox.bottom - WIDTHOF(rcBoundingBox),
		rcBoundingBox.right, rcBoundingBox.bottom);
	SetRect(&rcTrack, rcUpButton.left, rcUpButton.bottom,
		rcDownButton.right, rcDownButton.top);
	rcThumb.left = rcUpButton.left;
	rcThumb.right = rcUpButton.right;

	UpdateThumbRect();
}

void GDIGUIScrollBar::SetTrackRange(int nStart, int nEnd)
{
	this->nStart = nStart; this->nEnd = nEnd;
	Cap();
	UpdateThumbRect();
	if (!picThumb)
		picThumb = new memPic();
	picThumb->LoadImg(L"ScrollBarThumb.png", ROUND(width), HEIGHTOF(rcThumb));

}

void GDIGUIScrollBar::Scroll(int nDelta)
{
	// Perform scroll
	nPosition += nDelta;

	// Cap position
	Cap();

	// Update thumb position
	UpdateThumbRect();
}

void GDIGUIScrollBar::ShowItem(int nIndex)
{
	// Cap the index

	if (nIndex < 0)
		nIndex = 0;

	if (nIndex >= nEnd)
		nIndex = nEnd - 1;

	// Adjust position

	if (nPosition > nIndex)
		nPosition = nIndex;
	else if (nPosition + nPageSize <= nIndex)
		nPosition = nIndex - nPageSize + 1;

	UpdateThumbRect();
}

void GDIGUIScrollBar::UpdateThumbRect()
{
	if (nEnd - nStart > nPageSize)
	{
		int nThumbHeight = __max(HEIGHTOF(rcTrack) * nPageSize / (nEnd - nStart),
			SCROLLBAR_MINTHUMBSIZE);
		int nMaxPosition = nEnd - nStart - nPageSize;
		rcThumb.top = rcTrack.top + (nPosition - nStart) * (HEIGHTOF(rcTrack) - nThumbHeight)
			/ nMaxPosition;
		rcThumb.bottom = rcThumb.top + nThumbHeight;
		bShowThumb = true;

	}
	else
	{
		// No content to scroll
		rcThumb.bottom = rcThumb.top;
		bShowThumb = false;
	}
}

void GDIGUIScrollBar::Cap()
{
	if (nPosition < nStart ||
		nEnd - nStart <= nPageSize)
	{
		nPosition = nStart;
	}
	else if (nPosition + nPageSize > nEnd)
		nPosition = nEnd - nPageSize;
}

void GDIGUIScrollBar::InitPic()
{
	if (!picTrack)
		picTrack = new memPic();
	picTrack->LoadImg(L"ScrollBarTrack.png", ROUND(width), ROUND(height));
	if (!picThumb)
		picThumb = new memPic();
	picThumb->LoadImg(L"ScrollBarThumb.png", ROUND(width), 40);
	if (!picUpButton)
		picUpButton = new memPic();
	picUpButton->LoadImg(L"ScrollBarUpButton.png", ROUND(width), ROUND(width));
	if (!picDownButton)
		picDownButton = new memPic();
	picDownButton->LoadImg(L"ScrollBarDownButton.png", ROUND(width), ROUND(width));

}

byte GDIGUIScrollBar::HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
{
	if (bDisabled || !bVisible)
		return false;

	static int ThumbOffsetY;

	lastMouse = pt;
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	{
		// Check for click on up button
		if (PtInRect(&rcUpButton, pt))
		{
			//SetCapture(DXUTGetHWND());
			if (nPosition > nStart)
				--nPosition;
			UpdateThumbRect();
			//Arrow = CLICKED_UP;
			//dArrowTS = DXUTGetTime();

			displayEvent = GUI_EVENT_LBUTTONDOWN;
			return GUI_EVENT_LBUTTONDOWN;
		}

		// Check for click on down button

		if (PtInRect(&rcDownButton, pt))
		{
			//SetCapture(DXUTGetHWND());
			if (nPosition + nPageSize < nEnd)
				++nPosition;
			UpdateThumbRect();
			//Arrow = CLICKED_DOWN;
			//dArrowTS = DXUTGetTime();

			displayEvent = GUI_EVENT_LBUTTONDOWN;
			return GUI_EVENT_LBUTTONDOWN;
		}

		// Check for click on thumb

		if (PtInRect(&rcThumb, pt))
		{
			//SetCapture(DXUTGetHWND());
			bDrag = true;
			ThumbOffsetY = pt.y - rcThumb.top;

			displayEvent = GUI_EVENT_LBUTTONDOWN;
			return GUI_EVENT_LBUTTONDOWN;
		}

		// Check for click on track

		if (rcThumb.left <= pt.x &&
			rcThumb.right > pt.x)
		{
			//SetCapture(DXUTGetHWND());
			if (rcThumb.top > pt.y &&
				rcTrack.top <= pt.y)
			{
				Scroll(-(nPageSize - 1));

				displayEvent = GUI_EVENT_LBUTTONDOWN;
				return GUI_EVENT_LBUTTONDOWN;
			}
			else if (rcThumb.bottom <= pt.y &&
				rcTrack.bottom > pt.y)
			{
				Scroll(nPageSize - 1);

				displayEvent = GUI_EVENT_LBUTTONDOWN;
				return GUI_EVENT_LBUTTONDOWN;
			}
		}

		break;
	}

	case WM_LBUTTONUP:
	{
		bDrag = false;
		//ReleaseCapture();
		UpdateThumbRect();
		//Arrow = CLEAR;
		displayEvent = GUI_EVENT_LBUTTONUP;
		break;
	}

	case WM_MOUSEMOVE:
	{
		if (bDrag)
		{
			rcThumb.bottom += pt.y - ThumbOffsetY - rcThumb.top;
			rcThumb.top = pt.y - ThumbOffsetY;
			if (rcThumb.top < rcTrack.top)
				OffsetRect(&rcThumb, 0, rcTrack.top - rcThumb.top);
			else if (rcThumb.bottom > rcTrack.bottom)
				OffsetRect(&rcThumb, 0, rcTrack.bottom - rcThumb.bottom);

			// Compute first item index based on thumb position

			int nMaxFirstItem = nEnd - nStart - nPageSize;  // Largest possible index for first item
			int nMaxThumb = HEIGHTOF(rcTrack) - HEIGHTOF(rcThumb);  // Largest possible thumb position from the top

			nPosition = nStart +
				(rcThumb.top - rcTrack.top +
					nMaxThumb / (nMaxFirstItem * 2)) * // Shift by half a row to avoid last row covered by only one pixel
				nMaxFirstItem / nMaxThumb;

			displayEvent = GUI_EVENT_MOUSEMOVE;
			return GUI_EVENT_MOUSEMOVE;
		}

		break;
	}
	}

	return GUI_EVENT_NULL;
}

bool GDIGUIScrollBar::DisplayCycle(LPGDIDevice dev)
{
	if (displayEvent != GUI_EVENT_NULL)
	{
		bool res = Render(dev);

		displayEvent = GUI_EVENT_NULL;
		return res;
	}
	return false;
}

bool GDIGUIScrollBar::Render(LPGDIDevice dev)
{
	if (!bVisible)
		return false;

	if (!dev || dev->IsDCNull())
		return false;

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();
	if (pDestDC == NULL || pMemDC == NULL/* || pMemDCBack == NULL*/)
		return false;

	// 清除背景
	pMemDC->FillSolidRect(&rcBoundingBox, COLORGDI_DEFAULT);//清除背景

															// 绘制贴图
	if (picTrack)
		picTrack->Show(pMemDC, posX, posY, ROUNDF_BYTE(alpha), false);
	if (!bShowThumb)
	{
		RECT rcInnerTrack = rcBoundingBox;
		InflateRect(&rcInnerTrack, -1, -1);
		pMemDC->FillSolidRect(&rcInnerTrack, RGB(165, 165, 166));//清除滑动轨道
	}
	if (picUpButton)
		picUpButton->Show(pMemDC, posX, posY, ROUNDF_BYTE(alpha), false);
	if (picDownButton)
		picDownButton->Show(pMemDC, posX, posY + height - width, ROUNDF_BYTE(alpha), false);
	if (picThumb && bShowThumb)
		picThumb->Show(pMemDC, rcThumb.left, rcThumb.top, ROUNDF_BYTE(alpha), false);

	// 绘制文本
	RenderText(pMemDC, textFormat);

	// 显示到目标
	pDestDC->BitBlt(rcBoundingBox.left, rcBoundingBox.top, WIDTHOF(rcBoundingBox), HEIGHTOF(rcBoundingBox)
		, pMemDC, rcBoundingBox.left, rcBoundingBox.top, SRCCOPY);

	return true;
}

GDIGUIComboBox::GDIGUIComboBox()
{
	type = GUI_CONTROL_COMBOBOX;

	iSelected = -1;
	iFocused = -1;
	nRowHeight = 20;
	nDropHeight = 100;
	nSBWidth = 16;

	lastOpend = false;
	bOpened = false;
}

GDIGUIComboBox::~GDIGUIComboBox()
{
	RemoveAllItems();
}

void GDIGUIComboBox::InitPic()
{
	if (!pic)
		pic = new memPic();
	pic->LoadImg(L"FrameBox.png", WIDTHOF(rcText), HEIGHTOF(rcText));
	if (!picDropDown)
		picDropDown = new memPic();
	picDropDown->LoadImg(L"DropDown.png", WIDTHOF(rcDropdown), HEIGHTOF(rcDropdown));
	if (!picButton)
		picButton = new memPic();
	picButton->LoadImg(L"DropDownButton.png", WIDTHOF(rcButton), HEIGHTOF(rcButton));

	scrollBar.InitPic();
}

byte GDIGUIComboBox::HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
{
	if (bDisabled || !bVisible)
		return false;

	byte event = CGUIControl::HandleMouse(uMsg, pt, wParam, lParam);
	if (event)
	{
		displayEvent = event;
	}

	// Let the scroll bar handle it first.
	byte sbEvent = scrollBar.HandleMouse(uMsg, pt, wParam, lParam);
	if (sbEvent != 0)
	{
		displayEvent = GUI_EVENT_SCROLL;
		return GUI_EVENT_SCROLL;
	}

	switch (uMsg)
	{
	case WM_MOUSEMOVE:
	{
		if (bOpened && PtInRect(&rcDropdown, pt))
		{
			// Determine which item has been selected
			for (int i = 0; i < items.GetSize(); i++)
			{
				GUIComboBoxItem* pItem = items.GetAt(i);
				if (pItem->bVisible &&
					PtInRect(&pItem->rcActive, pt))
				{
					iFocused = i;
				}
			}
			displayEvent = GUI_EVENT_MOUSEMOVE;//父类消息处理函数不响应这个消息（指不因这个消息修改displayEvent）

			return GUI_EVENT_MOUSEMOVE;
		}
		break;
	}

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	{
		if (PtInRect(&rcBoundingBox, pt))
		{
			// Pressed while inside the control
			//bPressed = true;
			state = GUI_STATE_DOWN;//替换上一句
								   //SetCapture(DXUTGetHWND());

								   //if (!m_bHasFocus)
								   //	m_pDialog->RequestFocus(this);

								   // Toggle dropdown
			bOpened = !bOpened;
			/*if (m_bHasFocus)
			{
			m_bOpened = !m_bOpened;

			if (!m_bOpened)
			{
			if (!m_pDialog->m_bKeyboardInput)
			m_pDialog->ClearFocus();
			}
			}*/

			return event;
		}

		// Perhaps this click is within the dropdown
		if (bOpened && PtInRect(&rcDropdown, pt))
		{
			// Determine which item has been selected
			for (int i = scrollBar.GetTrackPos(); i < items.GetSize(); i++)
			{
				GUIComboBoxItem* pItem = items.GetAt(i);
				if (pItem->bVisible &&
					PtInRect(&pItem->rcActive, pt))
				{
					iFocused = iSelected = i;
					///m_pDialog->SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
					bOpened = false;

					//if (!m_pDialog->m_bKeyboardInput)
					//	m_pDialog->ClearFocus();

					break;
				}
			}
			displayEvent = GUI_EVENT_SELCHANGE;//父类消息处理函数无法处理rcDropdown内的鼠标消息

			return GUI_EVENT_SELCHANGE;
		}

		// Mouse click not on main control or in dropdown, fire an event if needed
		if (bOpened)
		{
			iFocused = iSelected;

			//m_pDialog->SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
			bOpened = false;
			displayEvent = GUI_EVENT_REFRESH;//父类消息处理函数无法处理区域外的鼠标消息
		}

		// Make sure the control is no longer in a pressed state
		state = GUI_STATE_OUT;

		// Release focus if appropriate
		/*if (!m_pDialog->m_bKeyboardInput)
		{
		m_pDialog->ClearFocus();
		}*/

		break;
	}

	case WM_LBUTTONUP:
	{
		if (state == GUI_STATE_DOWN && PtInRect(&rcBoundingBox, pt))
		{
			// Button click
			state = GUI_STATE_OVER;
			//ReleaseCapture();
			return event;
		}

		break;
	}

	case WM_MOUSEWHEEL:
	{
		int zDelta = (short)HIWORD(wParam) / WHEEL_DELTA;
		if (bOpened)
		{
			UINT uLines;
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0);
			scrollBar.Scroll(-zDelta * uLines);
		}
		else if (PtInRect(&rcBoundingBox, pt))//条件自己加的
		{
			if (zDelta > 0)
			{
				if (iFocused > 0)
				{
					iFocused--;
					iSelected = iFocused;

					displayEvent = GUI_EVENT_SELCHANGE;//父类消息处理函数无法处理rcDropdown内的鼠标消息
													   //if (!bOpened)
													   //	m_pDialog->SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
				}
			}
			else
			{
				if (iFocused + 1 < (int)GetNumItems())
				{
					iFocused++;
					iSelected = iFocused;

					displayEvent = GUI_EVENT_SELCHANGE;//父类消息处理函数无法处理rcDropdown内的鼠标消息
													   //if (!bOpened)
													   //	m_pDialog->SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, true, this);
				}
			}
		}

		return displayEvent;
	}
	};

	return GUI_EVENT_NULL;
}

bool GDIGUIComboBox::Render(LPGDIDevice dev)
{
	if (!bVisible)
		return false;

	if (!dev || dev->IsDCNull())
		return false;

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();
	CDC *pMemDCBack = dev->GetMemDCBack();
	if (pDestDC == NULL || pMemDC == NULL || pMemDCBack == NULL)
		return false;

	CFont *pold = (CFont*)pMemDC->SelectObject(font);
	pMemDC->SetTextColor(textColor);
	pMemDC->FillSolidRect(&rcBoundingBox, COLORGDI_DEFAULT);//清除背景

	byte curAlpha = (byte)alpha;
	if (!bOpened && state != GUI_STATE_OVER)
		curAlpha = (byte)(curAlpha*0.7f);
	if (pic)
		pic->Show(pMemDC, rcText.left, rcText.top, curAlpha, true, pMemDCBack);

	INT8 iState = 0;

	if (!bOpened)
		iState = /*DXUT_STATE_HIDDEN*/0;

	// Dropdown box
	//CDXUTElement* pElement = m_Elements.GetAt(2);

	// If we have not initialized the scroll bar page size,
	// do that now.
	static bool bSBInit;
	if (!bSBInit)
	{
		// Update the page size of the scroll bar
		/*if (m_pDialog->GetManager()->GetFontNode(pElement->iFont)->nHeight)
		scrollBar.SetPageSize(RectHeight(m_rcDropdownText) /
		m_pDialog->GetManager()->GetFontNode(pElement->iFont)->nHeight);
		else
		scrollBar.SetPageSize(RectHeight(m_rcDropdownText));*/
		scrollBar.SetPageSize(HEIGHTOF(rcDropdownText) / nRowHeight);
		bSBInit = true;
	}

	// Blend current color
	//pElement->TextureColor.Blend(iState, fElapsedTime);
	//pElement->FontColor.Blend(iState, fElapsedTime);

	//m_pDialog->DrawSprite(pElement, &m_rcDropdown, DXUT_NEAR_BUTTON_DEPTH);
	if (lastOpend != bOpened)
	{
		RECT rc = rcDropdown;
		rc.right += nSBWidth;
		//pMemDC->FillSolidRect(&rc, COLORGDI_DEFAULT);//清除背景
		pMemDC->BitBlt(rc.left, rc.top, rc.right, rc.bottom
			, pMemDCBack, rc.left, rc.top, SRCCOPY);
	}
	if (picDropDown && bOpened)
	{
		picDropDown->Show(pMemDC, rcDropdown.left, rcDropdown.top, ROUNDF_BYTE(alpha), true, pMemDCBack);
	}

	// Scroll bar
	if (bOpened)
		scrollBar.Render(dev);


	// Selection outline
	//CDXUTElement* pSelectionElement = m_Elements.GetAt(3);
	//pSelectionElement->TextureColor.Current = pElement->TextureColor.Current;
	//pSelectionElement->FontColor.Current = pSelectionElement->FontColor.States[DXUT_STATE_NORMAL];

	//DXUTFontNode* pFont = m_pDialog->GetFont(pElement->iFont);
	//if (pFont)
	//{
	int curY = rcDropdownText.top;
	int nRemainingHeight = HEIGHTOF(rcDropdownText);
	//WCHAR strDropdown[4096] = {0};

	for (int i = scrollBar.GetTrackPos(); i < items.GetSize(); i++)
	{
		GUIComboBoxItem* pItem = items.GetAt(i);

		// Make sure there's room left in the dropdown
		nRemainingHeight -= /*pFont->nHeight*/nRowHeight;
		if (nRemainingHeight < 0)
		{
			pItem->bVisible = false;
			continue;
		}

		SetRect(&pItem->rcActive, rcDropdownText.left, curY, rcDropdownText.right, curY + /*pFont->nHeight*/nRowHeight);
		curY += /*pFont->nHeight*/nRowHeight;

		//debug
		//int blue = 50 * i;
		//m_pDialog->DrawRect( &pItem->rcActive, 0xFFFF0000 | blue );

		pItem->bVisible = true;

		if (bOpened)
		{
			if ((int)i == iFocused)
			{
				RECT rc;
				SetRect(&rc, rcDropdown.left, pItem->rcActive.top - 2, rcDropdown.right,
					pItem->rcActive.bottom + 2);
				//m_pDialog->DrawSprite(pSelectionElement, &rc, DXUT_NEAR_BUTTON_DEPTH);
				//m_pDialog->DrawText(pItem->strText, pSelectionElement, &pItem->rcActive);

				pMemDC->FillSolidRect(&rc, RGB(0, 0, 255));
				pMemDC->DrawTextW(pItem->strText, &pItem->rcActive, TEXTFORMAT_CENTER);
			}
			else
			{
				//m_pDialog->DrawText(pItem->strText, pElement, &pItem->rcActive);
				pMemDC->DrawTextW(pItem->strText, &pItem->rcActive, TEXTFORMAT_CENTER);
			}
		}
	}
	//}

	int nOffsetX = 0;
	int nOffsetY = 0;

	iState = /*DXUT_STATE_NORMAL*/1;

	if (bVisible == false)
		iState = /*DXUT_STATE_HIDDEN*/0;
	else if (bDisabled == true)
		iState = /*DXUT_STATE_DISABLED*/0;
	else if (state == GUI_STATE_DOWN)
	{
		iState = /*DXUT_STATE_PRESSED*/2;

		nOffsetX = 1;
		nOffsetY = 2;
	}
	else if (/*m_bMouseOver*/state == GUI_STATE_OVER)
	{
		iState = /*DXUT_STATE_MOUSEOVER*/2;

		nOffsetX = -1;
		nOffsetY = -2;
	}
	/*else if (m_bHasFocus)
	iState = DXUT_STATE_FOCUS;*/

	//float fBlendRate = (iState == DXUT_STATE_PRESSED) ? 0.0f : 0.8f;

	// Button
	//pElement = m_Elements.GetAt(1);

	// Blend current color
	//pElement->TextureColor.Blend(iState, fElapsedTime, fBlendRate);

	RECT rcWindow = rcButton;
	OffsetRect(&rcWindow, nOffsetX, nOffsetY);
	//m_pDialog->DrawSprite(pElement, &rcWindow, DXUT_FAR_BUTTON_DEPTH);
	if (picButton)
		picButton->Show(pMemDC, rcButton.left, rcButton.top, curAlpha, true, pMemDCBack);

	if (bOpened)
		iState = /*DXUT_STATE_PRESSED*/1;

	// Main text box
	//TODO: remove magic numbers
	//pElement = m_Elements.GetAt(0);

	// Blend current color
	//pElement->TextureColor.Blend(iState, fElapsedTime, fBlendRate);
	//pElement->FontColor.Blend(iState, fElapsedTime, fBlendRate);

	//m_pDialog->DrawSprite(pElement, &m_rcText, DXUT_NEAR_BUTTON_DEPTH);

	if (iSelected >= 0 && iSelected < (int)items.GetSize())
	{
		GUIComboBoxItem* pItem = items.GetAt(iSelected);
		if (pItem != NULL)
		{
			//m_pDialog->DrawText(pItem->strText, pElement, &m_rcText);
			pMemDC->DrawTextW(pItem->strText, &rcText, TEXTFORMAT_CENTER);
		}
	}

	// 显示到目标
	pDestDC->BitBlt(rcBoundingBox.left, rcBoundingBox.top, WIDTHOF(rcBoundingBox), HEIGHTOF(rcBoundingBox)
		, pMemDC, rcBoundingBox.left, rcBoundingBox.top, SRCCOPY);
	if (bOpened || lastOpend != bOpened)
	{
		RECT rc = rcDropdown;
		rc.right += nSBWidth;
		pDestDC->BitBlt(rc.left, rc.top, WIDTHOF(rc), HEIGHTOF(rc)
			, pMemDC, rc.left, rc.top, SRCCOPY);
	}

	lastOpend = bOpened;

	return true;
}

void GDIGUIComboBox::UpdateRects()
{
	GDIGUIControl::UpdateRects();

	rcButton = rcBoundingBox;
	rcButton.left = rcButton.right - HEIGHTOF(rcButton);

	rcText = rcBoundingBox;
	rcText.right = rcButton.left;

	rcDropdown = rcText;
	OffsetRect(&rcDropdown, 0, (int)(0.90f * HEIGHTOF(rcText)));
	rcDropdown.bottom += nDropHeight;
	rcDropdown.right -= nSBWidth;

	rcDropdownText = rcDropdown;
	rcDropdownText.left += (int)(0.1f * WIDTHOF(rcDropdown));
	rcDropdownText.right -= (int)(0.1f * WIDTHOF(rcDropdown));
	rcDropdownText.top += (int)(0.1f * HEIGHTOF(rcDropdown));
	rcDropdownText.bottom -= (int)(0.1f * HEIGHTOF(rcDropdown));

	// Update the scrollbar's rects
	scrollBar.SetLocation(rcDropdown.right, rcDropdown.top + 2);
	scrollBar.SetSize(nSBWidth, HEIGHTOF(rcDropdown) - 2);
	//DXUTFontNode* pFontNode = pDialog->GetManager()->GetFontNode(Elements.GetAt(2)->iFont);
	//if (pFontNode && pFontNode->nHeight)
	//{
	scrollBar.SetPageSize(HEIGHTOF(rcDropdownText) / nRowHeight/*pFontNode->nHeight*/);

	// The selected item may have been scrolled off the page.
	// Ensure that it is in page again.
	scrollBar.ShowItem(iSelected);
	//}
}

HRESULT GDIGUIComboBox::AddItem(const WCHAR * strText, void * pData)
{
	// Validate parameters
	if (strText == NULL)
	{
		return E_INVALIDARG;
	}

	// Create a new item and set the data
	GUIComboBoxItem* pItem = new GUIComboBoxItem;
	if (pItem == NULL)
	{
		return /*DXTRACE_ERR_MSGBOX(L"new", E_OUTOFMEMORY)*/E_OUTOFMEMORY;
	}

	ZeroMemory(pItem, sizeof(GUIComboBoxItem));
	wcscpy_s(pItem->strText, 256, strText);
	pItem->pData = pData;

	items.Add(pItem);

	// Update the scroll bar with new range
	scrollBar.SetTrackRange(0, items.GetSize());

	// If this is the only item in the list, it's selected
	if (GetNumItems() == 1)
	{
		iSelected = 0;
		iFocused = 0;
		//pDialog->SendEvent(EVENT_COMBOBOX_SELECTION_CHANGED, false, this);
	}

	return S_OK;
}

void GDIGUIComboBox::RemoveAllItems()
{
	for (int i = 0; i < items.GetSize(); i++)
	{
		GUIComboBoxItem* pItem = items.GetAt(i);
		SAFE_DELETE(pItem);
	}

	items.RemoveAll();
	scrollBar.SetTrackRange(0, 1);
	iFocused = iSelected = -1;
}

void GDIGUIComboBox::RemoveItem(UINT index)
{
	for (int i = 0; i < items.GetSize(); i++)
	{
		GUIComboBoxItem* pItem = items.GetAt(i);
		SAFE_DELETE(pItem);
	}

	items.RemoveAll();
	scrollBar.SetTrackRange(0, 1);
	iFocused = iSelected = -1;
}

bool GDIGUIComboBox::ContainsItem(const WCHAR * strText, UINT iStart)
{
	return (-1 != FindItem(strText, iStart));
}

int GDIGUIComboBox::FindItem(const WCHAR * strText, UINT iStart)
{
	if (strText == NULL)
		return -1;

	for (int i = iStart; i < items.GetSize(); i++)
	{
		GUIComboBoxItem* pItem = items.GetAt(i);

		if (0 == wcscmp(pItem->strText, strText))
		{
			return i;
		}
	}

	return -1;
}

GDIGUIEdit::GDIGUIEdit()
{
	type = GUI_CONTROL_EDIT;

	textFormat = TEXTFORMAT_LEFT;
}

bool GDIGUIEdit::Render(LPGDIDevice dev)
{
	if (!bVisible)
		return false;

	if (!dev || dev->IsDCNull())
		return false;

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();
	CDC *pMemDCBack = dev->GetMemDCBack();
	if (pDestDC == NULL || pMemDC == NULL || pMemDCBack == NULL)
		return false;

	// 确定贴图

	// 清除背景
	RECT rcUnion;
	UnionRect(&rcUnion, &rcBoundingBox, &rcText);//控件区域和文本区域并
												 //pMemDC->FillSolidRect(&rcUnion, COLORGDI_DEFAULT);//清除背景

												 // 绘制贴图
	if (pic)
		pic->Show(pMemDC, posX, posY, ROUNDF_BYTE(alpha), true, pMemDCBack);

	// 绘制文本
	RenderText(pMemDC, textFormat);

	// 显示到目标
	pDestDC->BitBlt(rcUnion.left, rcUnion.top, WIDTHOF(rcUnion), HEIGHTOF(rcUnion)
		, pMemDC, rcUnion.left, rcUnion.top, SRCCOPY);

	return true;
}

#endif // USE_GDIGUI