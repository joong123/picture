#pragma once

#include "d3dwnd.h"

#include <d3d9.h>
#include <d3dx9tex.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

// 按键状态
#ifndef KEYSTATEMASK_DOWN
#define KEYSTATEMASK_DOWN				0x8000
#endif
#ifndef ISKEYDOWN
#define ISKEYDOWN(KEY)					((GetAsyncKeyState(KEY) & KEYSTATEMASK_DOWN) != 0)
#endif
#define ISSHIFTDOWN						ISKEYDOWN(VK_SHIFT)
#define CAPSLOCK_ON						((bool)(GetKeyState(VK_CAPITAL) & 0x01))
#define CAPS_STATUS_ON					(CAPSLOCK_ON ^ ISSHIFTDOWN)

#define ISCHAR(C)						(((C) <= 'Z' && (C) >= 'A') || (C) <= ('z' && (C) >= 'a'))
#define ISUPPERCHAR(C)					((C) <= 'Z' && (C) >= 'A')
#define ISLOWERCHAR(C)					((C) <= 'z' && (C) >= 'a')
#define TOUPPERCHAR_UNSAFE(C)			(C) += 'A' - 'a';
#define TOLOWERCHAR_UNSAFE(C)			(C) += 'a' - 'A';

// 操作
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)					{ if (p) { (p)->Release(); (p) = NULL; } }
#endif
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)					{ if (p) { delete (p); (p) = NULL; } }
#endif
#ifndef SAFE_DELETE_LIST
#define SAFE_DELETE_LIST(p)				{ if (p) { delete[] (p); (p) = NULL; } }
#endif
#ifndef SAFE_RELEASEDELETE
#define SAFE_RELEASEDELETE(p)			{ if (p) { (p)->Release(); delete (p); (p) = NULL; } }
#endif
#ifndef NULL_RETURN_FALSE
#define NULL_RETURN_FALSE(p)			{ if (!p) return false; }
#endif
#ifndef FALSE_RETURN_FALSE
#define FALSE_RETURN_FALSE(p)			NULL_RETURN_FALSE(p)
#endif
#ifndef FAILED_RETURN_FALSE
#define FAILED_RETURN_FALSE(p)			{ if (FAILED(p)) return false; }
#endif
#ifndef SAFE_POINTERVALUE_0
#define SAFE_POINTERVALUE_0(p)			( (p)? *p : 0 )
#endif

// 颜色
#define SETALPHA(C, A)					((C & 0xFFFFFF) | (A << 24))
#define COLOR_WHITE						0xFFFFFFFF

//
#define TEXTFORMAT_DEFAULT				( DT_CENTER | DT_VCENTER | DT_NOCLIP | DT_SINGLELINE )
#define TEXTFORMAT_LEFT					( DT_LEFT | DT_VCENTER | DT_NOCLIP )

// 控件类型 
#define GUI_CONTROL_NULL				0
#define GUI_CONTROL_STATIC				1     // 静态
#define GUI_CONTROL_BUTTON				2     // 按钮
#define GUI_CONTROL_EDIT				3     // 文本框
#define GUI_CONTROL_BACKDROP			4     // 背景图

// 控件状态
#define GUI_STATE_OUT					1
#define GUI_STATE_OVER					2
#define GUI_STATE_DOWN					3
//#define GUI_STATE_OUTDOWN				4

// 控件停靠
#define GUI_WINDOCK_NORMAL				0
#define GUI_WINDOCK_RIGHT				1
#define GUI_WINDOCK_BOTTOM				2
#define GUI_WINDOCK_BOTTOMRIGHT			3
#define GUI_WINDOCK_BOTTOMHSPAN			4	// 横跨窗口
#define GUI_WINDOCK_FULLSCREEN			5	// 铺满
#define GUI_WINDOCK_SCALE				6	// 相对窗口比例

// 事件处理（程序端）
// GUI系统输入控件的事件（GUI系统统一整合事件，传递给控件）
#define GUI_MOUSEEVENT_MOVE				0b000
#define GUI_MOUSEEVENT_UP				0b001
#define GUI_MOUSEEVENT_DOWN				0b010
#define GUI_MOUSEEVENT_DOWNMOVE			0b011

#define GUI_KEYEVENT_KEYDOWN			0b00
#define GUI_KEYEVENT_IMECHAR			0b01

// 控件事件（用户端）
#define GUI_EVENT_NULL					0

