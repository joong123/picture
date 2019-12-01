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
#include "picfile.h"
#include "wndaid.h"
#include "picpacklist.h"

// local dx include
#include "d3dfont.h" // ( use fast fFont render - CD3DFont )
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
// use to handle compile error 
///NODEFAULTLIB:"libcmt.lib" 

using std::cout;
using std::endl;

// 用户消息
#define WM_RESETDEVICE					(WM_USER + 1)	// 重置 D3D 设备消息
#define WM_SURFFORCERENEW				(WM_USER + 2)	// surface 更新消息
#define WM_TOGGLEFULLSCREEN				(WM_USER + 3)	// 切换全屏消息
#define WM_MOUSEWHEELEND				(WM_USER + 4)	// FLAGDELAY_WHEEL 设定延迟时间，滚轮结束后经过这段时间后，发送一个 "滚轮结束" 事件
#define WM_SIZEEND						(WM_USER + 5)	// FLAGDELAY_SIZE

// 命令行
#define CMDRECT_ALPHA					180
#define CMDRECT_COLOR					D3DCOLOR_ARGB(CMDRECT_ALPHA, 25, 220, 255)
#define CMDRECT_COLOR2					D3DCOLOR_ARGB(CMDRECT_ALPHA, 23, 24, 36) 
#define CMDRECT_COLOR3					0xB0CFF5DB 
#define CMDRECT_COLOR_USING				CMDRECT_COLOR

// 窗口尺寸
#define WINDOWPOSX_INIT					CW_USEDEFAULT // 默认值
#define WINDOWPOSY_INIT					100
#define WINDOWWIDTH_INIT				656
#define WINDOWHEIGHT_INIT				518

// 窗口控制
#define SIZE_NEAR_PIXEL					8	// 鼠标与边框贴合距离（窗口拉伸）

// 单位转换
#define B_TO_MB							1048576.0f

// 时间
#define ICONIC_SLEEP					50
#define NORMAL_FPS						60
#define NORMAL_FRAMETIME				(1000.0f / NORMAL_FPS)
#define STATIC_FPS_NORMAL				NORMAL_FPS // 静止状态窗口帧率
#define STATIC_FPS_SILENT				16
#define STATIC_FPS_SLEEP				0

// 标志
#define FLAGDELAY_WHEEL					400		// 设稍微大点为了 zoom 效率
#define FLAGDELAY_SIZE					1		// WM_SIZE 时不进入主循环，不会更新 size 标志。这个值表示 size 结束后标志更新响应下限
#define FLAGDELAY_SURFREFRESH			160		// 设稍微大点为了能够看清这个标志出现
#define FLAGDELAY_DRAG					100		// 设稍微大点为了 drag 效率
#define FLAGDELAY_DRAGZOOM				100		// 设稍微大点为了 dragzoom 效率

// 运行模式
#define MODE_PIC						0
#define MODE_CMD						1		// TODO：不设为模式，设为状态
//#define MODE_PICVIEWER					2	// 图片浏览模式
//#define MODE_VIDEO						3	// 视频播放模式

// 窗口模式
#define WINMODE_NORMAL					0
#define WINMODE_ROUND					1
#define WINMODE_INIT					WINMODE_NORMAL

// 文本显示
// 距离窗口边缘距离
#define TEXTMARGIN_SIDE					3
#define TEXTMARGIN_TOP					0
#define TEXTMARGIN_BOTTOM				2
#define TEXTMARGIN_ROW					2
// 颜色
#define COLOR_BKG0						D3DCOLOR_ARGB(200, 238, 243, 249)
#define COLOR_BKG1						D3DCOLOR_ARGB(200, 255, 255, 255)
#define COLOR_BKGNIGHT					D3DCOLOR_ARGB(200, 18, 12, 20)
#define COLOR_BKG_INIT					COLOR_BKG0//COLOR_BKG0
#define COLOR_CMD						D3DCOLOR_ARGB(255, 20, 100, 255) 
#define COLOR_CMD2						D3DCOLOR_ARGB(240, 240, 200, 23)
#define COLOR_CMD3						D3DCOLOR_ARGB(250, 230, 40, 60)
#define COLOR_CMD_INIT					COLOR_CMD3
#define COLOR_TEXT0						D3DCOLOR_ARGB(250, 255, 255, 255)
#define COLOR_TEXT1						D3DCOLOR_ARGB(220, 110, 0, 255)
#define COLOR_TEXT2						D3DCOLOR_ARGB(220, 250, 200, 10)
#define COLOR_TEXT3						D3DCOLOR_ARGB(230, 120, 122, 122)
#define COLOR_TEXT4						D3DCOLOR_ARGB(230, 200, 200, 160)
#define COLOR_TEXT5						D3DCOLOR_ARGB(230, 255, 201, 14)

