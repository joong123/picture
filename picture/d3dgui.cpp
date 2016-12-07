#include "stdafx.h"
#include "d3dgui.h"

CD3DGUISystem::CD3DGUISystem(LPDIRECT3DDEVICE9 device, int w, int h)
{
	m_fonts = NULL;
	m_controls = NULL;
	m_vertexBuffers = NULL;

	m_totalFonts = m_totalControls = m_totalBuffers = 0;
	m_windowWidth = m_windowHeight = 0;
	m_useBackDrop = false;

	m_device = device;
	m_windowWidth = w; m_windowHeight = h;

	memset(&m_backDrop, 0, sizeof(stGUIControl));
}

void CD3DGUISystem::OnLost()
{
	for (int i = 0; i < GetTotalControls(); i++)
	{
		// Get the current control.
		stGUIControl *pCnt = GetGUIControl(i);
		if (!pCnt) continue;

		LPD3DXFONT pFont;
		LPDIRECT3DVERTEXBUFFER9 pBuffer;
		// Take action depending on what type it is.
		switch (pCnt->m_type)
		{
		case UGP_GUI_STATICTEXT:
			pFont = GetFont(pCnt->m_listID);
			if (pFont)
			{
				pFont->OnLostDevice();
			}
			break;
		case UGP_GUI_BUTTON:
			//pBuffer = GetVertexBuffer(pCnt->m_listID);
			//SAFE_RELEASE(m_vertexBuffers[pCnt->m_listID]);
			/*SAFE_RELEASE(pCnt->m_upTex);
			SAFE_RELEASE(pCnt->m_overTex);
			SAFE_RELEASE(pCnt->m_downTex);*/
			
			break;
		}
	}
}

void CD3DGUISystem::OnReset(LPDIRECT3DDEVICE9 device)
{
	m_device = device;
	for (int i = 0; i < GetTotalControls(); i++)
	{
		// Get the current control.
		stGUIControl *pCnt = GetGUIControl(i);
		if (!pCnt) continue;

		LPD3DXFONT pFont;
		LPDIRECT3DVERTEXBUFFER9 pBuffer;
		// Take action depending on what type it is.
		switch (pCnt->m_type)
		{
		case UGP_GUI_STATICTEXT:
			pFont = GetFont(pCnt->m_listID);
			if (pFont)
			{
				pFont->OnResetDevice();
			}

			break;
		case UGP_GUI_BUTTON:
			break;
		}
	}
}

bool CD3DGUISystem::CreateFont(WCHAR *fontName, int *fontID
	, INT Height, INT Width, INT Weight
	, DWORD Quality
	, UINT MipLevels, bool Italics
	, DWORD Charset, DWORD OutputPrecision
	, DWORD PitchAndFamily)
{
	if (!m_device) return false;

	if (!m_fonts)
	{
		m_fonts = new LPD3DXFONT[1];
		if (!m_fonts) return false;
	}
	else
	{
		//copy and swap
		LPD3DXFONT *temp;
		temp = new LPD3DXFONT[m_totalFonts + 1];
		if (!temp) return false;
		memcpy(temp, m_fonts, sizeof(LPD3DXFONT) * m_totalFonts);
		delete[] m_fonts;
		m_fonts = temp;
	}

	if (FAILED(D3DXCreateFontW(m_device,
		Height, Width, Weight, MipLevels, Italics,
		Charset, OutputPrecision, Quality,
		PitchAndFamily, fontName,
		&m_fonts[m_totalFonts]))) return false;

	if (!m_fonts[m_totalFonts]) return false;

	if (fontID)

		*fontID = m_totalFonts;

	m_totalFonts++;

	return true;
}




//背景图是全屏的，并且该信息可以以像素为单位指定，

// 而不是过去在创建几何图形时用的单位。由于整个场景是二维的，因此这样做很有意义。

