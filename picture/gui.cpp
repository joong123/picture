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
	case 0x88760B59:
		wcscat_s(errormsg, L"0x88760B59");
		break;
	default:
		wcscat_s(errormsg, L"UNKNOWN");
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
	memcpy((*dest), src, len * 2);
	(*dest)[len] = (WCHAR)'\0';
	/*(*dest)[len] = 0;
	wcscpy_s(*dest, len, src);*/

	return true;
}

#ifdef USE_GDIGUI
bool SmoothLine(CDC * pDC, const POINT & start, const POINT & end, const DWORD & color, const DWORD &backcolor)
{
	if (pDC == NULL)
		return false;

	// smooth part
	if ((start.x != end.x) || (start.y != end.y))
	{
		POINT ps, pe;
		if (start.x < end.x || start.y < end.y)
		{
			ASIGNPOINT(ps, start);
			ASIGNPOINT(pe, end);
		}
		else if (start.x > end.x || start.y > end.y)
		{
			ASIGNPOINT(ps, end);
			ASIGNPOINT(pe, start);
		}
		else
		{
			//return true;
		}

		if (end.x == start.x)
		{
			pDC->MoveTo(start);
			pDC->LineTo(end);
			return true;
		}

		const float alphabase = 1.0f;
		const float alphabase2 = 1.0f;
		if ((pe.y - ps.y > pe.x - ps.x) || (pe.y - ps.y < ps.x - pe.x))
		{
			int delta = (pe.y > ps.y) ? 1 : -1;
			float slope = (pe.y - ps.y) / (float)(pe.x - ps.x);
			float temp1 = ps.x*slope - ps.y;
			for (int i = ps.y; i != pe.y; i += delta)
			{
				float xpos = (i + temp1) / slope;
				float xbias = xpos - (int)xpos;
				if (xbias < 0)
					xbias += 1;
				if (xbias >= 0.5f)
				{
					DWORD blendcolor = 0;
					byte *dest = ((byte*)&blendcolor);
					byte *cs = (byte*)&color;
					byte *cb = (byte*)&backcolor;
					float alpha = alphabase * (1 - xbias);
					float alpha2 = 1 - alpha;

					dest[2] = (byte)(cs[2] * alpha + cb[2] * alpha2);
					dest[1] = (byte)(cs[1] * alpha + cb[1] * alpha2);
					dest[0] = (byte)(cs[0] * alpha + cb[0] * alpha2);

					if (pDC->GetPixel({ (LONG)ceil(xpos) - 1,i }) > blendcolor)
						pDC->SetPixel({ (LONG)ceil(xpos) - 1,i }, blendcolor);

					alpha = xbias;
					alpha2 = 1 - alpha;
					dest[2] = (byte)(cs[2] * alpha + cb[2] * alpha2);
					dest[1] = (byte)(cs[1] * alpha + cb[1] * alpha2);
					dest[0] = (byte)(cs[0] * alpha + cb[0] * alpha2);

					if (pDC->GetPixel({ (LONG)xpos + 1,i }) > blendcolor)
						pDC->SetPixel({ (LONG)xpos + 1,i }, blendcolor);
				}
				else if (xbias >= 0.0f)
				{
					DWORD blendcolor = 0;
					byte *dest = ((byte*)&blendcolor);
					byte *cs = (byte*)&color;
					byte *cb = (byte*)&backcolor;
					float alpha = alphabase*xbias;
					float alpha2 = 1 - alpha;

					dest[2] = (byte)(cs[2] * alpha + cb[2] * alpha2);
					dest[1] = (byte)(cs[1] * alpha + cb[1] * alpha2);
					dest[0] = (byte)(cs[0] * alpha + cb[0] * alpha2);

					alpha = (1 - xbias);
					alpha2 = 1 - alpha;
					if (pDC->GetPixel({ (LONG)xpos + 1,i }) > blendcolor)
						pDC->SetPixel({ (LONG)xpos + 1,i }, blendcolor);

					dest[2] = (byte)(cs[2] * alpha + cb[2] * alpha2);
					dest[1] = (byte)(cs[1] * alpha + cb[1] * alpha2);
					dest[0] = (byte)(cs[0] * alpha + cb[0] * alpha2);

					if (pDC->GetPixel({ (LONG)ceil(xpos) - 1 + (xbias == 0),i }) > blendcolor)
						pDC->SetPixel({ (LONG)ceil(xpos) - 1 + (xbias == 0),i }, blendcolor);
				}
				else
				{
					DWORD blendcolor = 0;
					byte *dest = ((byte*)&blendcolor);
					byte *cs = (byte*)&color;
					byte *cb = (byte*)&backcolor;
					float alpha;
					if (slope > 0)
						alpha = alphabase2*(0.5f + abs(atanf(slope) - PI_F / 4) / PI_F * 2);
					else
						alpha = alphabase2*(0.5f + abs(atanf(slope) + PI_F / 4) / PI_F * 2);

					float alpha2 = 1 - alpha;

					dest[2] = (byte)(cs[2] * alpha + cb[2] * alpha2);
					dest[1] = (byte)(cs[1] * alpha + cb[1] * alpha2);
					dest[0] = (byte)(cs[0] * alpha + cb[0] * alpha2);

					alpha = alpha2 / 2.0f;
					alpha2 = 1 - alpha;
					if (pDC->GetPixel({ (LONG)xpos,i }) > blendcolor)
						pDC->SetPixel({ (LONG)xpos,i }, blendcolor);

					dest[2] = (byte)(cs[2] * alpha + cb[2] * alpha2);
					dest[1] = (byte)(cs[1] * alpha + cb[1] * alpha2);
					dest[0] = (byte)(cs[0] * alpha + cb[0] * alpha2);

					if (pDC->GetPixel({ (LONG)xpos - 1,i }) > blendcolor)
						pDC->SetPixel({ (LONG)xpos - 1,i }, blendcolor);
					if (pDC->GetPixel({ (LONG)xpos + 1,i }) > blendcolor)
						pDC->SetPixel({ (LONG)xpos + 1,i }, blendcolor);
				}
			}
		}
		else
		{
			int delta = (pe.x > ps.x) ? 1 : -1;
			float slope = (pe.y - ps.y) / (float)(pe.x - ps.x);
			float temp1 = ps.y - ps.x*slope;
			for (int i = ps.x; i != pe.x; i += delta)
			{
				float ypos = i*slope + temp1;
				float ybias = ypos - (int)ypos;
				if (ybias < 0)
					ybias += 1;
				if (ybias < 0.5f)
				{
					DWORD blendcolor = color;
					byte *dest = ((byte*)&blendcolor);
					byte *cs = (byte*)&color;
					byte *cb = (byte*)&backcolor;
					float alpha = ybias * alphabase;
					float alpha2 = 1 - alpha;

					dest[2] = (byte)(cs[2] * alpha + cb[2] * alpha2);
					dest[1] = (byte)(cs[1] * alpha + cb[1] * alpha2);
					dest[0] = (byte)(cs[0] * alpha + cb[0] * alpha2);

					if (pDC->GetPixel({ i,(LONG)ypos + 1 }) > blendcolor)
						pDC->SetPixel({ i,(LONG)ypos + 1 }, blendcolor);

					alpha = (1 - ybias);
					alpha2 = 1 - alpha;
					dest[2] = (byte)(cs[2] * alpha + cb[2] * alpha2);
					dest[1] = (byte)(cs[1] * alpha + cb[1] * alpha2);
					dest[0] = (byte)(cs[0] * alpha + cb[0] * alpha2);

					if (pDC->GetPixel({ i,(LONG)ceil(ypos) - 1 + (ybias == 0) }) > blendcolor)
						pDC->SetPixel({ i,(LONG)ceil(ypos) - 1 + (ybias == 0) }, blendcolor);
				}
				else
				{
					DWORD blendcolor = color;
					byte *dest = ((byte*)&blendcolor);
					byte *cs = (byte*)&color;
					byte *cb = (byte*)&backcolor;
					float alpha = alphabase * (1 - ybias);
					float alpha2 = 1 - alpha;

					dest[2] = (byte)(cs[2] * alpha + cb[2] * alpha2);
					dest[1] = (byte)(cs[1] * alpha + cb[1] * alpha2);
					dest[0] = (byte)(cs[0] * alpha + cb[0] * alpha2);

					if (pDC->GetPixel({ i,(LONG)ceil(ypos) - 1 }) > blendcolor)
						pDC->SetPixel({ i,(LONG)ceil(ypos) - 1 }, blendcolor);

					alpha = ybias;
					alpha2 = 1 - alpha;
					dest[2] = (byte)(cs[2] * alpha + cb[2] * alpha2);
					dest[1] = (byte)(cs[1] * alpha + cb[1] * alpha2);
					dest[0] = (byte)(cs[0] * alpha + cb[0] * alpha2);

					if (pDC->GetPixel({ i,(LONG)ypos + 1 }) > blendcolor)
						pDC->SetPixel({ i,(LONG)ypos + 1 }, blendcolor);
				}
			}
		}
	}

	return true;
}
#endif // USE_GDIGUI

