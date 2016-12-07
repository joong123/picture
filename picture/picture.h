#pragma once

//windows include
#include "time.h"
#include "shellapi.h"
#include <Psapi.h>
#include <iostream>
#include <windows.h>
#include <Commdlg.h>
#include <io.h>

//local include
#include "resource.h"
#include "generalgeo.h"
#include "BMP.h"
#include "Pic.h"
#include "inputer.h"
#include "d3dwnd.h"

//local dx include
//#include "d3dgui.h"
//(using fast font render-CD3DFont)
#include "d3dfont.h"

//dx include
#include <d3d9.h>
#include <d3dx9tex.h>
//#include <DXUT.h>
//#include <DXUTgui.h>

//lib
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
//#pragma comment(lib,"DXUT.lib")
//#pragma comment(lib,"DXUTOpt.lib")
//use to handle compile error 
///NODEFAULTLIB:"libcmt.lib" 

using std::cout;
using std::endl;

//用户消息
//#define WM_RESETDEVICE					(WM_USER + 1)
#define WM_SURFFORCERENEW				(WM_USER + 2)

//GUI 系统
#define BUTTON_ID_1						1

//命令行
#define CMDRECT_ALPHA					120
#define CMDRECT_COLOR					D3DCOLOR_ARGB(CMDRECT_ALPHA, 25, 220, 255)
#define CMDRECT_COLOR2					D3DCOLOR_ARGB(CMDRECT_ALPHA, 23, 24, 36) 
#define CMDRECT_COLOR3					0xB0CFF5DB 
#define CMDRECT_COLOR_USING				CMDRECT_COLOR2

//窗口信息
#define WINDOWPOSX_INIT					180
#define WINDOWPOSY_INIT					100
#define WINDOWWIDTH_INIT				800
#define WINDOWHEIGHT_INIT				520

//窗口控制
#define SIZE_NEAR_PIXEL					5	//窗口调整大小，鼠标接近边缘的程度

//按键状态掩码
#define KEYSTATEMASK_DOWN				0x8000

//单位转换
#define B_TO_MB							1048576.0f

//帧率
#define ICONIC_SLEEP					50
#define NORMAL_FPS						66
#define NORMAL_FRAMETIME				(1000.0f/NORMAL_FPS)
#define STATIC_FPS_NORMAL				16		//静止状态窗口帧率
#define STATIC_FRAMETIME_NORMAL			(1000.0f/STATIC_FPS_NORMAL)

//标志
#define FLAGDELAY_ZOOM					100		//设稍微大点为了zoom效率
#define FLAGDELAY_SIZE					1		//wm_size时不进入主循环，不会更新size标志。这个值表示size结束后标志更新响应下限
#define FLAGDELAY_SURFREFRESH			160		//设稍微大点为了能够看清这个标志出现
#define FLAGDELAY_DRAG					100		//设稍微大点为了drag效率
#define FLAGDELAY_DRAGZOOM				100		//设稍微大点为了dragzoom效率

//运行模式
#define MODE_PIC						0
#define MODE_CMD						1

//窗口模式
#define WINMODE_NORMALWINDOWS			0
#define WINMODE_ROUND					1
#define WINMODE_INIT					WINMODE_ROUND

//文字显示
//距离窗口边缘距离
#define TEXTMARGIN_SIDE					3
#define TEXTMARGIN_TOP					0
#define TEXTMARGIN_BOTTOM				2
#define TEXTMARGIN_ROW					2

//颜色
#define COLOR_BKG0						D3DCOLOR_ARGB(200, 238, 243, 249)
#define COLOR_BKG1						D3DCOLOR_ARGB(200, 255, 255, 255)
#define COLOR_BKGNIGHT					D3DCOLOR_ARGB(200, 18, 12, 20)
#define COLOR_BKG_INIT					COLOR_BKG0
#define COLOR_CMD						D3DCOLOR_ARGB(255, 20, 100, 255) 
#define COLOR_CMD2						D3DCOLOR_ARGB(240, 240, 200, 23)
#define COLOR_CMD_INIT					COLOR_CMD2
#define COLOR_TEXT0						D3DCOLOR_ARGB(160, 255, 255, 255)
#define COLOR_TEXT1						D3DCOLOR_ARGB(180, 100, 0, 255)