#define GUI_EVENT_DOWN					1	
#define GUI_EVENT_UP					2	
#define GUI_EVENT_ENTER					3
#define GUI_EVENT_LEAVE					4
#define GUI_EVENT_OUTUP					5

#define GUI_EVENT_CMD					11

// 参数
#define GUI_DEFAULT_STATICWIDTH			120
#define GUI_DEFAULT_STATICHEIGHT		30
#define GUI_DEFAULT_BUTTONWIDTH			80
#define GUI_DEFAULT_BUTTONHEIGHT		30
#define GUI_DEFAULT_EDITWIDTH			80
#define GUI_DEFAULT_EDITHEIGHT			30
#define GUI_DEFAULT_BACKDROPWIDTH		120
#define GUI_DEFAULT_BACKDROPHEIGHT		60

#define GUI_DEFAULTTEXTLEN				63


void ErrorShow(HRESULT hr, WCHAR *msg = NULL, HWND hwnd = NULL, WCHAR *title = L"ERROR");

void ChangeCtrlAlpha(LPDIRECT3DVERTEXBUFFER9 pbuf, byte alpha);
bool mywcscpy(WCHAR **dest, WCHAR *src);

class CGUIControl
{
public:
	byte type;							// 控件类型
	int ID;								// 控件ID
	//int groupID;						// 控件分组，0组默认显示

	DWORD color;						// 颜色

	byte dockmode;						// 停靠模式
	float dockX, dockY, dockW, dockH;	// 停靠坐标（相对）
	INT8 displaydx, displaydy;			// 显示偏移
	float width, height;				// 宽度、高度
	float posX, posY;					// 控件位置（左上角的坐标）

	int inputpos;						// 输入字符位置
	WCHAR *text;						// 文本内容
	DWORD textcolor;					// 文本颜色
	LPD3DXFONT font;					// 文本字体
	RECT textrect;						// 文本显示区域

	bool bDisabled;						// 控件使能
	bool bVisible;						// 控件可见性

	byte state;							// 状态

	// 显示效果effect（渐变效果）
	byte displayevent;					// 需要特效时的事件
	LARGE_INTEGER lasttick;				// 计时
	byte alpha;							// 特效alpha值
	INT8 overdx, overdy;				// 显示位移（鼠标事件）
	INT8 downdx, downdy;

	static LARGE_INTEGER frequency;
public:
	CGUIControl();
	virtual ~CGUIControl();
	virtual bool Release();

	virtual void KillFocus();// not stable
	virtual void SetEnabled(bool bEnabled);
	virtual void SetVisible(bool bVisible);

	virtual void Translation(int dx, int dy);// not stable
	virtual void Dock(UINT *pbufw, UINT *pbufh);// stable
	virtual bool HandleInput(WPARAM wParam, bool IME = false);// stable
	virtual byte HandleMouse(byte lmbstate, LONG mouseX, LONG mouseY);// stable

	virtual bool Render()// not stable
	{
		return false;
	};
};


#define D3DFVF_GUI						(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

struct D3DGUIVertex
{
	float x, y, z, rhw;
	DWORD color;
	float tu, tv;
};

#define D3DGUI_RENDER_VBUFFER(DEV, LPBUF)	{\
	DEV->SetStreamSource(0, LPBUF, 0, sizeof(D3DGUIVertex));\
	DEV->SetFVF(D3DFVF_GUI);\
	DEV->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2); }

class D3DGUIControl :public CGUIControl
{
public:
	LPDIRECT3DVERTEXBUFFER9 vbuffer;	// 模型

	LPDIRECT3DTEXTURE9 m_outTex;		// 纹理
	LPDIRECT3DTEXTURE9 m_overTex;
	LPDIRECT3DTEXTURE9 m_downTex;

public:
	D3DGUIControl();
	~D3DGUIControl();
	bool Release();

	void Translation(int dx, int dy);// 平移 + RefreshVertexBuffer()
	void KillFocus();

	bool RefreshVertexBuffer();
	bool Render(LPDIRECT3DDEVICE9 dev);
};


class GDIGUIControl :public CGUIControl
{
public:
public:
	GDIGUIControl();
	~GDIGUIControl();
	bool Release();
};


#define FOCUS_CONTROL(i)		{ pFocusControl = controls[i]; }
#define BLOCK_ON_CONTROL(i)		{ pFocusControl = controls[i];	block = true; }
#define FOCUS_OFF				pFocusControl = NULL
#define BLOCK_OFF				block = false
#define ISFOCUS(i)				controls[i] == pFocusControl