CGUIControl::CGUIControl()
{
	if (!frequency.QuadPart)
		QueryPerformanceFrequency(&frequency);

	type = GUI_CONTROL_NULL;
	ID = 0;

	color = 0x00000000;
	dockMode = GUI_WINDOCK_NORMAL;
	dockX = 0;
	dockY = 0;
	displayDx = 0;
	displayDy = 0;
	width = 0;
	height = 0;
	posX = 0;
	posY = 0;

	inputPos = 0;
	text = NULL;
	textColor = COLOR_WHITE;
	textRect = RECT(0, 0, 0, 0);

	bDisabled = false;
	bVisible = true;

	state = GUI_STATE_OUT;

	displayEvent = GUI_EVENT_NULL;
	lastTick = { 0 };
	alpha = 0;
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
	SAFE_DELETE_LIST(text)

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

	mywcscpy(&text, str);
	return true;
}

void CGUIControl::KillFocus()
{
	POINT cursor;
	GetCursorPos(&cursor);
	bool inside = (cursor.x > posX) && (cursor.x < posX + width) &&
		(cursor.y > posY) && (cursor.y < posY + height);

	Translation(0 - displayDx, 0 - displayDy);

	if (inside)
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

void CGUIControl::SetEnabled(bool bEnabled)
{
	bDisabled = !bEnabled;
}

void CGUIControl::SetVisible(bool bVisible)
{
	this->bVisible = bVisible;
}

void CGUIControl::Translation(int dx, int dy)
{
	displayDx += dx;
	displayDy += dy;
}

void CGUIControl::Dock(UINT * pbufw, UINT * pbufh)
{
	UINT bufw = SAFE_POINTERVALUE_0(pbufw);
	UINT bufh = SAFE_POINTERVALUE_0(pbufh);

	if (dockMode == GUI_WINDOCK_NORMAL)
	{
		posX = ROUND(dockX);
		posY = ROUND(dockY);
	}
	else if (dockMode == GUI_WINDOCK_RIGHT)
	{
		posX = ROUND((float)bufw - width - dockX);
		posY = ROUND(dockY);
	}
	else if (dockMode == GUI_WINDOCK_BOTTOM)
	{
		posX = ROUND(dockX);
		posY = ROUND((float)bufh - height - dockY);
	}
	else if (dockMode == GUI_WINDOCK_BOTTOMRIGHT)
	{
		posX = ROUND((float)bufw - width - dockX);
		posY = ROUND((float)bufh - height - dockY);
	}
	else if (dockMode == GUI_WINDOCK_BOTTOMHSPAN)
	{
		width = (int)bufw;
		posX = 0;
		posY = ROUND((float)bufh - height - dockY);
	}
	else if (dockMode == GUI_WINDOCK_FULLSCREEN)
	{
		width = (int)bufw;
		height = (int)bufh;
		posX = 0;
		posY = 0;
	}
	else if (dockMode == GUI_WINDOCK_SCALE)
	{
		width = ROUND(bufw*dockW);
		height = ROUND(bufh*dockH);
		posX = ROUND(dockX*bufw);
		posY = ROUND(dockY*bufh);
	}
	textRect = RECT((LONG)posX, (LONG)posY, (LONG)width, (LONG)height);// 更新文本显示区域
}

bool CGUIControl::HandleKeyboard(WPARAM wParam, bool isIME)
{
	if (bDisabled || !bVisible)
		return false;

	if (wParam == '\b')
	{
		if (inputPos > 0)
		{
			text[--inputPos] = 0;
		}
	}
	else if (wParam == '\r')
	{
		return true;
	}
	else if (inputPos < GUI_DEFAULTTEXTLEN)
	{
		if (isIME)
		{
			text[inputPos++] = wParam;
			text[inputPos] = 0;
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

					text[inputPos++] = wParam;
				text[inputPos] = 0;
			}
			else if (wParam == 190)
			{
				text[inputPos++] = '.';
				text[inputPos] = 0;
			}
		}
	}
	return false;
}