/*
 * 窗口信息&标志
 */
HWND mainwnd;							//主窗口句柄
RECT wndrect, clientrect;				//主窗口区域，客户区域
POINT clientsize;						//客户区大小
POINT cursor, lastpos;					//当前鼠标坐标（全局）。lastpos用于拖动时记录上一位置，与lastcursor不可混用
int wbias, hbias;						//窗口与客户区宽度差
int wlimit, hlimit;						//窗口长宽限位
RECT originwndrect;						//全屏恢复
//其他信息
HANDLE hprocess;						//进程句柄
float memoryin, memoryout;				//内存占用量
//选项标志
UINT8 mode;								//模式
UINT8 winmode;							//窗口模式
D3DCOLOR backcolor;						//背景色

bool sizeEnable;						//是否允许改变窗口大小
bool easymoveEnable;					//是否允许右键拖动窗口
bool flagshow;							//是否显示状态标志信息		
bool infoshow;							//是否显示文字信息
bool fpslimit;							//是否限制帧率
bool screencoloron;						//是否获取屏幕颜色
//状态标志
//dragging, onzoom, draggingzoom用来确定帧率控制策略，和确定重绘事件WM_SURFFORCERENEW响应
//WM_SIZE事件时不进入主循环，所以不需要判断onsize来确定帧率
//bool LMBDown;							//鼠标左键是否按下
bool purewnd;
bool iswindowedfullscreen;
bool ondrag;							//是否开始了拖动图像状态
bool dragging;							//是否在拖动图像状态（确定帧率）
bool ondragzoom;						//是否开始了拖动放大图像状态
bool draggingzoom;						//是否在拖动放大图像状态（确定帧率）
bool onzoom;							//是否在放大（确定帧率）
bool onsize;							//是否在缩放窗口
bool surfrefresh;						//surface是否在刷新，【这个标志暂时只用来查看】
bool needforcerenew;					//需要强制更新surface标志（用来判断是否发送WM_SURFFORCERENEW）

short mousestate;						//当前鼠标位置（用于设置鼠标类型，无边框窗口使用）
DWORD screencolor;						//鼠标当前位置对应屏幕的颜色

/*
 * 计时
 */
time_t loopcount;						//总循环计数(PEEKMESSAGE计数)
LARGE_INTEGER frequency, stime, etime;	//高精度计时
time_t nowtime, lasttime;				//秒级计时
float fps, avgfps, cvgfps;				//帧率
short staticfps; float staticframetime;
time_t zoomtick, sizetick\
	, surfrefreshtick, dragtick, dragzoomtick;	//标志计时
ULONGLONG fpscount;						//计算帧率次数（用于计算收敛帧率cvgfps）
float frametime;						//帧时间

/*
 * D3D
 */
D3DWnd *pd3dwnd;						//D3D封装类
LPDIRECT3DDEVICE9 maindevice;			//主设备 linked with "D3DWnd"
UINT *pbufferw, *pbufferh;				//设备缓冲区尺寸 linked with "D3DWnd's backbuffer"
//LPD3DXSPRITE  m_sprite;				//不知道怎么用!


/**************************************************************
一个PicPack存储一个图片文件的所有信息
主BMP：mainbmp作为当前活跃图片的一个镜像，原像在PicPack中。是当前操作的图片
有限个Surfer用于绑定不同BMP进行绘制
***************************************************************/
Surfer surfer;							//图片显示器 linked with "D3D Device" and BMP
/*
 * 图片组
 */
//PicPack *piclist;
PicPack mainpicpack;					//主图片组
BMP *mainbmp;							//当前图片 linked with "PicPack"。加速运算
WCHAR strFileName[MAX_PATH];			//文件名

/*
 * 图片组管理
 */
//_finddata_t filegroup;
//UINT32 picnum;
//UINT32 piccount;
//UINT32 piclistlen;
//PicPack *piclist;

/*
 * 信息显示
 */