#define COLOR_BLOCKALPHA				255
#define COLOR_BLOCKBACK					COLOR_BKG0

// D3D
// GUI 系统
#define BUTTON_ID_OPEN					1
#define BUTTON_ID_SAVE					2
#define BUTTON_ID_AID					3
#define BUTTON_ID_FULLSCREEN			11
#define INPUT_IN_CMD					21


/*
* 启动窗口信息、标志
*/
extern D3DWnd *startup;
extern HWND hWndStartup;
extern bool bDrag;
extern POINT ptCurStartup;
extern LRESULT CALLBACK    StartWndProc(HWND, UINT, WPARAM, LPARAM);

extern HWND hWndAid;
extern PROCESS_INFORMATION pi;
extern STARTUPINFO si;// 用于指定新进程的主窗口特性的一个结构


/*
 * 窗口信息&标志
 */
// 窗口状态
HWND hWndMain;							// 主窗口句柄
RECT rcWindow, rcClient;				// 主窗口区域，客户区域
RECT rcOriginalWnd;						// 记录全屏之前窗口区域，用于从全屏状态恢复
HCURSOR hCursorMain;					// 自定义光标
HCURSOR hCursorG;
POINT ptLastCursor;						// 上一鼠标位置。

// 其他信息
HANDLE hProcess;						// 进程句柄
float memoryIn, memoryOut;				// 内存占用量

// 选项标志
UINT8 mode;								// 运行模式
UINT8 winMode;							// 窗口模式
D3DCOLOR BackgroundColor;				// 背景色
bool bFlagsShow;						// 是否显示状态标志信息		
bool bInfoShow;							// 是否显示文字信息
bool bFpsLimited;						// 是否限制帧率

// 状态标志
bool bPureWnd;							// 是否是纯窗口状态 ( 无标题栏 )
bool bWindowedFullscreen;				// 是否在伪全屏状态

bool bSurfRenew;						// surface 是否在刷新，【这个标志暂时只用来查看】
bool bNeedForceRenew;					// 需要强制更新 surface 标志 ( 用来判断是否发送 WM_SURFFORCERENEW )
// TODO: 加入统一鼠标键盘事件处理
bool bOnZoom;							// 是否在放大
bool bOnSize;							// 是否在缩放窗口
bool bOnDrag;							// 是否开始了拖动图片状态
bool bDragging;							// 是否在拖动图片状态
bool bOnDragzoom;						// 是否开始了拖动放大图片状态
bool bDragzooming;						// 是否在拖动放大图片状态
bool bOnDragRotate;

// 设置标志 :
#define SF_SR(RETVAL)	{ bSurfRenew = RETVAL;if(bSurfRenew) surfRenewTick = GetTickCount(); }
#define SF_SRFR(RETVAL) { bSurfRenew = RETVAL;if(bSurfRenew) surfRenewTick = GetTickCount(); bNeedForceRenew |= bSurfRenew;}
#define SF_DR()			{ bDragging = true;dragTick = GetTickCount(); }
#define SF_OZ()			{ bOnZoom = true;zoomTick = GetTickCount(); }

/*
 * 计时
 */
LONGLONG nLoops;						// 主循环计数
float fps, avgFps, cvgFps;				// 帧率
ULONGLONG fpsCount;						// 计算帧率次数 ( 用于计算收敛帧率 cvgFps )
float frameTime;						// 帧时间
float procTime;							// 处理时间
short staticFps; float staticFrameTime;	// 帧率限制
float normalFps; float normalFrameTime;
time_t zoomTick, sizeTick\
	, surfRenewTick, dragTick\
	, dragzoomTick, decorateTick;		// 标志计时

/*
 * D3D
 */
D3DWnd *pD3DWnd;						// D3D 窗口 封装类
LPDIRECT3DDEVICE9 mainDevice;			// 主设备，与 D3DWnd 绑定
UINT *pBufferW, *pBufferH;				// 设备缓冲区尺寸，与 D3DWnd 的 backbuffer 绑定
//LPD3DXSPRITE  m_sprite;				// 不知道怎么用!

bool bDecorateOn;
LPD3DXMESH decorate;
D3DLIGHT9 light;
D3DMATERIAL9 material;
D3DXMATRIX matWorld;

Surfer surfer;							// 图片显示器，与 D3D 设备、图片 BMP 绑定
/*
* 图片组
*/
extern bool bPicOn;
extern PicPackList picList;				// 图片列表
extern PicPack *pLastPicpack;
extern PicPack *pLivePicpack;			// 主图片组
extern BMP *pLiveBmp;
extern WCHAR picInfoStr[256];			// 图片信息缓存

