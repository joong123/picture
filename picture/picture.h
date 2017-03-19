#pragma once

// windows include
#include "time.h"
#include "shellapi.h"
#include <Psapi.h>
#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#include <Commdlg.h>
#include <io.h>

// local include
#include "resource.h"
#include "generalgeo.h"
#include "BMP.h"
#include "Pic.h"

// local dx include
//(using fast font render-CD3DFont)
#include "d3dfont.h"
#include "d3dwnd.h"
#include "gui.h"

// dx include
#include <d3d9.h>
#include <d3dx9tex.h>
//#include <DXUT.h>
//#include <DXUTgui.h>

// lib
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
//#pragma comment(lib,"DXUT.lib")
//#pragma comment(lib,"DXUTOpt.lib")
//use to handle compile error 
///NODEFAULTLIB:"libcmt.lib" 

using std::cout;
using std::endl;

// 用户消息
#define WM_RESETDEVICE					(WM_USER + 1)
#define WM_SURFFORCERENEW				(WM_USER + 2)
#define WM_TOGGLEFULLSCREEN				(WM_USER + 3)
#define WM_MOUSEWHEELEND				(WM_USER + 4)

// 命令行
#define CMDRECT_ALPHA					180
#define CMDRECT_COLOR					D3DCOLOR_ARGB(CMDRECT_ALPHA, 25, 220, 255)
#define CMDRECT_COLOR2					D3DCOLOR_ARGB(CMDRECT_ALPHA, 23, 24, 36) 
#define CMDRECT_COLOR3					0xB0CFF5DB 
#define CMDRECT_COLOR_USING				CMDRECT_COLOR

// 窗口尺寸
#define WINDOWPOSX_INIT					180
#define WINDOWPOSY_INIT					100
#define WINDOWWIDTH_INIT				800
#define WINDOWHEIGHT_INIT				520

// 窗口控制
#define SIZE_NEAR_PIXEL					5	//窗口调整大小，鼠标接近边缘的程度

// 单位转换
#define B_TO_MB							1048576.0f

// 帧率
#define ICONIC_SLEEP					50
#define NORMAL_FPS						60
#define NORMAL_FRAMETIME				(1000.0f/NORMAL_FPS)
#define STATIC_FPS_NORMAL				NORMAL_FPS		//静止状态窗口帧率
#define STATIC_FPS_SILENT				16
#define STATIC_FPS_SLEEP				0

// 标志
#define FLAGDELAY_ZOOM					500		//设稍微大点为了zoom效率
#define FLAGDELAY_SIZE					1		//wm_size时不进入主循环，不会更新size标志。这个值表示size结束后标志更新响应下限
#define FLAGDELAY_SURFREFRESH			160		//设稍微大点为了能够看清这个标志出现
#define FLAGDELAY_DRAG					100		//设稍微大点为了drag效率
#define FLAGDELAY_DRAGZOOM				100		//设稍微大点为了dragzoom效率

// 运行模式
#define MODE_PIC						0
#define MODE_CMD						1
//#define MODE_INTRO						2
//#define MODE_PICVIEWER					3
//#define MODE_VIDEO						4

// 窗口模式
#define WINMODE_NORMALWINDOWS			0
#define WINMODE_ROUND					1
#define WINMODE_INIT					WINMODE_ROUND

// 文字显示
// 距离窗口边缘距离
#define TEXTMARGIN_SIDE					3
#define TEXTMARGIN_TOP					0
#define TEXTMARGIN_BOTTOM				2
#define TEXTMARGIN_ROW					2

// 颜色
#define COLOR_BKG0						D3DCOLOR_ARGB(200, 238, 243, 249)
#define COLOR_BKG1						D3DCOLOR_ARGB(200, 255, 255, 255)
#define COLOR_BKGNIGHT					D3DCOLOR_ARGB(200, 18, 12, 20)
#define COLOR_BKG_INIT					COLOR_BKG0
#define COLOR_CMD						D3DCOLOR_ARGB(255, 20, 100, 255) 
#define COLOR_CMD2						D3DCOLOR_ARGB(240, 240, 200, 23)
#define COLOR_CMD3						D3DCOLOR_ARGB(250, 230, 40, 60)
#define COLOR_CMD_INIT					COLOR_CMD3
#define COLOR_TEXT0						D3DCOLOR_ARGB(250, 255, 255, 255)
#define COLOR_TEXT1						D3DCOLOR_ARGB(220, 110, 0, 255)
#define COLOR_TEXT2						D3DCOLOR_ARGB(220, 250, 200, 10)
#define COLOR_TEXT3						D3DCOLOR_ARGB(230, 120, 122, 122)
#define COLOR_TEXT4						D3DCOLOR_ARGB(230, 200, 200, 160)

