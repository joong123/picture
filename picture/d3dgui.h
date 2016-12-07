#pragma once

// Types of controls we support.
#define UGP_GUI_STATICTEXT 1     //静态文本
#define UGP_GUI_BUTTON     2     // 按钮
#define UGP_GUI_BACKDROP   3     //背景图

// Mouse button states.
#define UGP_BUTTON_UP      1    
#define UGP_BUTTON_OVER    2
#define UGP_BUTTON_DOWN    3

#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }

#include <d3d9.h>
#include <d3dx9tex.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

// A structure for our custom vertex type
struct stGUIVertex
{
	float x, y, z, rhw;
	unsigned long color;
	float tu, tv;
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_GUI (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1) 

struct stGUIControl
{
	//控件类型
	int m_type;
	
	//控件id
	int m_id;

	//控件颜色
	unsigned long m_color;

	// 如果控件类型是静态文本，则是GUISystem中字体链表中的索引；否则是定点缓存链表中的索引。
	int m_listID;

	//控件左上角的坐标位置
	float m_xPos, m_yPos;

	// 按钮的宽度和高度（仅对按钮有效，静态文本，背景图时无效）
	float m_width, m_height;

	// 静态文本的文字内容（仅对静态文本有效）
	WCHAR *m_text;
	
	// 背景图用的纹理 （仅对背景图有效）
	LPDIRECT3DTEXTURE9 m_backDrop;

	// 按钮弹起，按下，鼠标移动到按钮上时分别显示的图片（仅对按钮有效）
	LPDIRECT3DTEXTURE9 m_upTex, m_downTex, m_overTex;
};

class CD3DGUISystem
{
public:
	CD3DGUISystem(LPDIRECT3DDEVICE9 device, int w, int h);
	~CD3DGUISystem() { Shutdown(); }

	void OnLost();
	void OnReset(LPDIRECT3DDEVICE9 device);
	// 创建字体
	// @fontName 新创建字体的名字
	// @size     新创建字体对象的大小
	// @fontID   新创建字体的ID指针
	bool CreateFont(WCHAR *fontName, int *fontID
		, INT Height = 0, INT Width = 0, INT Weight = 0
		, DWORD Quality = DEFAULT_QUALITY//交换了次序
		, UINT MipLevels = 1, bool Italics = false
		, DWORD Charset = DEFAULT_CHARSET, DWORD OutputPrecision = OUT_DEFAULT_PRECIS
		, DWORD PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE
		);

	// 创建用于保存背景图几何形状的顶点缓存,并加载要映射到表面上的纹理图像
	// @fileName 要加载的纹理图像文件名
	bool AddBackdrop(WCHAR *fileName);
	
	// 添加静态文本控件
	// @id 控件ID
	// @text 想要显示的文本内容
	// @x   文本的起始x坐标
	// @y   文本的起始y坐标
	// @color 文本颜色
	// @fontID  文本要是用的字体的id（在字体链表中的索引）
	bool AddStaticText(int id, WCHAR *text, float x, float y, unsigned long color, int fontID);
	
	// 添加按钮控件
	// @id 控件id
	// @x  左上角的x坐标
	// @y  左上角的y坐标
	// @up  按钮弹起时显示的纹理
	// @over 鼠标在按钮上时显示的纹理
	// @down 按钮按下时显示的纹理
	bool AddButton(int id, float x, float y, WCHAR *up, WCHAR *over, WCHAR *down);
	
	void Shutdown();
	
	LPD3DXFONT GetFont(int id)
	{
		if (id < 0 || id >= m_totalFonts) return NULL;
		return m_fonts[id];
	}
	
	stGUIControl *GetGUIControl(int id)
	{
		if (id < 0 || id >= m_totalControls) return NULL;
		return &m_controls[id];
	}
	
	LPDIRECT3DVERTEXBUFFER9 GetVertexBuffer(int id)
	{
		if (id < 0 || id >= m_totalBuffers) return NULL;
		return m_vertexBuffers[id];
	}
	
	int GetTotalFonts() { return m_totalFonts; }

	int GetTotalControls() { return m_totalControls; }

	int GetTotalBuffers() { return m_totalBuffers; }

	int GetWindowWidth() { return m_windowWidth; }

	int GetWindowHeight() { return m_windowHeight; }

	LPDIRECT3DDEVICE9 GetDevice() { return m_device; }

	stGUIControl *GetBackDrop() { return &m_backDrop; }

	LPDIRECT3DVERTEXBUFFER9 GetBackDropBuffer() { return m_backDropBuffer; }

	bool UseBackDrop() { return m_useBackDrop; }

	void SetWindowSize(int w, int h) { m_windowWidth = w; m_windowHeight = h; }
		
private:

	// D3D设备,用于创建Direct3D字体对象、顶点缓存和纹理。
	LPDIRECT3DDEVICE9 m_device;
	
	// Direct3D字体对象链表
	LPD3DXFONT *m_fonts;
	
	// GUI控件对象数组
	stGUIControl *m_controls;

	//顶点缓存链表是Direct3D顶点缓存对象链表,该链表存储系统中用到的全部按钮的顶点缓存.
	LPDIRECT3DVERTEXBUFFER9 *m_vertexBuffers;
	
	//背景图属性存储背景图信息。由于只有一幅背景图，因此将其从控件链表中分出来。
	stGUIControl m_backDrop;

	//背景图的顶点缓存用于存储全屏背景四方形的三角形数据。
	LPDIRECT3DVERTEXBUFFER9 m_backDropBuffer;
	
	// 是否使用背景图
	bool m_useBackDrop;
	
	// 字体总数
	int m_totalFonts;
	
	// 控件总数
	int m_totalControls;
	
	// 顶点缓存总数
	int m_totalBuffers;
	
	// 窗口宽度、高度
	int m_windowWidth;
	int m_windowHeight;

};

// @gui 需要处理的GUI
// @LMBDown 左键是都被按下
// @mouseX 以像素为单位的指针的x坐标
// @mouseY 以像素为单位的指针的y坐标
// @funcPtr 回调函数指针
// @id      回调函数需要的控件id
// @state   回调函数需要的控件状态
void ProcessGUI(CD3DGUISystem *gui, bool LMBDown, int mouseX, int mouseY, void(*funcPtr)(int id, int state));