bool CD3DGUISystem::AddBackdrop(WCHAR *fileName)
{
	if (!fileName) return false;

	m_backDrop.m_type = UGP_GUI_BACKDROP;

	// Load the texture image from file.
	if (D3DXCreateTextureFromFile(m_device, fileName, &m_backDrop.m_backDrop) != D3D_OK)
		return false;

	// Get window width and height so we can create a fullscreen quad.
	float w = (float)m_windowWidth;
	float h = (float)m_windowHeight;

	stGUIVertex obj[] =
	{
		{ w, 0, 0.0f, 1, D3DCOLOR_XRGB(255,255,255), 1.0f, 0.0f },
		{ w, h, 0.0f, 1, D3DCOLOR_XRGB(255,255,255), 1.0f, 1.0f },
		{ 0, 0, 0.0f, 1, D3DCOLOR_XRGB(255,255,255), 0.0f, 0.0f },
		{ 0, h, 0.0f, 1, D3DCOLOR_XRGB(255,255,255), 0.0f, 1.0f },
	};
	
	// Create the vertex buffer.
	if (FAILED(m_device->CreateVertexBuffer(sizeof(obj), 0, D3DFVF_GUI,
		D3DPOOL_DEFAULT, &m_backDropBuffer, NULL))) return false;
	
	// Fill the vertex buffer.
	void *ptr;
	if (FAILED(m_backDropBuffer->Lock(0, sizeof(obj), (void**)&ptr, 0))) return false;
	memcpy(ptr, obj, sizeof(obj));
	m_backDropBuffer->Unlock();

	m_useBackDrop = true;

	return true;
}

//可以有一个或多个使用相同或不同字体对象的静态文本。
//对大部分静态文本而言，可能将使用相同的字体对象，但也可以将字体对象组合起来使用。
bool CD3DGUISystem::AddStaticText(int id, WCHAR *text, float x, float y, unsigned long color, int fontID)
{
	if (!text || fontID < 0 || fontID >= m_totalFonts) return false;

	// Create a blank control.
	if (!m_controls)
	{
		m_controls = new stGUIControl[1];
		if (!m_controls) return false;
		memset(&m_controls[0], 0, sizeof(stGUIControl));
	}
	else
	{
		stGUIControl *temp;
		temp = new stGUIControl[m_totalControls + 1];
		if (!temp) return false;
		memset(temp, 0, sizeof(stGUIControl) * (m_totalControls + 1));
		memcpy(temp, m_controls, sizeof(stGUIControl) * m_totalControls);
		delete[] m_controls;
		m_controls = temp;
	}

	// Fill it with all the info we need for static text.
	m_controls[m_totalControls].m_type = UGP_GUI_STATICTEXT;
	m_controls[m_totalControls].m_id = id;
	m_controls[m_totalControls].m_color = color;
	m_controls[m_totalControls].m_xPos = x;
	m_controls[m_totalControls].m_yPos = y;
	m_controls[m_totalControls].m_listID = fontID;
	
	// Copy text data.
	int len = wcslen(text);
	m_controls[m_totalControls].m_text = new WCHAR[len + 1];
	if (!m_controls[m_totalControls].m_text) return false;
	memcpy(m_controls[m_totalControls].m_text, text, len);
	m_controls[m_totalControls].m_text[len] = (WCHAR)'\0';
	
	// Increment total.
	m_totalControls++;
	return true;
}