#define COLOR_BLOCKALPHA				255
#define COLOR_BLOCKBACK					COLOR_BKG0

// D3D
#define COLORBLOCK_X					160
#define COLORBLOCK_Y					-1
#define COLORBLOCK_RADIUS				16

// GUI 系统
#define BUTTON_ID_1						1
#define BUTTON_ID_2						2
#define BUTTON_ID_3						3
#define BUTTON_ID_4						4
#define BUTTON_ID_5						5
#define BUTTON_ID_6						6
#define STATIC_ID_1						11
#define STATIC_ID_2						12
#define INPUT_IN_1						21


/*
 * 窗口信息&标志
 */
// 窗口信息
HWND mainwnd;							// 主窗口句柄
RECT wndrect, clientrect;				// 主窗口区域，客户区域
POINT clientsize;						// 客户区大小
POINT cursor, lastcursor;				// 当前鼠标坐标（全局）。lastcursor用于拖动时记录上一位置，与lastcursor不可混用
int wbias, hbias;						// 窗口与客户区宽度差
int wlimit, hlimit;						// 窗口长宽限位
RECT originwndrect;						// 全屏恢复
// 其他信息
HANDLE hprocess;						// 进程句柄
float memoryin, memoryout;				// 内存占用量
DWORD screencolor;						// 鼠标当前位置对应屏幕的颜色
// 选项标志
UINT8 mode;								// 模式
UINT8 winmode;							// 窗口模式
bool colorblockon;						// 显示当前颜色块
D3DCOLOR backcolor;						// 背景色
bool sizeEnable;						// 是否允许改变窗口大小
bool easymoveEnable;					// 是否允许右键拖动窗口
bool flagshow;							// 是否显示状态标志信息		
bool infoshow;							// 是否显示文字信息
bool debuginfoshow;
bool fpslimit;							// 是否限制帧率
bool screencoloron;						// 是否获取屏幕颜色
// 状态标志
bool LMBdown;							// 鼠标左键是否按下
bool purewnd;							// 是否是纯窗口状态（无标题栏）
bool iswindowedfullscreen;				// 是否在伪全屏状态
bool isiconic;							// 是否在最小化
bool ondrag;							// 是否开始了拖动图像状态
bool dragging;							// 是否在拖动图像状态（确定帧率）
bool ondragzoom;						// 是否开始了拖动放大图像状态
bool draggingzoom;						// 是否在拖动放大图像状态（确定帧率）
bool onzoom;							// 是否在放大（确定帧率）
bool onsize;							// 是否在缩放窗口
bool surfrenew;							// surface是否在刷新，【这个标志暂时只用来查看】
bool needforcerenew;					// 需要强制更新surface标志（用来判断是否发送WM_SURFFORCERENEW）

/*
 * 计时
 */
time_t loopcount, lastloopcount;		// 主循环计数
float fps, avgfps, cvgfps;				// 帧率
ULONGLONG fpscount;						// 计算帧率次数（用于计算收敛帧率cvgfps）
float frametime;						// 帧时间
short staticfps; float staticframetime;	// 帧率限制
float normalfps; float normalframetime;
time_t zoomtick, sizetick\
	, surfrenewtick, dragtick\
	, dragzoomtick, decoratetick;		// 标志计时

/*
 * D3D
 */
D3DWnd *pd3dwnd;						// D3D封装类
LPDIRECT3DDEVICE9 maindevice;			// 主设备 linked with "D3DWnd"
UINT *pbufferw, *pbufferh;				// 设备缓冲区尺寸 linked with "D3DWnd's backbuffer"
//LPD3DXSPRITE  m_sprite;				// 不知道怎么用!

LPD3DXMESH backdecorate;
D3DLIGHT9 light;
D3DMATERIAL9 material;
D3DXMATRIX basemat;


Surfer surfer;							// 图片显示器 linked with "D3D Device" and BMP
/*
 * 图片组
 */
//_finddata_t filegroup;
bool haspic;
PicPackList piclist;					// 图片列表
PicPack *mainpicpack;					// 主图片组
BMP *mainbmp;							// 当前图片 linked with "PicPack"。加速运算