//命令行			@封装入类WndDev
//D3DXMATRIXA16 matView;				//视角矩阵，用于设置视角（用临时变量代替）
//D3DXMATRIXA16 proj;					//投影矩阵，（用临时变量代替）
D3DXVECTOR3 eye;						//眼睛位置（观察位置）
D3DXVECTOR3 at;							//视线目标
D3DXVECTOR3 up;							//屏幕向上的世界向量
LPD3DXMESH cmdlight;					//命令行#1-三维画法
//LPDIRECT3DVERTEXBUFFER9 cmdlightrhw;	//命令行#2-二维画法（客户区坐标,缺点是重新定位时需要重建模型，除非固定命令行位置）
Inputer_W inputer0;						//命令行输入器

//其他信息		@封装入类WndDev
LPD3DXFONT font;						//显示字体
LPD3DXFONT font2;
LPD3DXFONT fontcmd;
//LPCD3DFont d3dfont1;					//高速但效果一般的字体绘制类
RECT textrect;							//显示字体区域
RECT textrect0;							//基本信息区域
RECT textrect2;							//状态信息区域
RECT cmdrect;							//命令行显示区域
WCHAR picinfostr[256];					//显示信息缓存，（存储基本信息-更新少，访问多）

/*
 * 窗口
 */
//inline void WinSizeProc();			//WM_SIZE消息处理函数
//inline void MouseMoveProc();			//WM_MOUSEMOVE消息处理函数
void ClickDragWindow_Custom(int cursorx, int cursory);
void KeyDownProc(WPARAM wparam);		//WM_KEYDOWN消息处理函数
inline void SetWindowMode(UINT8 wmode);	//设置圆角矩形窗口
inline void MaintainWindowStyle();
inline void OnWinChange();				//获得窗口和客户区大小
inline void RefreshTextRect();			//更新文字显示区域
void FitWnd(int bmpwith, int bmpheight);//设置标准窗口尺寸
bool FullScreen_Windowed(bool tofull);
bool PureWindow(bool topure);
//鼠标
inline short GetSizeType(CPoint);		//根据当前坐标，获得鼠标类型（改变窗口大小）
void CustomWinSizeProc();				//WM_LBUTTONDOWN消息的自定义拉伸窗口的处理函数
inline void SetCursor_Custom();				//如果允许调整大小，设置鼠标样式
//初始化
bool Init();							//初始化
//D3D初始化
bool D3DInit();							//D3D初始化
//文件
void OpenFileWin();						//打开文件窗口
void SaveFileWin();
bool OnSaveFile(WCHAR file[]);
bool OnLoadFile(WCHAR file[]);	//加载图片
bool OnWininitFile(LPWSTR file);//初始化时载入图片
//标志
inline void DelayFlag();		//延迟标志的消除
void ClearFlag();				//清除活跃标志
//操作
void BeginDragPic();			//开始图片拖动
void EndDragPic();				//结束图片拖动
void BeginDragZoomPic();		//开始图片拖动
void EndDragZoomPic();			//结束图片拖动
//窗口模式
inline void EnterCMDMode();		//进入命令行模式
inline void ExitCMDMode();		//退出命令行模式（回到正常模式）

/*
 * 图像
 */
//mainbmp
void MYCALL1 Clear();					//清除当前图片
inline bool MYCALL1 NoPic();			//判断图片和surface存在

/*
 * D3D
 */
inline void MYCALL1 SetRenderState();	//设置渲染状态
inline void MYCALL1 SetView();			//设置视角，（三维物体的绘制）
inline bool MYCALL1 InfoRender();		//渲染文字信息
inline void MYCALL1 Render();			//渲染
inline bool MYCALL1 ResetDevice();		//重置设备
inline void MYCALL1 OnLostDevice();		//设备丢失动作
inline bool MYCALL1 OnResetDevice(int clientw, int clienth);	//设备重置动作
inline bool MYCALL1 OnResetDevice();	//设备重置动作

//GUI 系统
//CD3DGUISystem *g_gui;
//int fontID1;

//回调函数
void CALLBACK CMDProc(WCHAR *wstr);		//命令行回调
//void CALLBACK GUICallback(int id, int state);
//void CALLBACK TimerProc(HWND hwnd, UINT message, UINT iTimerID, DWORD dwTime);//计时器处理，增加cpu占用率!