bool CD3DGUISystem::AddButton(int id, float x, float y, WCHAR *up, WCHAR *over, WCHAR *down)
{
	if (!up || !over || !down) return false;

	if (!m_controls)
	{
		m_controls = new stGUIControl[1];
		if (!m_controls) return false;
		memset(&m_controls[0], 0, sizeof(stGUIControl));
	}
	else
	{
		stGUIControl *temp;
		temp = new stGUIControl[m_totalControls + 1];
		if (!temp) return false;
		memset(temp, 0, sizeof(stGUIControl) * (m_totalControls + 1));
		memcpy(temp, m_controls, sizeof(stGUIControl) * m_totalControls);
		delete[] m_controls;
		m_controls = temp;
	}

	// Set all the data needed to render/process a button.
	m_controls[m_totalControls].m_type = UGP_GUI_BUTTON;
	m_controls[m_totalControls].m_id = id;
	m_controls[m_totalControls].m_xPos = x;
	m_controls[m_totalControls].m_yPos = y;
	m_controls[m_totalControls].m_listID = m_totalBuffers;
	
	// Load the texture image from file.
	if (D3DXCreateTextureFromFile(m_device, up, &m_controls[m_totalControls].m_upTex) != D3D_OK)
	{
		return false;
	}

	if (D3DXCreateTextureFromFile(m_device, over, &m_controls[m_totalControls].m_overTex) != D3D_OK)
	{
		return false;
	}
	
	if (D3DXCreateTextureFromFile(m_device, down, &m_controls[m_totalControls].m_downTex) != D3D_OK)
	{
		return false;
	}
	
	unsigned long white = D3DCOLOR_XRGB(255, 255, 255);

	// Get width and height of the image so we use that size.
	D3DSURFACE_DESC desc;
	m_controls[m_totalControls].m_upTex->GetLevelDesc(0, &desc);
	
	float w = (float)desc.Width;
	float h = (float)desc.Height;

	m_controls[m_totalControls].m_width = w;
	m_controls[m_totalControls].m_height = h;
	
	stGUIVertex obj[] =
	{
		{ w + x, 0 + y, 0.0f, 1, white, 1.0f, 0.0f },
		{ w + x, h + y, 0.0f, 1, white, 1.0f, 1.0f },
		{ 0 + x, 0 + y, 0.0f, 1, white, 0.0f, 0.0f },
		{ 0 + x, h + y, 0.0f, 1, white, 0.0f, 1.0f },
	};
	
	// Create the vertex buffer.
	if (!m_vertexBuffers)
	{
		m_vertexBuffers = new LPDIRECT3DVERTEXBUFFER9[1];
		if (!m_vertexBuffers) return false;
	}
	else
	{
		LPDIRECT3DVERTEXBUFFER9 *temp;
		temp = new LPDIRECT3DVERTEXBUFFER9[m_totalBuffers + 1];
		if (!temp) return false;
		memcpy(temp, m_vertexBuffers, sizeof(LPDIRECT3DVERTEXBUFFER9) * m_totalBuffers);
		delete[] m_vertexBuffers;
		m_vertexBuffers = temp;
	}

	//这句导致设备丢失处理异常
	if (FAILED(m_device->CreateVertexBuffer(sizeof(obj), 0,
		D3DFVF_GUI, D3DPOOL_SYSTEMMEM, &m_vertexBuffers[m_totalBuffers], NULL)))
	{
		return false;
	}
	// Fill the vertex buffer.
	void *ptr;
	if (FAILED(m_vertexBuffers[m_totalBuffers]->Lock(0, sizeof(obj), (void**)&ptr, 0)))
	{
		return false;
	}
	memcpy(ptr, obj, sizeof(obj));
	m_vertexBuffers[m_totalBuffers]->Unlock();
	
	// Increase.
	m_totalBuffers++;
	
	// Increase.
	m_totalControls++;
	
	return true;
}

void CD3DGUISystem::Shutdown()
{
	if (m_useBackDrop)
	{
		if (m_backDrop.m_backDrop)
			m_backDrop.m_backDrop->Release();
		if (m_backDropBuffer)
			m_backDropBuffer->Release();
	}
	m_backDrop.m_backDrop = NULL;
	m_backDropBuffer = NULL;
	
	for (int i = 0; i < m_totalFonts; i++)
	{
		if (m_fonts[i])
		{
			m_fonts[i]->Release();
			m_fonts[i] = NULL;
		}
	}
	if (m_fonts) delete[] m_fonts;
	m_fonts = NULL;
	m_totalFonts = 0;
	
	for (int i = 0; i < m_totalBuffers; i++)
	{
		if (m_vertexBuffers[i])
		{
			m_vertexBuffers[i]->Release();
			m_vertexBuffers[i] = NULL;
		}
	}

	if (m_vertexBuffers) delete[] m_vertexBuffers;
	m_vertexBuffers = NULL;
	m_totalBuffers = 0;
	
	for (int i = 0; i < m_totalControls; i++)
	{
		if (m_controls[i].m_backDrop)
		{
			m_controls[i].m_backDrop->Release();
			m_controls[i].m_backDrop = NULL;
		}

		if (m_controls[i].m_upTex)
		{
			m_controls[i].m_upTex->Release();
			m_controls[i].m_upTex = NULL;
		}
		
		if (m_controls[i].m_downTex)
		{
			m_controls[i].m_downTex->Release();
			m_controls[i].m_downTex = NULL;
		}

		if (m_controls[i].m_overTex)
		{
			m_controls[i].m_overTex->Release();
			m_controls[i].m_overTex = NULL;
		}
		
		if (m_controls[i].m_text)
		{
			delete[] m_controls[i].m_text;
			m_controls[i].m_text = NULL;
		}
	}

	if (m_controls) delete[] m_controls;
	m_controls = NULL;
	m_totalControls = 0;
}