byte CGUIControl::HandleMouse(byte LMBState, LONG mouseX, LONG mouseY)
{
	if (bDisabled || !bVisible)
		return GUI_EVENT_NULL;

	byte theevent = GUI_EVENT_NULL;
	byte newstate = state;
	bool inside = (mouseX > posX) && (mouseX < posX + width) &&
		(mouseY > posY) && (mouseY < posY + height);

	// 鼠标事件处理
	if (inside)
	{
		if (state == GUI_STATE_OUT)
		{
			theevent = GUI_EVENT_ENTER;
			newstate = GUI_STATE_OVER;
		}
	}
	else
	{
		if (state == GUI_STATE_OVER)
		{
			theevent = GUI_EVENT_LEAVE;
			newstate = GUI_STATE_OUT;
		}
	}

	switch (LMBState) {
	case GUI_MOUSEEVENT_DOWN:
		if (inside && state != GUI_STATE_DOWN)
		{
			theevent = GUI_EVENT_DOWN;
			newstate = GUI_STATE_DOWN;
		}
		break;
	case GUI_MOUSEEVENT_UP:
		if (state == GUI_STATE_DOWN)
		{
			if (inside)
			{
				theevent = GUI_EVENT_UP;
				newstate = GUI_STATE_OVER;
			}
			else
			{
				theevent = GUI_EVENT_OUTUP;
				newstate = GUI_STATE_OUT;
			}
		}
		break;
	}

	// 状态转移
	state = newstate;

	return theevent;
}

void CGUIControl::HandleCMD(UINT cmd)
{
}


#ifdef USE_D3DGUI
D3DGUIControl::D3DGUIControl()
{
	font = NULL;

	vbuffer = NULL;

	m_outTex = NULL;
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
	SAFE_RELEASE(font)

		SAFE_RELEASE(vbuffer)

		SAFE_RELEASE(m_outTex)

		CGUIControl::Release();

	return true;
}

void D3DGUIControl::Translation(int dx, int dy)
{
	CGUIControl::Translation(dx, dy);

	RefreshVertexBuffer();
	//if (!vbuffer) 
	//	return;

	//D3DVERTEXBUFFER_DESC vbd;
	//vbuffer->GetDesc(&vbd);

	//void *ptr;
	//if (FAILED(vbuffer->Lock(0, vbd.Size, (void**)&ptr, 0)))
	//	return;
	//*(float*)((byte*)ptr) += dx;//改坐标
	//*(float*)((byte*)ptr + sizeof(float)) += dy;
	//*(float*)((byte*)ptr + sizeof(D3DGUIVertex)) += dx;
	//*(float*)((byte*)ptr + sizeof(D3DGUIVertex) + sizeof(float)) += dy;
	//*(float*)((byte*)ptr + 2 * sizeof(D3DGUIVertex)) += dx;
	//*(float*)((byte*)ptr + 2 * sizeof(D3DGUIVertex) + sizeof(float)) += dy;
	//*(float*)((byte*)ptr + 3 * sizeof(D3DGUIVertex)) += dx;
	//*(float*)((byte*)ptr + 3 * sizeof(D3DGUIVertex) + sizeof(float)) += dy;
	//vbuffer->Unlock();
}

void D3DGUIControl::KillFocus()
{
	CGUIControl::KillFocus();

	//Translation(0 - displaydx, 0 - displaydy);
}

bool D3DGUIControl::RefreshVertexBuffer()
{
	if (vbuffer)
	{
		D3DGUIVertex obj[] =
		{
			{ (float)width + posX + displayDx, (float)posY + displayDy, 0.0f, 1.0f, color, 1.0f, 0.0f },
			{ (float)width + posX + displayDx, (float)height + posY + displayDy, 0.0f, 1.0f, color, 1.0f, 1.0f },
			{ (float)posX + displayDx, (float)posY + displayDy, 0.0f, 1.0f, color, 0.0f, 0.0f },
			{ (float)posX + displayDx, (float)height + posY + displayDy, 0.0f, 1.0f, color, 0.0f, 1.0f },
		};
		// Fill the vertex buffer.
		void *ptr;
		if (FAILED(vbuffer->Lock(0, sizeof(obj), (void**)&ptr, 0)))
		{
			return false;
		}
		memcpy(ptr, obj, sizeof(obj));
		vbuffer->Unlock();

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
			&defaultfont);
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
		pbufferw = pd3dwnd->GetPBufferWidth();
		pbufferh = pd3dwnd->GetPBufferHeight();

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
		SAFE_RELEASE(fonts[i])
	}
	SAFE_DELETE_LIST(fonts)
		nFonts = 0;

	// 背景控件清除
	SAFE_RELEASEDELETE(pBackdrop)


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
		defaultfont->OnLostDevice();
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
		defaultfont->OnResetDevice();
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
			pBackdrop->Dock(pbufferw, pbufferh);
			pBackdrop->RefreshVertexBuffer();//刷新模型
		}

		// 其他控件重置
		controls[i]->Dock(pbufferw, pbufferh);// 控件重新停靠
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
	device = NULL;
	pbufferw = NULL;
	pbufferh = NULL;

	defaultfont = NULL;
	fonts = NULL;
	pBackdrop = NULL;
	bUseBackdrop = true;

	nFonts = 0;
	nControls = 0;

	lastLMBdown = false;
	pFocusControl = NULL;
	block = false;

	pEventProc = NULL;
}

bool CD3DGUISystem::ControlListExpand()
{
	/*if (!controls)
	{
	controls = new D3DGUIControl*[1];
	if (!controls) return false;
	controls[0] = new D3DGUIControl;

	nControls = 1;
	}
	else
	{
	D3DGUIControl **temp;
	temp = new D3DGUIControl*[nControls + 1];
	if (!temp) return false;
	for (int i = 0; i < nControls; i++)
	{
	temp[i] = controls[i];
	}
	delete[] controls;
	controls = temp;
	controls[nControls] = new D3DGUIControl;

	nControls++;
	}*/

	return true;
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

bool CD3DGUISystem::AddBackdrop(WCHAR *TexFileName, float x, float y, float width, float height, byte dock)
{
	NULL_RETURN_FALSE(device);

	if (pBackdrop)
		SAFE_RELEASEDELETE(pBackdrop)
		pBackdrop = new D3DGUIBack;
	bool succeed = true;

	pBackdrop->color = COLOR_WHITE;// 创建vertexbuffer用
	pBackdrop->dockMode = dock;
	pBackdrop->dockX = x;
	pBackdrop->dockY = y;
	pBackdrop->dockW = width;
	pBackdrop->dockH = height;
	pBackdrop->width = ROUND(width);
	pBackdrop->height = ROUND(height);
	pBackdrop->Dock(pbufferw, pbufferh);

	pBackdrop->state = GUI_STATE_OUT;

	D3DXCreateTextureFromFile(device, TexFileName, &pBackdrop->m_outTex);
	if (FAILED(device->CreateVertexBuffer(sizeof(D3DGUIVertex) * 4, 0, D3DFVF_GUI,
		D3DPOOL_SYSTEMMEM, &pBackdrop->vbuffer, NULL)))
		succeed = false;
	pBackdrop->RefreshVertexBuffer();

	if (succeed)
	{
		return true;
	}
	else
	{
		SAFE_DELETE(pBackdrop)
			return false;
	}
}

bool CD3DGUISystem::AddStatic(int ID, float x, float y, float width, float height, WCHAR *text, DWORD color, int fontID, byte dock)
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
	pStatic->width = ROUND(width);
	pStatic->height = ROUND(height);
	pStatic->Dock(pbufferw, pbufferh);

	pStatic->SetText(text);
	pStatic->textColor = color;
	pStatic->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : defaultfont;

	pStatic->state = GUI_STATE_OUT;
}