/*
 * 信息显示
 */

// 颜色块
LPDIRECT3DVERTEXBUFFER9 colorblock;
LPDIRECT3DVERTEXBUFFER9 colorblockback;

// 其他信息		@封装入类WndDev
LPD3DXFONT font;						// 显示字体
LPD3DXFONT font2;
LPD3DXFONT fontcmd;
LPD3DXFONT fontpic;
//LPCD3DFont d3dfont1;					// 高速但效果一般的字体绘制类
RECT textrect0;							// 图片信息区域
RECT textrect1;							// surface信息区域
RECT textrect2;							// 状态信息区域
RECT cmdrect;							// 命令行显示区域
RECT picrect;							// 图片状态显示区域
WCHAR picinfostr[256];					// 显示信息缓存，（存储基本信息-更新少，访问多）

/*
 * 窗口
 */
void ClickDragWindow_Custom(int cursorx, int cursory);// 自定义拉伸窗口
void KeyDownProc(WPARAM wParam);		// WM_KEYDOWN消息处理函数
void SetWindowMode(UINT8 wmode);		// 设置窗口模式
inline void MaintainWindowStyle();		// 维持窗口模式（对于异形窗口）
inline void OnWinChange();				// 获得窗口和客户区大小
inline void RefreshTextRect();			// 更新文字显示区域
void FitWnd(int bmpwidth, int bmpheight);// 设置标准窗口尺寸
bool FullScreen_Windowed(bool tofull);
bool PureWindow(bool topure);
void SetStaticFps(float sfps);
// 鼠标
inline short GetSizeType(CPoint point);	// 根据当前坐标，获得鼠标类型（改变窗口大小）
inline void SetCursor_Custom();			// 如果允许调整大小，设置鼠标样式
// 初始化
HFONT hf;
bool Init();							// 初始化
// D3D初始化
bool D3DInit();							// D3D初始化
// 文件
void OnSave();
void OnSaveAs();
bool OnWininitFile(LPWSTR cmdline);		// 初始化时载入图片
void OpenFileWin();						// 打开文件窗口
void SaveFileWin(WCHAR file[] = L"");	// 保存文件窗口

bool OnSaveFile(WCHAR file[]);			// 保存图片
bool OnLoadFile(WCHAR file[]);			// 加载图片

// 标志
inline void DelayFlag();				// 延迟标志的消除
void ClearFlag();						// 清除活跃标志
// 状态切换
inline void BeginDragPic();				// 开始图片拖动
inline void EndDragPic();				// 结束图片拖动
inline void BeginDragZoomPic();			// 开始图片拖动放大
inline void EndDragZoomPic();			// 结束图片拖动放大
// 模式
inline void EnterCMDMode();				// 进入命令行模式
inline void ExitCMDMode();				// 退出命令行模式（回到正常模式）

/*
 * 图像
 */
inline bool HasPic()					// 计算当前是否图片非空
{
	if (mainbmp)
		if (mainbmp->isNotEmpty())
			return true;
	return false;
}
void InitNonPic();						// 初始化无图片状态的参数
bool SetPic(short picidx = 1);			// 根据指定编号设置图片
bool SetTailPic();						// 定位到最后一张图片
bool SetPrevPic();						// 定位到下一图片
bool SetNextPic();						// 定位到下一图片
void Drop();							// 清除当前图片
void SetSurface(PicPack *oldpinpack);	// 设置图片之后设置surface
void SetPicInfo();						// 设置当前图片信息，根据mainpicpack

/*
 * D3D
 */
inline void SetRenderState();			// 设置渲染状态
inline void SetView();
inline void SetLight();
inline bool InfoRender();				// 渲染文字信息
inline void Render();					// 渲染
// 设备处理
inline bool ResetDevice();				// 重置设备
inline void OnLostDevice();				// 设备丢失动作
inline bool OnResetDevice2(int clientw, int clienth);	// 设备重置动作
inline bool OnResetDevice();			// 设备重置动作
inline void PostResetDevice();			// 设备重置后动作

// GUI 系统
CD3DGUISystem *g_gui;
int fontID1;

// 回调函数
void CALLBACK CMDProc(WCHAR *wstr);		// 命令行回调
void CALLBACK GUICallback(int ID, WPARAM wp, LPARAM lp = NULL);// GUI回调
//MMRESULT TimerID_main;
//void CALLBACK TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);//计时器处理