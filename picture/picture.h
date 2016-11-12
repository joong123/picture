#pragma once

#include "resource.h"
#include "shellapi.h"
#include <windows.h>
#include <Commdlg.h>
#include "time.h"
#include <io.h>

#include <d3d9.h>
#include <d3dx9tex.h>

#include "BMP.h"
#include "Pic.h"

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

#define MYHT_WND					33

//图片组
#define PICLISTLEN_INIT				2
#define SIZE_NEAR_PIXEL				5

//按键状态掩码
#define KEYSTATEMASK_DOWN			0X8000

//在放大倍率小于1的情况下，假想的surface长宽小于以下尺寸，则强制clip
#define MAX_FORCECLIP_WIDTH			2000
#define MAX_FORCECLIP_HEIGHT		MAX_FORCECLIP_WIDTH

#define MAX_FORCECLIP_WIDTH_DRAG	3000
#define MAX_FORCECLIP_HEIGHT_DRAG	MAX_FORCECLIP_WIDTH_DRAG

//帧率
#define ICONIC_SLEEP				10
#define NORMAL_FPS					70
#define WINDOW_FPS					16
#define MIN_FPS						16

//标志
#define FLAGDELAY_ZOOM				400//onzoom在主循环决定循环频率设置，要设大点
#define FLAGDELAY_SIZE				1
#define FLAGDELAY_MOVE				1
#define FLAGDELAY_SURFREFRESH		100
#define FLAGDELAY_DRAG				100

//zoom
#define MAX_ZOOM					200
#define MIN_ZOOM					0.001f
#define MIN_DELTAZOOM				0.001f

#define DELTAZOOM1					0.006f
#define DELTAZOOM2					0.003f

//运行模式
#define MODE_PIC					0
#define MODE_CMD					1

//窗口模式
#define WINMODE_NORMALWINDOWS		0
#define WINMODE_ROUND				1

//鼠标位置
#define CURSORPOS_PIC				0
#define CURSORPOS_BLANK				1
#define CURSORPOS_OUTWINDOW			2

//文字显示
#define TEXTMARGIN_SIDE				3
#define TEXTMARGIN_TOP				0
#define TEXTMARGIN_BOTTOM			2
#define TEXTMARGIN_ROW				2

//颜色
#define COLOR_BKG0					D3DCOLOR_ARGB(200, 238, 243, 249)
#define COLOR_BKG1					D3DCOLOR_ARGB(200, 255, 255, 255)
#define COLOR_TEXT0					D3DCOLOR_ARGB(160, 255, 255, 255)
#define COLOR_TEXT1					D3DCOLOR_ARGB(180, 90, 0, 255)
#define COLOR_BKG					COLOR_BKG0

const string cursorposshow[3] = { "PIC", "BLANK", "OUTSIDE CLIENT"};
const string clipshow[2] = {""};
const WCHAR yesno1[2] = { L'×', L'√' };
const WCHAR yesno2[2] = { L'×', L'●' };//●

//
//窗口信息&状态
//
HWND mainwnd;							//主窗口句柄
RECT wndrect, clientrect;				//主窗口区域，客户区域
POINT clientsize;						//客户区大小
POINT cursor, lastcursor, lastpos;		//当前鼠标坐标（全局）。lastpos用于拖动时记录上一位置，与lastcursor不可混用
int wbias, hbias;						//窗口与客户区宽度差
int wlimit, hlimit;						//窗口长宽限位

//状态标志
bool ondrag;							//是否在拖动图像状态
bool dragging;							//是否在拖动图像状态
bool onzoom;							//是否在放大
bool onsize;							//是否在缩放窗口
bool onmove;							//是否在移动窗口
bool surfrefresh;						//surface是否在刷新
bool picclipped;						//当前图片是否clip
//选项标志
UINT8 mode;								//模式
UINT8 winmode;							//窗口模式
D3DCOLOR backcolor;						//背景色
bool sizeEnable;						//是否允许改变窗口大小
bool easymoveEnable;
bool flagshow;							//是否显示状态标志信息		
bool infoshow;							//是否显示文字信息
bool fpslimit;
bool screencoloron;						//是否获取屏幕颜色


POINT picpixel;							//鼠标当前位置对应原图的像素
DWORD picpixelcolor;					//鼠标当前位置对应原图的颜色
DWORD screencolor;						//鼠标当前位置对应屏幕的颜色
UINT8 cursorpos;						//当前鼠标位置（种类，处于哪个区域：图片、空白、窗口外）

short mousestate;						//当前鼠标位置（用于设置鼠标类型）

//
//计时
//
time_t loopcount;						//总循环计数
LARGE_INTEGER frequency, stime, etime;	//高精度计时
time_t nowtime, lasttime;				//秒级计时
LARGE_INTEGER lastwheeltick, wheeltick;	//滚轮计时
time_t zoomtick, sizetick, movetick\
	, surfrefreshtick, dragtick;		//标志计时
