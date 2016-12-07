#pragma once

/*
//surfer info

LPDIRECT3DSURFACE9 mainsurf;			//主surface
D3DSURFACE_DESC surfDesc;				//主surface信息

LARGE_INTEGER lastwheeltick, wheeltick;	//滚轮计时
POINT basepoint;//用于dragzoom
POINT piccenter;

float surfzoom;							//实际使用的放大倍数
float oldsurfzoom;						//上一次生成surface参数用的放大倍数
float actualzoomx, actualzoomy;			//实际x,y方向上的放大倍数
int zoomw, zoomh;						//放大realzoom倍后假想surface的尺寸

bool surffailed;						//surface是否创建失败，在有mainbmp情况下
bool clip;								//surface是否clip（截去客户区外区域）
bool surfclipped;						//当前surface是否clip
bool picclipped;						//当前图片是否clip
bool outsideclient;						//surface和窗口客户区是否无交集（在客户区外）
POINT surfsrc;							//放大realzoom倍的假想surface的左上角到窗口客户区的向量
POINT surfbase;							//surface拷贝到backbuffer, backbuffer的起始点
RECT surfrect;							//surface要拷贝到backbuffer的区域

SurfParamInit()
:	mainsurf = NULL; 
	basepoint = { 0,0 };
	piccenter = { 0,0 };

	surfzoom = 1.0f;
	oldsurfzoom = 1.0f;
	actualzoomx = 1.0f;
	actualzoomy = 1.0f;
	zoomw = 0;
	zoomh = 0;

	surffailed = true;
	clip = true;
	surfclipped = false;
	picclipped = false;
	outsideclient = false;
	surfsrc = { 0,0 };
	surfbase = { 0,0 };
	surfrect = { 0,0,0,0 };

SurfPicInfoInit(int picw, int pich)
:	surfzoom => zoomw, zoomh
	surfsrc	=> piccenter

SurfHoming()
:	=> surfsrc
	zoomw, zoomh, surfsrc => piccenter

SurfLocate(LONG x, LONG y)
:	x, y => surfsrc
	zoomw, zoomh, surfsrc => piccenter
	
SurfMove(int dx, int dy)
:	dx, dy => surfsrc
	zoomw, zoomh, surfsrc => piccenter
	
SurfSetZoom(float z)
:	z => surfzoom

SurfZoom(POINT client2cursor, bool stable)
:	surfzoom, BMP => zoomw, zoomh
	zoomw, zoomh, BMP => actualzoomx&y
	zoomw, zoomh, piccenter/client2cursor => surfsrc
	zoomw, zoomh, surfsrc => piccenter
	=> clip
	=> oldsurfzoom


*/