/*
 * 信息显示
 */
// 其他信息		@封装入类WndDev
LPD3DXFONT pFontPic;					// 显示字体
LPD3DXFONT pFontFlags;
LPD3DXFONT pFontPicState;
//LPCD3DFont d3dfont1;					// 高速但效果一般的字体绘制类
RECT rcPic;								// 图片信息区域
RECT rcSurface;							// surface 信息区域
RECT rcFlag;							// 状态信息区域
RECT rcPicState;						// 图片状态显示区域

/*
 * 窗口
 */
void BeginDragWindow_Inner(POINT ptCur);// 自定义拉伸窗口
void BeginDragWindow_Custom(POINT ptCur);// 自定义拉伸窗口
void KeyDownProc(WPARAM wParam);		// WM_KEYDOWN 消息处理函数
void SetWindowMode(UINT8 wmode);		// 设置窗口模式
inline void MaintainWindowStyle();		// 维持窗口模式 ( 对于异形窗口 )
inline void OnWinChange();				// 获得窗口和客户区大小
inline void RefreshTextRect();			// 更新文字显示区域
bool FullScreen_Windowed(bool tofull, bool restore = true);
bool PureWindow(bool topure);
void SetStaticFps(float sfps);
void ToggleNight();

// 鼠标
inline short GetSizeType();	// 根据当前坐标，获得鼠标类型 ( 改变窗口大小 )
// 初始化
bool Init();							// 初始化
// D3D初始化
bool D3DInit();							// D3D 初始化
// 文件
void OnSave();
void OnSaveAs();
bool OnWinInitFile(LPWSTR cmdline);		// 初始化时载入图片
void OpenFileWin();						// 打开文件窗口
void SaveFileWin(const WCHAR file[] = L"");	// 保存文件窗口

bool SaveFile(WCHAR file[]);			// 保存图片
bool LoadFile(WCHAR file[]);			// 加载图片

// 标志
inline void DelayFlag();				// 延迟标志的消除
void ClearFlag();						// 清除活跃标志
// 状态切换
void BeginDragPic();					// 开始图片拖动
void EndDragPic();						// 结束图片拖动
void BeginDragZoomPic();				// 开始图片拖动放大
void EndDragZoomPic();					// 结束图片拖动放大
void BeginDragRotatePic();
void EndDragRotatePic();
// 模式
void EnterCMDMode();					// 进入命令行模式
void ExitCMDMode();						// 退出命令行模式 ( 回到正常模式 )

/*
 * 图片 ( 图片列表 PicPackList 一系列操作）
 */
extern void UpdateLocalPicStr();
extern inline bool HasPic();
extern void InitNonPic();
extern bool SetPic(short picidx);
extern bool SetNewPic();
extern bool SetTailPic();
extern bool SetPrevPic();
extern BMP *GetPrevPic();
extern bool SetNextPic();
extern void Drop();

// 处理 picpack 变动处理、 信息变动处理
void UpdateTitle();	// 更新窗口标题
// PostPicPackChange 中不用 PostPicPackInfoChange 是因为 picpack 模块 PostPicEvent() 已实现
void PostPicPackChange(bool bRenew = true);// picpack 切换后处理 ( 用来补充 picpack 模块 PostPicEvent() 功能)
void PostPicPackInfoChange();// picpack 信息变化处理 ( 用来填补 picpack 没切换但是信息需要更新的情况)

// 处理 surface 对 PicPack 的绑定
void SwitchSurface(PicPack *newpicpack, PicPack *oldpicpack, bool renew = true);// 设置图片之后设置 surface
void UpdateSurfaceInfo();// 更新 surface 信息（图片更改、鼠标等信息更改）

//图片（surface操作过渡）
void PicMove(int dx, int dy);
void PicRestore();
void PicClipWindow();
void PicDock();
void PicCenter();
void PicFit();

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
inline void OnLostDevice();				// 设备丢失
inline bool OnResetDevice();			// 设备重置
inline bool OnResetDevice2(int clientw, int clienth);	// 设备重置
inline void PostResetDevice();			// 设备重置后

// GUI 系统
CD3DGUISystem *g_gui;
int fontID1;

// 回调函数
void CALLBACK CMDProc(WCHAR *wstr);		// 命令行回调
void CALLBACK GUICallback(int ID, WPARAM wp, LPARAM lp = NULL);// GUI 回调
//MMRESULT TimerID_main;
//void CALLBACK TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);// 计时器处理