float fps, avgfps, cvgfps, showfps, showavgfps;						//帧率
ULONGLONG fpscount;

//
//D3D
//
LPDIRECT3D9	lpD3D;
LPDIRECT3DDEVICE9 maindevice;			//主设备
D3DPRESENT_PARAMETERS d3dpp;
D3DDISPLAYMODE displaymode;				//显示模式
D3DCAPS9 caps;							//设备能力
int vertexprocessing;					//vertexprocessing方式

//
//表面和相关信息
//
WCHAR strFileName[MAX_PATH];			//文件名
D3DSURFACE_DESC surfDesc;				//主surface信息
LPDIRECT3DSURFACE9 mainsurf, tempsurf;	//主surface，和临时surface

float zoom;								//直接操控的放大倍数
float oldzoom;							//上一个放大倍数
float realzoom;							//实际使用的放大倍数
float actualzoomx, actualzoomy;			//实际x,y方向上的放大倍数
bool surffailed;						//surface是否创建失败，在有mainbmp情况下
bool outsideclient;						//surface和窗口客户区是否无交集（在客户区外）
bool clip;								//surface创建是否clip（截去客户区外区域）
bool surfclipped;						//当前surface是否clip
int zoomw, zoomh;						//放大realzoom倍后假想surface的尺寸
POINT surfsrc;							//放大realzoom倍的假想surface的左上角到窗口客户区的向量
POINT surfbase;							//surface拷贝到backbuffer, backbuffer的起始点
RECT surfrect;							//surface要拷贝到backbuffer的区域


//_finddata_t filegroup;
//UINT32 picnum;
UINT32 piccount;
UINT32 piclistlen;
PicPack *piclist;
PicPack *mainpack;
//
//位图组
//
//属于位图组的surface信息：(surfsrc)
PicInfo imginfo0;						//图片信息
D3DXIMAGE_INFO imginfo;
BMP mainbmp;							//当前图片

//
//信息显示
//
LPD3DXFONT font;						//显示字体
LPD3DXFONT font2;
RECT textrect;							//显示字体区域
RECT textrect2;
WCHAR infowstr[512];					//显示信息缓存

//
//初始化 & 窗口
//
//窗口
void SetRoundWnd();
inline void Get2WndRect();				//获得窗口和客户区大小
//鼠标
inline short GetSizeType(CPoint);		//根据当前坐标，获得鼠标类型（改变窗口大小）
inline void SetCursor();				//如果允许调整大小，设置鼠标样式
//初始化
bool Init();							//初始化
//D3D初始化
bool D3DInit();							//D3D初始化
bool InitDevice();						//创建设备，设置渲染状态
//文件
bool OnDropFile(WCHAR file[]);			//拖入图片，统一处理添加图片
bool OnLoadFile(LPWSTR file);			//初始化时载入图片
//标志
inline void DelayFlag();				//延迟标志的消除
inline void ClearFlag();				//清除活跃标志
//操作
void BeginDragListen();
void EndDragListen();

//
//图像
//
bool FitWnd();							//设置标准窗口尺寸
void CenterPic();						//将图片居中（窗口）
void SurfAdjustZoom(WPARAM wParam);		//根据调整值调整缩放倍率
void SurfMove(int dx, int dy);									//surface移动，不带surface更新，外部条件更新	
void SurfZoom(float oldzoom, float zoom, POINT cursor);			//缩放
void SurfRotate(POINT center, POINT corsor, POINT cursor2);		//旋转
void SurfManipulate(POINT center, POINT corsor, POINT cursor2);	//旋转点+偏移，自由操控
inline bool SurfRenew();				//重新生成surface
inline void SurfStatusChange();			//surface状态改变，RefreshSurf后执行
inline void CalcSurfMapInfo();			//根据当前clip标志和surfsrc、窗口客户区尺寸，计算surface拷贝到backbuffer的起始点和区域

inline void CalcClipped();				//计算当前图片(假象放大zoom倍的未经clip的surface)clip状态
inline void Clear();					//清除当前图片
inline bool NoPic();					//判断图片和surface存在
inline bool NoSurf();					//判断surface存在
//获取信息
inline void GetCurInfo();				//获取当前鼠标位置对应图片的像素
inline void GetCurPos();				//获取当前鼠标位置(种类)
inline bool GetCurColor();				//根据GetCurPos得到的像素，获取对应的颜色

//
//D3D
//
inline bool InfoRender();				//渲染文字信息
inline bool LoadBackbuffer();			//将当前surface载入backbuffer，渲染之前执行
inline bool Render();					//渲染
bool ResetDevice();						//重置设备
void OnLostDevice();					//设备丢失动作
bool OnResetDevice();					//设备重置动作

//
//辅助
//
string GetFMT(D3DFORMAT);				//获得图片格式字符串
inline int Widthof(const RECT &rect);	//求RECT宽度
inline int Heightof(const RECT &rect);	//求RECT高度
inline bool inside(const POINT &point, const RECT &rect);//判断点在RECT内