typedef void (CALLBACK *LPGUIEVENTCALLBACK)(int ID, WPARAM wp, LPARAM lp);

class CD3DGUISystem
{
private:
	LPDIRECT3DDEVICE9 device;						// D3D设备
	UINT *pbufferw, *pbufferh;						// link， 实时backbuffer尺寸

	LPD3DXFONT defaultfont;							// 默认字体
	LPD3DXFONT *fonts;								// Direct3D字体对象链表
	D3DGUIControl **controls;						// GUI控件列表

	D3DGUIControl *backdrop;						// 背景图控件
	bool bUseBackdrop;

	int nFonts;										// 字体总数
	int nControls;									// 控件总数

	bool lastLMBdown;								// 存储鼠标按下情况
	D3DGUIControl *pFocusControl;					// 焦点
	bool block;										// 是否阻塞 

	LPGUIEVENTCALLBACK pEventProc;					// 事件回调
protected:
	bool ControlListExpand();//扩展控件列表空间

public:
	CD3DGUISystem();
	CD3DGUISystem(LPDIRECT3DDEVICE9 device);
	~CD3DGUISystem();

	bool Bind(D3DWnd *pd3dwnd);
	void Shutdown();
	void OnLostDevice();
	void OnResetDevice();

	// 创建字体
	bool CreateDXFont(WCHAR *fontName, int *fontID
		, INT Height = 0, INT Width = 0, INT Weight = 0
		, DWORD Quality = DEFAULT_QUALITY // 交换了次序
		, UINT MipLevels = 1, bool Italics = false
		, DWORD Charset = DEFAULT_CHARSET, DWORD OutputPrecision = OUT_DEFAULT_PRECIS
		, DWORD PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE
	);

	// 创建用于保存背景图几何形状的顶点缓存,并加载要映射到表面上的纹理图像
	// 添加静态文本控件
	// 添加按钮控件
	bool AddBackdrop(WCHAR *TexFileName, float x = 0, float y = 0
		, float width = GUI_DEFAULT_BACKDROPWIDTH, float height = GUI_DEFAULT_BACKDROPHEIGHT
		, byte dock = GUI_WINDOCK_FULLSCREEN);
	bool AddStatic(int ID, float x, float y
		, float width = GUI_DEFAULT_STATICWIDTH, float height = GUI_DEFAULT_STATICHEIGHT
		, WCHAR *text = L"", DWORD color = COLOR_WHITE, int fontID = 0
		, byte dock = GUI_WINDOCK_NORMAL);
	bool AddButton(int ID, float x, float y
		, float width = GUI_DEFAULT_BUTTONWIDTH, float height = GUI_DEFAULT_BUTTONHEIGHT
		, WCHAR *text = L"", int fontID = 0, byte dock = GUI_WINDOCK_NORMAL
		, WCHAR *up = L"1nd.png", WCHAR *over = L"2nd.png", WCHAR *down = L"3nd.png");
	bool AddEdit(int ID, float x, float y
		, float width = GUI_DEFAULT_EDITWIDTH, float height = GUI_DEFAULT_EDITHEIGHT
		, DWORD color = COLOR_WHITE, DWORD txtcolor = COLOR_WHITE, int fontID = 0, byte dock = GUI_WINDOCK_NORMAL);

	bool DropControl(int ID);							// 删除一个控件
	int GetState(int ID);								// 获取控件状态
	bool SetControlText(int ID, WCHAR *text);			// 设置控件文本
#define ShowControl(ID)			SetControlVisible(ID, true)
#define HideControl(ID)			SetControlVisible(ID, false)
	bool SetControlVisible(int ID, bool bVisible);		// 设置控件可见性
#define EnableControl(ID)		SetControlEnabled(ID, true)
#define DisableControl(ID)		SetControlEnabled(ID, false)
	bool SetControlEnabled(int ID, bool bEnable);		// 设置控件使能
	LPD3DXFONT GetFont(int ID);							// 从字体列表获取字体
	bool SetFocus(int ID);								// 设置焦点

	// 设置回调函数
	// 事件处理
	// 绘制
	void SetEventProc(LPGUIEVENTCALLBACK pevent);
	void HandleMouse(bool LMBDown, LONG mouseX, LONG mouseY);
	void HandleKeyboard(UINT8 keytype, WPARAM wParam);

	void RenderBack();									// 背景绘制
	void Render();										// 控件绘制
};