void ProcessGUI(CD3DGUISystem *gui, bool LMBDown, int mouseX, int mouseY, void(*funcPtr)(int id, int state))
{
	if (!gui) return;

	LPDIRECT3DDEVICE9 device = gui->GetDevice();
	if (!device) return;

	// 先渲染背景图。因为该控件位于个控件的最下方，所以要在绘制
	// 其他控件之前先绘制它，这样可以在它上面绘制其他控件。
	// 由于只与二维空间打交道，因此渲染GUI时，可以避免考虑深度测试这样的问题.
	stGUIControl *backDrop = gui->GetBackDrop();
	LPDIRECT3DVERTEXBUFFER9 bdBuffer = gui->GetBackDropBuffer();
	
	// Only can draw if we have created it.
	if (gui->UseBackDrop() && backDrop && bdBuffer)
	{
		device->SetTexture(0, backDrop->m_backDrop);
		device->SetStreamSource(0, bdBuffer, 0, sizeof(stGUIVertex));
		device->SetFVF(D3DFVF_GUI);
		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		device->SetTexture(0, NULL);
	}

	//循环控件列表，并按照每个控件类型对其加以渲染。在循环语句中有一
	//个switch语句，来检查正在处理的控件类型。如果是静态文本控件，就要获取
	//该文本使用的D3D字体对象，设置文本位置，并调用LPD3DXFONT的成员函数DrawText()显示该文本。
	// Objects used to display text.
	LPD3DXFONT pFont = NULL;
	RECT fontPosition = { 0, 0, (long)gui->GetWindowWidth(), (long)gui->GetWindowHeight() };

	// Object used to render buttons;
	LPDIRECT3DVERTEXBUFFER9 pBuffer = NULL;
	int status = UGP_BUTTON_UP;

	// Loop through all controls and display them.
	for (int i = 0; i < gui->GetTotalControls(); i++)
	{
		// Get the current control.
		stGUIControl *pCnt = gui->GetGUIControl(i);
		if (!pCnt) continue;

		bool callback = false;

		// Take action depending on what type it is.
		switch (pCnt->m_type)
		{
		case UGP_GUI_STATICTEXT:
			// Get font object this text uses.
			pFont = gui->GetFont(pCnt->m_listID);
			if (!pFont) continue;

			// Set text position.
			fontPosition.left = pCnt->m_xPos;
			fontPosition.right = fontPosition.left + 400;
			fontPosition.top = pCnt->m_yPos;

			// Display text.
			pFont->DrawTextW(NULL, pCnt->m_text, -1, &fontPosition, DT_LEFT|DT_NOCLIP, pCnt->m_color);
			break;
		case UGP_GUI_BUTTON:
			status = UGP_BUTTON_UP;

			// Get vertex buffer object this button uses.
			pBuffer = gui->GetVertexBuffer(pCnt->m_listID);
			if (!pBuffer) continue;

			// Set alpha transparency on for the texture image.
			//device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			//device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			//device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			// Check if we are over the button with the mouse or pressing it.
			if (mouseX > pCnt->m_xPos && mouseX < pCnt->m_xPos + pCnt->m_width &&
				mouseY > pCnt->m_yPos && mouseY < pCnt->m_yPos + pCnt->m_height)
			{
				if (LMBDown)
					status = UGP_BUTTON_DOWN;
				else
					status = UGP_BUTTON_OVER;
				callback = true;
			}

			// Depending on the mouse state will depend on it's texture.
			if (status == UGP_BUTTON_UP) device->SetTexture(0, pCnt->m_upTex);
			if (status == UGP_BUTTON_OVER) device->SetTexture(0, pCnt->m_overTex);
			if (status == UGP_BUTTON_DOWN) device->SetTexture(0, pCnt->m_downTex);

			// Render button.
			if (pBuffer)
			{
				device->SetStreamSource(0, pBuffer, 0, sizeof(stGUIVertex));
				device->SetFVF(D3DFVF_GUI);
				device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			}

			// Turn off alpha.
			//device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			break;
		}

		// Process control by calling the callback function for it.
		if (funcPtr && callback)
		{
			funcPtr(pCnt->m_id, status);
		}
	}
}