bool CD3DGUISystem::AddButton(int ID, float x, float y, float width, float height, WCHAR *text, DWORD color, int fontID, byte dock, WCHAR *up, WCHAR *over, WCHAR *down)
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
	pButton->width = ROUND(width);
	pButton->height = ROUND(height);
	pButton->Dock(pbufferw, pbufferh);

	pButton->SetText(text);
	pButton->textColor = color;
	pButton->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : defaultfont;

	pButton->overdx = -1;
	pButton->overdy = -1;
	pButton->downdx = 1;
	pButton->downdy = 1;
	pButton->state = GUI_STATE_OUT;
	pButton->alpha = 0;
	pButton->displayEvent = GUI_EVENT_NULL;

	D3DXCreateTextureFromFile(device, up, &pButton->m_outTex);
	D3DXCreateTextureFromFile(device, over, &pButton->m_overTex);
	D3DXCreateTextureFromFile(device, down, &pButton->m_downTex);

	// Create the vertex buffer.
	if (FAILED(device->CreateVertexBuffer(sizeof(D3DGUIVertex) * 4, 0,
		D3DFVF_GUI, D3DPOOL_MANAGED, &pButton->vbuffer, NULL)))
	{
		SAFE_DELETE(pButton);
		return false;
	}
	pButton->RefreshVertexBuffer();

	return true;
}

bool CD3DGUISystem::AddEdit(int ID, float x, float y, float width, float height, DWORD color, DWORD txtcolor, int fontID, byte dock)
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
	pEdit->width = ROUND(width);
	pEdit->height = ROUND(height);
	pEdit->Dock(pbufferw, pbufferh);
	pEdit->displayDx = 0;
	pEdit->displayDy = 0;

	pEdit->text = new WCHAR[GUI_DEFAULTTEXTLEN + 1];
	pEdit->text[0] = L'\0';
	pEdit->inputPos = 0;
	pEdit->textColor = txtcolor;
	pEdit->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : defaultfont;

	pEdit->state = GUI_STATE_OUT;
	pEdit->alpha = 0;
	pEdit->displayEvent = GUI_EVENT_NULL;

	// Create the vertex buffer.
	if (FAILED(device->CreateVertexBuffer(sizeof(D3DGUIVertex) * 4, 0,
		D3DFVF_GUI, D3DPOOL_SYSTEMMEM, &pEdit->vbuffer, NULL)))
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

int CD3DGUISystem::GetState(int ID)
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
	NULL_RETURN_FALSE(text)

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

LPD3DXFONT CD3DGUISystem::GetFont(int ID)
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
		block = false;
		pFocusControl->KillFocus();

		return true;
	}

	return false;
}

void CD3DGUISystem::SetEventProc(LPGUIEVENTCALLBACK pevent)
{
	pEventProc = pevent;
}

void CD3DGUISystem::HandleMouse(bool LMBDown, LONG mouseX, LONG mouseY)
{
	byte LMBstate = (byte)lastLMBdown + (byte)(LMBDown << 1);
	lastLMBdown = LMBDown;
	bool controldown = false;//是否有控件按下
	for (int i = 0; i < nControls; i++)
	{
		if (!block || ISFOCUS(i))//不阻塞或者第i个控件就是焦点控件
		{
			if (!controls[i]->bDisabled && controls[i]->bVisible)// 控件处于使能状态
			{
				int event = controls[i]->HandleMouse(LMBstate, mouseX, mouseY);

				// 控件系统调度
				if (event == GUI_EVENT_DOWN)//按下的按钮阻塞其他按钮
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
				else if (event == GUI_EVENT_OUTUP || event == GUI_EVENT_UP)
					BLOCK_OFF;

				if (event && pEventProc)//回调
				{
					pEventProc(controls[i]->GetID(), event, 0);
				}
			}
		}
	}
}

