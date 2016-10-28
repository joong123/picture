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

#define PICLISTLEN_INIT				2
#define SIZE_NEAR_PIXEL				8

//在放大倍率小于1的情况下，拟定的surface长宽小于以下尺寸，则强制clip
#define MAX_FORCECLIP_WIDTH			1000
#define MAX_FORCECLIP_HEIGHT		MAX_FORCECLIP_WIDTH

#define MAX_FORCECLIP_WIDTH_DRAG	2000
#define MAX_FORCECLIP_HEIGHT_DRAG	MAX_FORCECLIP_WIDTH_DRAG

//帧率
#define NORMAL_FPS					70
#define MIN_FPS						16

//标志
#define ZOOMFLAG_DELAY				400
#define SIZEFLAG_DELAY				1
#define MOVEFLAG_DELAY				1

//zoom
#define MAX_ZOOM					200
#define MIN_ZOOM					0.001
#define MIN_DELTAZOOM				0.001

//运行模式
#define MODE_PIC					0
#define MODE_CMD					1

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

const string cursorposshow[3] = { "PIC", "BLANK", "OUT WINDOW"};
const string clipshow[2] = {""};


//
//窗口信息&状态
//
HWND mainwnd;							//主窗口句柄
RECT wndrect, clientrect;				//主窗口区域，客户区域
POINT clientsize;						//客户区大小
POINT cursor, lastcursor, lastpos;		//当前鼠标坐标（全局）。lastpos用于拖动时记录上一位置，与lastcursor不可混用
int wbias, hbias;						//窗口与客户区宽度差

int wlimit, hlimit;						//窗口长宽限位
D3DCOLOR backcolor;						//背景色
time_t loopcount;						//总循环计数
UINT8 mode;								//模式
bool ondrag;							//是否在拖动图像状态
bool onzoom;							//是否在放大
bool onsize;
bool onmove;
bool surffailed;						//surface是否创建失败，在有mainbmp情况下
bool sizeEnable;						//是否允许改变窗口大小
bool flagshow;
bool infoshow;

long oldstyle;

POINT curpixel;							//鼠标当前位置对应原图的像素
DWORD pixelcolor;						//鼠标当前位置对应原图的颜色
DWORD screencolor;						//鼠标当前位置对应屏幕的颜色
UINT8 cursorpos;						//当前鼠标位置（种类）

short mousestate;						//当前鼠标位置（用于设置鼠标类型）

//
//计时
//
LARGE_INTEGER frequency, stime, etime;	//
time_t nowtime, lasttime;				//秒级计时
LARGE_INTEGER lastwheeltick, wheeltick;	//滚轮计时
time_t zoomtick, sizetick, movetick;	//标志计时
float fps;								//帧率

//
//D3D
//
LPDIRECT3D9	lpD3D;
LPDIRECT3DDEVICE9 maindevice;
D3DPRESENT_PARAMETERS d3dpp;
D3DDISPLAYMODE displaymode;				//显示模式
D3DCAPS9 caps;							//设备能力
int vertexprocessing;					//vertexprocessing方式

//
//表面和相关信息
//
WCHAR strFileName[MAX_PATH];
D3DSURFACE_DESC surfDesc;
LPDIRECT3DSURFACE9 mainsurf, tempsurf;
bool clip;								//surface创建是否clip（截去客户区外区域）
float zoom;								//直接操控的放大倍数
float oldzoom;							//上一个放大倍数
float realzoom;							//实际使用的放大倍数
float actualzoomx, actualzoomy;			//实际x,y方向上的放大倍数
int zoomw, zoomh;						//放大realzoom倍后拟定surface的尺寸
POINT surfsrc;							//放大realzoom倍的拟定surface的左上角到窗口客户区的向量
POINT surfbase;							//surface拷贝到backbuffer的起始点
RECT surfrect;
bool outsideclient;						//surface和窗口客户区是否无交集（在客户区外）
bool surfclipped;						//当前surface是否clip
bool picclipped;						//当前图片是否clip


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
PicInfo imginfo0;
D3DXIMAGE_INFO imginfo;
BMP mainbmp;

//
//信息显示
//
LPD3DXFONT font;
LPD3DXFONT font2;
RECT textrect;
RECT textrect2;
WCHAR infowstr[1024];

//
//初始化 & 窗口
//
inline short GetSizeType(CPoint);
inline void SetCursor();					//如果允许调整大小，设置鼠标样式
inline void Get2WndRect();					//获得窗口和客户区大小
bool Init();
bool D3DInit();
bool InitDevice();
bool OnDropFile(WCHAR file[]);
bool OnLoadFile(LPWSTR file);
inline void DelayFlag();
inline void ClearFlag();

//
//图像
//
bool FitWnd();
void CenterPic();
void Togglezoom(WPARAM wParam);
bool Zoom(float oldzoom, float zoom);
inline bool RefreshSurf();
inline void DisplayStatusChange(bool winchange = true, bool surfchange = true);			//显示状态改变的处理。refreshsurf或图片位置，窗口改变之后所做
inline void SurfStatusChange();
inline void CalcSurfMapInfo();				//根据当前clip标志和surfsrc、窗口客户区尺寸，计算surface拷贝到backbuffer的起始点和区域
inline void CalcClipped();					
inline void GetCurInfo();					//获取当前鼠标位置对应图片的像素
inline void GetCurPos();					//获取当前鼠标位置(种类)
inline bool GetCurColor();					//根据GetCurPos得到的像素，获取对应的颜色
inline bool NoPic();
inline void Clear();

//
//D3D
//
inline bool InfoRender();
inline bool LoadBackbuffer();
inline bool Render();
bool ResetDevice();
void OnLostDevice();
bool OnResetDevice();

//
//辅助
//
string GetFMT(D3DFORMAT);
inline int Widthof(const RECT &rect);
inline int Heightof(const RECT &rect);
inline bool inside(const POINT &point, const RECT &rect);