void CD3DGUISystem::HandleKeyboard(UINT8 keytype, WPARAM wParam)
{
	for (int i = 0; i < nControls; i++)
	{
		if (controls[i]->GetType() == GUI_CONTROL_EDIT && ISFOCUS(i))
		{
			if (!controls[i]->bDisabled && controls[i]->bVisible)
			{
				bool cmd = controls[i]->HandleKeyboard(wParam, keytype == GUI_KEYEVENT_IMECHAR);

				if (cmd && pEventProc)
				{
					pEventProc(controls[i]->GetID(), GUI_EVENT_CMD, (LPARAM)controls[i]->text);
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
	//LARGE_INTEGER tick = { 0 };
	//QueryPerformanceCounter(&tick);

	if (font && text)
		font->DrawTextW(NULL, text, -1, &textRect, TEXTFORMAT_LEFT, color);

	//lastTick.QuadPart = tick.QuadPart;

	return true;
}

D3DGUIButton::D3DGUIButton()
{
	type = GUI_CONTROL_BUTTON;

	m_overTex = NULL;
	m_downTex = NULL;
}

D3DGUIButton::~D3DGUIButton()
{
	Release();
}

bool D3DGUIButton::Release()
{
	SAFE_RELEASE(m_overTex)
		SAFE_RELEASE(m_downTex)

		D3DGUIControl::Release();

	return true;
}

byte D3DGUIButton::HandleMouse(byte LMBState, LONG mouseX, LONG mouseY)
{
	byte event = CGUIControl::HandleMouse(LMBState, mouseX, mouseY);
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
		Translation(ddx - displayDx, ddy - displayDy);
	}

	// 显示
	if (event)
	{
		//存入事件
		displayEvent = event;
		QueryPerformanceCounter(&lastTick);

		//事件起始设定
		if (displayEvent == GUI_EVENT_LEAVE)
			alpha = 255;
		else if (displayEvent == GUI_EVENT_UP)
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

	if (!vbuffer)
		return false;

	// 底层绘制
	dev->SetTexture(0, m_outTex);
	D3DGUI_RENDER_VBUFFER(dev, vbuffer);

	// 第二图层绘制
	// 不用多线程处理渐变
	float dAplha = 1000.0f*(tick.QuadPart - lastTick.QuadPart) / frequency.QuadPart *speed;
	if (displayEvent == GUI_EVENT_ENTER)
	{
		if (alpha != 255)
			alpha += min(255 - alpha, dAplha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_LEAVE)
	{
		if (alpha)
			alpha -= min(alpha, dAplha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_OUTUP)
	{
		if (alpha)
			alpha -= min(alpha, dAplha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_UP)
	{
		if (alpha != 255)
			alpha += min(255 - alpha, dAplha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_DOWN)
	{
		alpha = 255;
	}

	// 设置二层纹理
	if (displayEvent == GUI_EVENT_OUTUP
		|| displayEvent == GUI_EVENT_DOWN)
		dev->SetTexture(0, m_downTex);
	else
		dev->SetTexture(0, m_overTex);

	// 绘制第二图层
	ChangeAlpha(vbuffer, ROUND_BYTE(alpha));//设置实时透明度
	D3DGUI_RENDER_VBUFFER(dev, vbuffer);
	ChangeAlpha(vbuffer, 255);

	// 按钮文本绘制
	if (text && font)
	{
		textRect = { (LONG)(posX + displayDx), LONG(posY + displayDy - 1) //-1修正
			, LONG(posX + displayDx + width)
			, LONG(posY + displayDy + height) };
		//阴影绘制
		/*defaultfont->DrawTextW(NULL, pCnt->text
		, -1, &textregion, TEXTFORMAT_DEFAULT
		, SETALPHA(~pCnt->textcolor, 255));*/
		/*DWORD alphacolor = D3DCOLOR_ARGB(textcolor >> 24
		, ((textcolor >> 16) & 0xFF)*(255 - alpha) / 255
		, ((textcolor >> 8) & 0xFF)*(255 - alpha) / 255
		, (textcolor & 0xFF)*(255 - alpha) / 255);*/
		DWORD alphacolor = D3DCOLOR_ARGB(textColor >> 24
			, (BYTE_PART(textColor >> 16)*ROUND_BYTE(255 - alpha) + ROUND_BYTE(alpha)*(255 - BYTE_PART(textColor >> 16))) >> 8
			, (BYTE_PART(textColor >> 8)*ROUND_BYTE(255 - alpha) + ROUND_BYTE(alpha)*(255 - BYTE_PART(textColor >> 8))) >> 8
			, (BYTE_PART(textColor)*ROUND_BYTE(255 - alpha) + ROUND_BYTE(alpha)*(255 - BYTE_PART(textColor))) >> 8);
		/*textregion.bottom -= 2;
		textregion.right -= 2;*/
		font->DrawTextW(NULL, text
			, -1, &textRect, TEXTFORMAT_DEFAULT, alphacolor);
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
	D3DGUI_RENDER_VBUFFER(dev, vbuffer);

	if (text && font)
		font->DrawTextW(NULL, text, -1, &textRect, TEXTFORMAT_LEFT, textColor);

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

	dev->SetTexture(0, m_outTex);
	D3DGUI_RENDER_VBUFFER(dev, vbuffer);

	return true;
}
#endif // USE_D3DGUI


#ifdef USE_GDIGUI
GDIDevice::GDIDevice()
{
	pmDB = NULL;
	pdevDC = NULL;
}

GDIDevice::~GDIDevice()
{
	DeleteMemDCBMP(pmDB);
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

	return S_OK;
}
GDIGUIControl::GDIGUIControl()
{
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

void GDIGUIControl::DisplayCycle(LPGDIDevice dev)
{
	if (displayEvent != GUI_EVENT_NULL)
	{
		// 重绘
		Render(dev);
	}
}

bool GDIGUIControl::Render(LPGDIDevice dev)
{
	if (!dev || dev->IsDCNull())
	{
		return false;
	}

	return true;
}

GDIGUIStatic::GDIGUIStatic()
{
	type = GUI_CONTROL_STATIC;
}

bool GDIGUIStatic::Render(LPGDIDevice dev)
{
	if (!dev || dev->IsDCNull())
	{
		return false;
	}

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();

	RECT refreshrect = GDIRECT(posX, posY, width, height);
	pMemDC->FillSolidRect(&refreshrect, COLORGDI_DEFAULT);

	// 贴图
	if (pic)
		pic->Show_2(pMemDC, ROUND(posX), ROUND(posY), ROUND_BYTE(alpha), false);//测试

																				// 文本显示
	if (text)
	{
		CFont *pold = (CFont*)pMemDC->SelectObject(font);
		pMemDC->SetTextColor(textColor);

		pMemDC->SetBkMode(TRANSPARENT);
		int rtv = pMemDC->DrawTextW(text, &textRect, TEXTFORMAT_DEFAULT);
		pMemDC->SelectObject(pold);
		if (rtv == 0)//测试是否出错
			pMemDC->DrawTextW(text, &textRect, TEXTFORMAT_DEFAULT);
	}

	// 显示到目标
	pDestDC->BitBlt(posX, posY, width, height
		, pMemDC, posX, posY, SRCCOPY);

	return true;
}

GDIGUIButton::GDIGUIButton()
{
	type = GUI_CONTROL_BUTTON;

	picon = NULL;
	picdown = NULL;
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

byte GDIGUIButton::HandleMouse(byte LMBState, LONG mouseX, LONG mouseY)
{
	byte event = CGUIControl::HandleMouse(LMBState, mouseX, mouseY);

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
		Translation(ddx - displayDx, ddy - displayDy);
	}

	if (event)
	{
		//存入事件
		displayEvent = event;
		QueryPerformanceCounter(&lastTick);

		//事件起始设定
		if (displayEvent == GUI_EVENT_LEAVE)
			alpha = 255;
		else if (displayEvent == GUI_EVENT_UP)
			alpha = 0;
	}

	return event;
}

void GDIGUIButton::DisplayCycle(LPGDIDevice dev)
{
	const float speed = ALPHASPEED_DEFAULT;

	LARGE_INTEGER tick = { 0 };
	QueryPerformanceCounter(&tick);
	float dAlpha = 1000.0f*(tick.QuadPart - lastTick.QuadPart) / frequency.QuadPart *speed;

	if (displayEvent == GUI_EVENT_ENTER)
	{
		if (alpha != 255)
			alpha += min(255 - alpha, dAlpha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_LEAVE)
	{
		if (alpha)
			alpha -= min(alpha, dAlpha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_OUTUP)
	{
		if (alpha)
			alpha -= min(alpha, dAlpha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_UP)
	{
		if (alpha != 255)
			alpha += min(255 - alpha, dAlpha);
		else
			displayEvent = GUI_EVENT_NULL;
	}
	else if (displayEvent == GUI_EVENT_DOWN)
	{
		alpha = 255;
	}

	lastTick.QuadPart = tick.QuadPart;

	if (displayEvent)
		Render(dev);
}

bool GDIGUIButton::Render(LPGDIDevice dev)
{
	if (!dev || dev->IsDCNull())
	{
		return false;
	}

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();

	int rposx = ROUND(posX), rposy = ROUND(posY);//当前贴图坐标
	int basex = rposx, basey = rposy;//待刷新区域坐标
	int basew = ROUND(width), baseh = ROUND(height);//待刷新区域尺寸
	RECT rtxtrect = textRect;//实际文本位置矩形框
	memPic *secondpic = NULL;//二层贴图

							 // 确定第二层贴图 & 当前贴图实际位置
	if (state == GUI_STATE_OUT)
	{
		if (displayEvent == GUI_EVENT_LEAVE)
		{
			if (picon)
				secondpic = picon;
		}
		else if (displayEvent == GUI_EVENT_OUTUP)
		{
			if (picdown)
				secondpic = picdown;
		}
	}
	else if (state == GUI_STATE_OVER)
	{
		rposx += overdx;
		rposy += overdy;
		rtxtrect.left += overdx;
		rtxtrect.right += overdx;
		rtxtrect.bottom += overdy;
		rtxtrect.top += overdy;
		if (picon)
			secondpic = picon;
	}
	else if (state == GUI_STATE_DOWN)
	{
		rposx += downdx;
		rposy += downdy;
		rtxtrect.left += downdx;
		rtxtrect.right += downdx;
		rtxtrect.bottom += downdy;
		rtxtrect.top += downdy;
		if (picdown)
			secondpic = picdown;

	}

	//确定需要刷新的区域（当前贴图和上一次状态的贴图并集）
	if (displayEvent == GUI_EVENT_ENTER || displayEvent == GUI_EVENT_LEAVE)
	{
		if (overdx < 0)
			basex += overdx;
		if (overdy < 0)
			basey += overdy;
		basew += abs(overdx);
		baseh += abs(overdy);
	}
	else if (displayEvent == GUI_EVENT_OUTUP)
	{
		if (downdx < 0)
			basex += downdx;
		if (downdy < 0)
			basey += downdy;
		basew += abs(downdx);
		baseh += abs(downdy);
	}
	else if (displayEvent == GUI_EVENT_DOWN || displayEvent == GUI_EVENT_UP)
	{
		basex += min(min(overdx, downdx), 0);
		basey += min(min(overdy, downdy), 0);
		basew += max(max(abs(overdx), abs(downdx)), abs(overdx - downdx));
		baseh += max(max(abs(overdy), abs(downdy)), abs(overdy - downdy));
	}

	// 清除背景
	RECT refreshrect = GDIRECT(basex, basey, basew, baseh);
	pMemDC->FillSolidRect(&refreshrect, COLORGDI_DEFAULT);//清除背景

														  // 绘制两层贴图
	if (pic)
		pic->Show_2(pMemDC, rposx, rposy, 255, false);
	if (secondpic)
		secondpic->Show_2(pMemDC, rposx, rposy, ROUND_BYTE(alpha), false);

	// 文本
	if (text)
	{
		CFont *pold = (CFont*)pMemDC->SelectObject(font);
		DWORD alphacolor = RGB(
			(BYTE_PART(textColor >> 16)*ROUND_BYTE(255 - alpha) + ROUND_BYTE(alpha)*(255 - BYTE_PART(textColor >> 16))) >> 8
			, (BYTE_PART(textColor >> 8)*ROUND_BYTE(255 - alpha) + ROUND_BYTE(alpha)*(255 - BYTE_PART(textColor >> 8))) >> 8
			, (BYTE_PART(textColor)*ROUND_BYTE(255 - alpha) + ROUND_BYTE(alpha)*(255 - BYTE_PART(textColor))) >> 8);
		pMemDC->SetTextColor(alphacolor);
		pMemDC->SetBkMode(TRANSPARENT);

		pMemDC->DrawTextW(text, &rtxtrect, TEXTFORMAT_DEFAULT);
		pMemDC->SelectObject(pold);
	}

	// 显示到目标
	pDestDC->BitBlt(basex, basey, basew, baseh
		, pMemDC, basex, basey, SRCCOPY);

	return true;
}


bool GDIGUIWave::SetStopBackground()
{
	if (wavebuf == NULL)
		return false;
	if (wavebuf->pDC == NULL)
		return false;

	// 清除背景
	RECT refreshrect = GDIRECT(0, 0, width, height);
	wavebuf->pDC->FillSolidRect(&refreshrect, color);//清除背景

	CPen pen(PS_SOLID, 1, COLORGDI_MIDGREY);
	CPen* pOldPen = (CPen*)wavebuf->pDC->SelectObject(&pen);
	wavebuf->pDC->MoveTo({ 0,(LONG)(height / 2) });
	wavebuf->pDC->LineTo({ (LONG)width,(LONG)(height / 2) });
	wavebuf->pDC->SelectObject(pOldPen);

	return true;
}

GDIGUIWave::GDIGUIWave()
{
	type = GUI_CONTROL_WAVE;

	wavestate = GUIWAVE_STATE_STOPED;
	linecolor = COLORGDI_BLACK;

	starttime = { 0 };
	lasttime = 0;
	lastamp = 0;
	wavemoved = 0;
	curwavemoved = 0;

	time = 0;
	timespan = 2000;
	ampbase = 10000;
	amplist = NULL;
	wavebuf = NULL;
}

GDIGUIWave::~GDIGUIWave()
{
	Release();
}

bool GDIGUIWave::Release()
{
	SAFE_DELETE_LIST(amplist);
	if (wavebuf)
	{
		DeleteMemDCBMP(wavebuf);
		wavebuf = NULL;
	}

	GDIGUIControl::Release();

	return true;
}

void GDIGUIWave::PrepareMemDC(CDC * pdc)
{
	if (pdc)
	{
		if (wavebuf)
			delete wavebuf;

		wavebuf = new memDCBMP;
		MyDrawPrepareOne(pdc, wavebuf, 0, 0, CRect(0, 0, width, height));

		SetStopBackground();
	}
}

void GDIGUIWave::DisplayCycle(LPGDIDevice dev)
{
	if (wavestate == GUIWAVE_STATE_RUNNING)
	{
		LARGE_INTEGER tick = { 0 };
		QueryPerformanceCounter(&tick);

		int roundXBias = ROUND(width*1000.0f*(tick.QuadPart - starttime.QuadPart) / (float)frequency.QuadPart / timespan - wavemoved);
		curwavemoved = roundXBias;
		wavemoved += roundXBias;//总移动量

		if (wavebuf)
		{
			if (wavebuf->pDC)
			{
				TRANSLATION_PDC(wavebuf->pDC, width, height, -roundXBias, 0);
				RECT refreshrect = RECT(width - roundXBias, 0, roundXBias, height);
				wavebuf->pDC->FillSolidRect(&refreshrect, color);
			}
		}
		lastTick.QuadPart = tick.QuadPart;

		//if (displayEvent)
		Render(dev);
	}
	else if (displayEvent != GUI_EVENT_NULL)
	{
		if (wavestate == GUIWAVE_STATE_STOPED)
			SetStopBackground();

		Render(dev);
	}

	displayEvent = GUI_EVENT_NULL;
}

void GDIGUIWave::HandleCMD(UINT cmd)
{
	switch (cmd)
	{
	case GUI_CMD_START:
		if (wavestate == GUIWAVE_STATE_STOPED)
		{
			QueryPerformanceCounter(&starttime);
		}
		else if (wavestate == GUIWAVE_STATE_PAUSED)
		{
			LARGE_INTEGER tick;
			QueryPerformanceCounter(&tick);
			starttime.QuadPart += tick.QuadPart - lastTick.QuadPart;
		}
		wavestate = GUIWAVE_STATE_RUNNING;
		break;
	case GUI_CMD_PAUSE:
		if (wavestate == GUIWAVE_STATE_RUNNING)
		{
			wavestate = GUIWAVE_STATE_PAUSED;
			QueryPerformanceCounter(&lastTick);
		}
		else if (wavestate == GUIWAVE_STATE_PAUSED)
		{
			LARGE_INTEGER tick;
			QueryPerformanceCounter(&tick);
			starttime.QuadPart += tick.QuadPart - lastTick.QuadPart;
			wavestate = GUIWAVE_STATE_RUNNING;
		}
		break;
	case GUI_CMD_STOP:
		if (wavestate != GUIWAVE_STATE_STOPED)
		{
			wavestate = GUIWAVE_STATE_STOPED;
		}
		break;
	}
}

bool GDIGUIWave::Render(LPGDIDevice dev)
{
	if (!dev || dev->IsDCNull())
	{
		return false;
	}

	CDC *pDestDC = dev->GetDestDC();
	CDC *pMemDC = dev->GetMemDC();

	if (wavebuf)
	{
		if (wavebuf->pDC)
		{
			if (wavestate == GUIWAVE_STATE_RUNNING)
			{
				LARGE_INTEGER tick = { 0 };
				QueryPerformanceCounter(&tick);

				int testamp = MAKE_RANDOM(tick.QuadPart, -24, 24);
				CPen pen(PS_SOLID, 1, linecolor);
				CPen* pOldPen = (CPen*)wavebuf->pDC->SelectObject(&pen);
				SmoothLine(wavebuf->pDC
					, { (LONG)(width - curwavemoved - 1), (LONG)(height / 2 - lastamp) }
					, { (LONG)(width - 1), (LONG)(height / 2 - testamp) }
				, linecolor, color);
				wavebuf->pDC->SetPixel({ (LONG)(width - 1),(LONG)(height / 2 - testamp) }, linecolor);
				wavebuf->pDC->SelectObject(pOldPen);

				lastamp = (float)testamp;
			}

			/*WCHAR txt[56] = { 0 };
			swprintf_s(txt, L"%d-AMP:%d", curwavemoved, testamp);
			SetText(txt);*/

			pMemDC->BitBlt(posX, posY, width, height
				, wavebuf->pDC, 0, 0, SRCCOPY);
		}
	}

	if (text)
	{
		CFont *pold = (CFont*)pMemDC->SelectObject(font);
		pMemDC->SetTextColor(COLORGDI_BLACK);
		pMemDC->SetBkMode(TRANSPARENT);

		pMemDC->DrawTextW(text, &textRect, TEXTFORMAT_TOP);
		pMemDC->SelectObject(pold);
	}

	// 显示到目标
	pDestDC->BitBlt(posX, posY, width, height
		, pMemDC, posX, posY, SRCCOPY);

	return true;
}

void CGDIGUISystem::ThreadGUI(LPVOID lpParam)
{
	tpgui *ppgui = (tpgui*)lpParam;
	CGDIGUISystem *pguisys = (CGDIGUISystem*)ppgui->pguisys;
	GDIGUIControl **ppctrl;
	CDC *pDC;
	if (!pguisys)
	{
		return;
	}
	else
	{
		ppctrl = pguisys->controls;
		if (!pguisys->pdev)
			return;

		pDC = pguisys->pdev->GetDestDC();

		if (!pDC)
			return;
	}
	LPGDIDevice pdev = pguisys->pdev;

	while (true)
	{
		if (ppctrl != pguisys->controls)//刷新控件列表指针
			ppctrl = pguisys->controls;
		if (pguisys->nControls <= 0)//没有控件
		{
			Sleep(10);
			continue;
		}

		//扫描每个控件
		for (int i = 0; i < pguisys->nControls; i++)
		{
			GDIGUIControl *pctrl = ppctrl[i];

			byte dispevent = pctrl->displayEvent;//这次的显示事件

			if (!pctrl->bDisabled && pctrl->bVisible
				|| dispevent == GUI_EVENT_REFRESH)//控件处于使能状态或强制刷新（使能、可见切换）
			{
				pctrl->DisplayCycle(pdev);
			}
		}

		Sleep(20);
	}
}

bool CGDIGUISystem::ControlListExpand()
{
	if (!controls)
	{
		controls = new GDIGUIControl*[1];
		if (!controls) return false;
		controls[0] = new GDIGUIControl;

		nControls = 1;
	}
	else
	{
		GDIGUIControl **temp;
		temp = new GDIGUIControl*[nControls + 1];
		if (!temp) return false;
		for (int i = 0; i < nControls; i++)
		{
			temp[i] = controls[i];
		}
		delete[] controls;
		controls = temp;
		controls[nControls] = new GDIGUIControl;

		nControls++;
	}

	return true;
}

void CGDIGUISystem::VarInit()
{
	pdev = NULL;
	pbufferw = NULL;
	pbufferh = NULL;

	defaultfont = NULL;
	fonts = NULL;
	controls = NULL;
	backdrop = NULL;
	bUseBackdrop = true;

	nFonts = 0;
	nControls = 0;

	lastLMBdown = false;
	pFocusControl = NULL;
	block = false;

	pEventProc = NULL;
	hGUIThread = NULL;

	if (!defaultfont)
		defaultfont = new CFont;
	defaultfont->CreateFontW(
		14, 0, 0, 0, FW_NORMAL
		, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS
		, PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
}

void CGDIGUISystem::BeginGUIThread()
{
	threadParam.pguisys = this;
	hGUIThread = (HANDLE)_beginthread(ThreadGUI, 0, &threadParam); // 创建线程
}

bool CGDIGUISystem::AddControl(GDIGUIControl * pControl)
{
	if (ControlListExpand())
	{
		controls[nControls - 1] = pControl;
		return true;
	}

	return false;
}

CGDIGUISystem::CGDIGUISystem()
{
	VarInit();

	//BeginGUIThread();
}

CGDIGUISystem::CGDIGUISystem(LPGDIDevice dev)
{
	VarInit();
	pdev = dev;

	//BeginGUIThread();
}

CGDIGUISystem::~CGDIGUISystem()
{
	if (hGUIThread)
		_endthread();

	// 字体清除
	for (int i = 0; i < nFonts; i++)
	{
		if (fonts[i])
			fonts[i]->DeleteObject();//TODO
	}
	SAFE_DELETE_LIST(fonts)
		nFonts = 0;

	// 背景控件清除
	SAFE_RELEASEDELETE(backdrop)

		// 控件列表清除
		for (int i = 0; i < nControls; i++)
		{
			SAFE_DELETE(controls[i])
		}
	SAFE_DELETE_LIST(controls)
		nControls = 0;
}

CFont * CGDIGUISystem::GetFont(int ID)
{
	if (ID < 0 || ID >= nFonts) return NULL;
	return fonts[ID];
}

bool CGDIGUISystem::SetControlText(int ID, WCHAR * text)
{
	NULL_RETURN_FALSE(text)

		for (int i = 0; i < nControls; i++)
		{
			if (controls[i]->GetID() == ID)
			{
				controls[i]->SetText(text);

				controls[i]->inputPos = wcslen(text);

				//controls[i]->Render(pdev);//留到多线程中绘制
				controls[i]->displayEvent = GUI_EVENT_TEXTCHANGE;


				return true;
			}
		}
	return false;
}

bool CGDIGUISystem::AddGDIFont(WCHAR * fontName, int * fontID, INT Height, INT Width, INT Weight, BYTE Quality, int Escapement, int Orientation, bool Italics, bool Underline, bool StrikeOut, BYTE Charset, BYTE OutPrecision, BYTE ClipPrecision, BYTE PitchAndFamily)
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

bool CGDIGUISystem::AddStatic(int ID, float x, float y, float width, float height, WCHAR * text, DWORD color, int fontID, WCHAR *file, byte dock)
{
	if (!pdev)
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
	pStatic->Dock(pbufferw, pbufferh);

	pStatic->SetText(text);
	pStatic->textColor = color;
	pStatic->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : defaultfont;

	pStatic->state = GUI_STATE_OUT;
	pStatic->alpha = 255;

	//导入图片
	if (!pStatic->pic)
		pStatic->pic = new memPic();
	pStatic->pic->LoadImg(file, ROUND(width), ROUND(height));

	return true;
}

bool CGDIGUISystem::AddButton(int ID, float x, float y, float width, float height, WCHAR * text, DWORD color, int fontID, byte dock, WCHAR * up, WCHAR * on, WCHAR * down)
{
	if (!pdev)
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
	pButton->Dock(pbufferw, pbufferh);

	pButton->SetText(text);
	pButton->textColor = color;
	pButton->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : defaultfont;

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

bool CGDIGUISystem::AddWave(int ID, float x, float y, float width, float height, DWORD color, DWORD linecolor, byte dock, int fontID)
{
	if (!pdev)
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
	pWave->Dock(pbufferw, pbufferh);

	pWave->textColor = color;
	pWave->font = (fontID > 0 && GetFont(fontID - 1)) ? GetFont(fontID - 1) : defaultfont;

	pWave->state = GUI_STATE_OUT;
	pWave->linecolor = linecolor;
	pWave->alpha = 255;

	pWave->PrepareMemDC(pdev->GetDestDC());

	return true;
}

void CGDIGUISystem::SetEventProc(LPGUIEVENTCALLBACK pevent)
{
	pEventProc = pevent;
}

void CGDIGUISystem::HandleMouse(bool LMBDown, LONG mouseX, LONG mouseY)
{
	byte LMBstate = (byte)lastLMBdown + (byte)(LMBDown << 1);
	lastLMBdown = LMBDown;
	bool controldown = false;//是否有控件按下
	for (int i = 0; i < nControls; i++)
	{
		if (!block || ISFOCUS(i))//不阻塞或者第i个控件就是焦点控件
		{
			if (!controls[i]->bDisabled && controls[i]->bVisible)// 控件处于使能状态
			{
				byte event = controls[i]->HandleMouse(LMBstate, mouseX, mouseY);

				// 控件系统调度
				if (event == GUI_EVENT_DOWN)//按下的按钮阻塞其他按钮
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
				else if (event == GUI_EVENT_OUTUP || event == GUI_EVENT_UP)
					BLOCK_OFF;

				if (event)
				{

					//重绘(交给多线程)
					/*if (controls[i]->type == GUI_CONTROL_BUTTON)
					{
					controls[i]->Render(pdev);
					}*/

					//回调
					if (pEventProc)
						pEventProc(controls[i]->GetID(), event, 0);
				}
			}
		}
	}
}

void CGDIGUISystem::HandleKeyboard(UINT8 keytype, WPARAM wParam)
{
	for (int i = 0; i < nControls; i++)
	{
		if (controls[i]->GetType() == GUI_CONTROL_EDIT && ISFOCUS(i))
		{
			if (!controls[i]->bDisabled && controls[i]->bVisible)
			{
				bool cmd = controls[i]->HandleKeyboard(wParam, keytype == GUI_KEYEVENT_IMECHAR);

				if (cmd && pEventProc)
				{
					pEventProc(controls[i]->GetID(), GUI_EVENT_CMD, (LPARAM)controls[i]->text);
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

			controls[i]->displayEvent = GUI_EVENT_REFRESH;
			break;
		}
	}
}

void CGDIGUISystem::Invalidate()
{
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
	for (int i = 0; i < nControls; i++)
	{
		controls[i]->Render(pdev);
	}
	QueryPerformanceCounter(&etime);
	time = 1000.0f*(etime.QuadPart - stime.QuadPart) / freq.QuadPart;
}

#endif // USE_GDIGUI