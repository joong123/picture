#pragma once 

#include <new>
#include <iostream>
#include <stdlib.h>
#include <winnt.h>
#include <strsafe.h>

#include "picfile.h"
#include "generalgeo.h"

#include <d3d9.h>
#include <d3dx9tex.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

// 透明背景效果
#define TRANSPARENTBACK_MASK			0b1000
#define TRANSPARENTBACK_HOLLOWCOLOR		D3DCOLOR_XRGB(255, 255, 255)
#define TRANSPARENTBACK_FILLCOLOR		D3DCOLOR_XRGB(204, 204, 204)
#define TRANSPARENTBACK_HOLLOWDENSITY	255
#define TRANSPARENTBACK_FILLDENSITY		230

#define RGB2GRAY(R, G, B)				(((R)*38 + (G)*75 + (B)*15) >> 7)

// 通用
#define SAFE_RELEASE(p)					{ if (p) { (p)->Release(); (p) = NULL; } }
#define SAFE_DELETE(p)					{ if (p) { delete (p); (p) = NULL;} }
#define SAFE_DELETE_LIST(p)				{ if (p) { delete[] (p); (p) = NULL;} }
#define DELETE_LIST(p)					{ delete[] (p); (p) = NULL; }
#define FAILED_RETURN(hr)				{ if (FAILED(hr)) return hr; }
#define FAILED_RETURN_FALSE(hr)			{ if (FAILED(hr)) return false; }
#define FALSE_RETURN(r)					{ if (!(r)) return false;}

#define MALLOC(type, size)				(type *)malloc(size*sizeof(type))
#define REALLOC(P, type, size)			P = (type *)realloc(P, size*sizeof(type));

#define _CDECL							__cdecl
#define _STDCALL						__stdcall
#define _FASTCALL						__fastcall
#define MYCALL1							_CDECL

// 图片缩放、旋转
#define ZOOM_MAX						INT64_MAX
#define ZOOM_MIN						0.0

#define ZOOMFACTOR_DRAG					0.010	// 缩放灵敏度
#define ZOOMFACTOR_WHEEL				0.025
#define ROTATEFACTOR_DRAG				0.008	// 缩放灵敏度

#define ZOOM_MAXCONTROLED				0.618	// surface小于窗口几倍大小，进行放大限位

#define NN_MIN							2.0		// 大于这个放大倍率使用 NN 采样
#define MAXSAMPLE_WIDTH_UPPERLIMIT		20000	// 大于这个尺寸图片不进行最大采样 MAXSAMPLE
#define MAXSAMPLE_HEIGHT_UPPERLIMIT		MAXSAMPLE_WIDTH_UPPERLIMIT

#define MIN_NORMALRENEWSIZE				300		// 图片小于这个尺寸，在加速时才允许使用 SingleSample ( 太大缩放过程可能有卡顿 )

#define MIN_FORCECLIPSIZE				1500	// 图片大于这个尺寸，则设置 bClip。增加缩放速度

#define MAX_NOCLIPSIZE_DRAG				3000	// 图片小于这个尺寸，拖动图片时取消 bClip。增加拖动速度

// 图片列表
#define PICPACKLIST_SIZE_INIT			3
#define PICPACKLIST_SIZE_INCREMENT		2

// 一些 Surfer 数据
#define PICMAXSCALE_INIT				0.8

// 鼠标位置
#define CURSORPOS_PIC					0
#define CURSORPOS_BLANK					1
#define CURSORPOS_OUTWINDOW				2

// 颜色
#define BACKCOLOR_INIT					0xFF888888
#ifndef SETALPHA
#define SETALPHA(C, A)					((C & 0xFFFFFF) | (A << 24))
#endif

using std::string;
using std::bad_alloc;
using std::endl;
using std::cerr;

// 颜色类型，用于颜色精确计算
typedef float	COLORTYPE;
// AUXTYPE为重采样 ( 或插值 ) 算法中影响极限状态精度的关键类型
//  定义为 float 的话，超大倍率缩放，Area、Single 算法的颜色计算有较明显误差
typedef double	AUXTYPE;
// ZOOMTYPE为重采样 ( 或插值 ) 算法中影响极限状态像素位置精度的关键类型
//  定义为 float 时，超大倍率缩放，像素边界位置有明显偏差
typedef double	ZOOMTYPE;

struct COLOR4f {
	COLORTYPE a;
	COLORTYPE r;
	COLORTYPE g;
	COLORTYPE b;

	COLORTYPE w;// 权重（累加）

	COLOR4f();
	COLOR4f(COLORTYPE f);

	/*inline DWORD AlphaBlend(DWORD back) {
		byte alpha = (byte)a;
		DWORD blend = 0;
		blend += (DWORD)((a / 2.0 + ((back >> 24) & 0xFF) / 2.0) / 255) << 24;
		blend += (DWORD)((r*alpha + ((back >> 16) & 0xFF)*(255 - alpha)) / 255) << 16;
		blend += (DWORD)((g*alpha + ((back >> 8) & 0xFF)*(255 - alpha)) / 255) << 8;
		blend += (DWORD)((b*alpha + (back & 0xFF)*(255 - alpha)) / 255);
		return blend;
	}*/
};

COLOR4f *COLOR4fNormalize(COLOR4f *c4f);
COLOR4f *COLOR4fWNormalize(COLOR4f *c4f);
COLOR4f *COLOR4fAccum(COLOR4f *c4f, DWORD dw, const COLORTYPE weight = (COLORTYPE)1.0);
COLOR4f *COLOR4fAssign(COLOR4f *c4f, COLORTYPE c);
DWORD COLOR4fGetDW(COLOR4f *c4f);

class ALPHABLEND {
public:
	// 对单一（或 2 种）destcolor ( 只能是灰度，不能是任意颜色 ) 进行任意 srccolor 和 alpha 的 alphablend 查询表
	static DWORD lastcolor1, lastresult1;
	static DWORD lastcolor2, lastresult2;
	static byte **alphablendtable1, **alphablendtable2;

	// 对任意 destcolor、任意 srccolor 进行 alphablend 的查询表
	static byte ***alphablendcube1;

	ALPHABLEND() {}

	// 内联示例
	/*static inline DWORD AlphaBlend1(COLOR4f src) {
		byte alpha = roundf2b(src.a);
		DWORD result = 0;
		result += (DWORD)alphablendtable1[(byte)(src.r)][alpha] << 16;
		result += (DWORD)alphablendtable1[(byte)(src.g)][alpha] << 8;
		result += (DWORD)alphablendtable1[(byte)(src.b)][alpha];

		return result;
	}*/
	// 内联示例
	/*static inline DWORD AlphaBlend1(DWORD srccolor) {
		if (srccolor == ALPHABLEND::lastcolor1)
		{
			return ALPHABLEND::lastresult1;
		}
		else
		{
			byte *src = (byte*)&srccolor;
			byte *dest = (byte*)&lastresult1;

			dest[2] = alphablendtable1[src[2]][src[3]];
			dest[1] = alphablendtable1[src[1]][src[3]];
			dest[0] = alphablendtable1[src[0]][src[3]];

			ALPHABLEND::lastcolor1 = srccolor;
			ALPHABLEND::lastresult1 = *(DWORD*)dest;

			return lastresult1;
		}
	}*/
	static void InitAlphBlendTable(byte color1, byte color2);
	static void InitAlphBlendTCube();
};

COLOR4f RGB2HSV_F3(DWORD c1);
DWORD RGB2HSV(DWORD color);
DWORD HSV2RGB_F3(COLOR4f color);
DWORD HSV2RGB(DWORD color);

// 采样策略
enum SAMPLE_METHOD {
	SAMPLE_UNKNOWN,
	SAMPLE_NN,
	SAMPLE_BILINEAR,
	SAMPLE_SINGLE,
	SAMPLE_CUBE,
	SAMPLE_AREA
};
const char* GetSampleSchemaStr(SAMPLE_METHOD sm);

class BMP {
private:
	/*static void __cdecl newhandlerfunc()
	{
		cerr << "Allocate failed in class BMP" << endl;
		abort();
	}*/
public:
	int width, height;
	byte **data;
	bool bAlpha;		//是否有透明通道

	BMP();
	~BMP();

	// 操作
	void SetAlpha(bool alpha);
	bool Load(LPDIRECT3DSURFACE9 surf);			//从 surface 生成对应大小图片
	bool UpLoad(LPDIRECT3DSURFACE9 surf);		//拷贝到 surface，从 {0, 0} 对齐开始，尽可能多的拷贝

	/*
	 * 对于已定制的surface的拷贝
	 */
	/***************************************************
	surfData: 目标地址空间，surfSize: 目标尺寸
	ptSurfOffset: surface 相对原图偏移（用于原图选区）

	双线性插值的两种策略 ( 有色差，无色差 ) 在 Sample_Single 中详细定义
	****************************************************/
	// 最大无损失采样 ( 缩小效果较好，缩小速度最慢 )
	bool MYCALL1 Sample_Area(DWORD *surfData, const POINT &surfSize
		, ZOOMTYPE zoom, const POINTi64 &ptSurfOffset = POINTi64(0,0));
	// NN 和 bilinear 结合，跨交界处像素使用混合
	bool MYCALL1 Sample_Single(DWORD *surfData, const POINT &surfSize
		, ZOOMTYPE zoom, const POINTi64 &ptSurfOffset = POINTi64(0, 0));

	/*
	 * 图片处理
	 */
	void TestInc();
	void TestAlpha(BMP *pb);

	bool MYCALL1 Gray();

	bool MYCALL1 Inverse();

	bool MYCALL1 LOSE_R();
	bool MYCALL1 LOSE_G();
	bool MYCALL1 LOSE_B();

	bool MYCALL1 RGB2HSV();

	bool Diff(BMP *pre);

	// 信息
	inline bool MYCALL1 GetPixel(int x, int y, DWORD *dest)
	{
		if (data == NULL || data[y] == NULL || dest == NULL)
			return false;

		if (x < 0 || y < 0 || x >= width || y >= height)
			return false;

		*dest = ((DWORD*)data[y])[x];

		return true;
	}

	// 状态
	void MYCALL1 Clear();
	inline bool MYCALL1 isEmpty()
	{
		return (data == NULL);
	}
	inline bool MYCALL1 isNotEmpty()
	{
		return (data != NULL);
	}
};

// 最近邻近 ( 放大缩小都可以，效果不会太好 )
bool MYCALL1 Sample_NN(BMP *pBmp, DWORD *surfData, const POINT &surfSize
	, ZOOMTYPE zoom, POINTi64 &ptSurfOffset = POINTi64(0, 0));
bool MYCALL1 Sample_NNRotate(BMP *pBmp, DWORD *surfData, const POINT &surfSize
	, ZOOMTYPE zoom, float rotate = 0, POINTi64 &ptSurfOffset = POINTi64(0, 0));
// 双线性 ( 缩小过多或放大过多效果差 )
bool MYCALL1 Sample_BiLinear(BMP *pBmp, DWORD *surfData, const POINT &surfSize
	, ZOOMTYPE zoom, const POINTi64 &ptSurfOffset = POINTi64(0, 0));

string GetFMTStr(D3DFORMAT);

// surfer 中用于捆绑于图片的信息
//  surfer 切换图片后，由 PicPack 来保存这些 surfer 里用到的信息
struct PIC_LIVEINFO {
	POINTi64 src;
	double zoom;
	float rotate;
};

class PicPack {
private:
	BMP *pBmp;
	PicInfo myPicInfo;				// 自定图片信息
	D3DXIMAGE_INFO D3DPicInfo;		// 图片信息
	WCHAR strFileName[MAX_PATH];	// 文件名

	bool bSaved;
	bool bHasDirectory;

	// 与 surface 捆绑的信息
	PIC_LIVEINFO liveInfo;

	// 信息
	WCHAR picInfoStr[256];
public:
	PicPack();
	~PicPack();

	inline bool isEmpty() const
	{
		return pBmp->isEmpty();
	}
	inline bool isNotEmpty() const
	{
		return pBmp->isNotEmpty();
	}
	inline const PIC_LIVEINFO &GetLiveInfo() const
	{
		return liveInfo;
	}
	inline void SetLiveInfo(PIC_LIVEINFO info)
	{
		liveInfo = info;
	}
	inline BMP* GetPBMP() const
	{
		return pBmp;
	}
	const WCHAR *GetFileName() const;

	HRESULT LoadFile(LPDIRECT3DDEVICE9 pDevice, WCHAR[]);
	bool SaveFile(LPDIRECT3DDEVICE9 pDevice, WCHAR[]);

	void UpdatePicInfoStr();
	const WCHAR *GetPicInfoStr() const;
};

class PicPackList {
private:
	PicPack **data;
	int size;
	int count;

	int cur;

	bool SetSizeInternal(int newSize);
public:
	PicPack *pLivePicPack;
	BMP *pLiveBMP;

	PicPackList();

	bool isEmpty() const
	{
		return count == 0;
	}
	int GetCurPos() const;
	int GetCount() const;

	bool Add(PicPack *newpp);

	PicPack *SetPicPack(int idx);
	PicPack *SetTailPicPack();
	PicPack *SetPrev();
	PicPack *GetPrev();
	PicPack *SetNext();
	PicPack *Drop();
};

class SurferBase {
protected:
	BMP *pBmp;								// BMP 指针·捆绑
	UINT *pBufferW, *pBufferH;				// 目标 surface 尺寸·捆绑。TODO: UINT 越界处理：超过 LONG
	bool bHasPic;							// 是否有图片

	// "BMP" 映射 "surface" 参数
	double			zoom;					// 放大倍数
	double			lastZoom;				// 上一个放大倍数
	float			rotate;					// 旋转量
	float			lastRot;				// 上一个旋转量
	// 映射状态
	INT64			zoomW, zoomH;			// 放大 zoom 倍后 surface 尺寸（不 clip 的情况），【减少临时运算量，需要手动更新】(需要手动更新场合：缩放，捆绑图片)
	double			zoomXActual, zoomYActual;// 实际 x,y 方向上的放大倍数【暂时只用来查看】

	// "源 surface" 映射 "目标 surface" 参数
	// TODO: ptSurfBase 是 INT64，处理超出 LONG_MAX 时对其他数据的影响，比如：surfSize、ptSurfDest、rcSurfSrc
	POINTi64		ptSurfBase;				// 放大 zoom 倍的假想 surface 相对窗口客户区的位置（左上角）
	// 映射状态
	POINTi64		ptSurfOffset;			// clip 后的 surface 相对于假想 surface 的偏移
	POINT			surfSize;				// surface 尺寸
	POINT			lastSurfSize;			// 存储最后一次 surfRenew 时的尺寸

	// 功能参数
	POINT			ptZoomBase;				// 用于 dragzoom 的中心点
	double			ptScaleX;				// ptZoomBase 相对图片的比例值
	double			ptScaleY;

	// 计时
	LARGE_INTEGER wheeltick, lastwheeltick;	// 滚轮计时
	LARGE_INTEGER freq;						// 频率

protected:
	void			CalcBindState();	// 更新捆绑状态 ( 与图片捆绑状态 )

	void			GetZoomSize(double z, double r, INT64 *pZoomW, INT64 *pZoomH);// 计算即时假象 surface 尺寸
	// 所有相关参数的更新，surface 生成前准备
	// ptSurfBase 和 zoomWH 唯一确定生成 surface 的性质（位置和大小）
	// actualZoomXY、ptSurfOffset 等其他参数用于辅助 surface 生成，起到信息查看、数据缓存的作用
	// 第 1 层：zoom、rotate、clip 改变引起的 zoomWH、actualZoomXY、ptSurfBase 改变
	inline void		OnZoomChange();// zoom 更新后需要立即执行的
	void			PostZoomChange(bool bPosRefresh = false, bool stable = true);
	void			PostRotateChange(bool bPosRefresh = false);
	// 第 2 层：ptSurfOffset、bPicClipped、bOutClient、surfSize 参数更新
	inline void		CalcSurfSize();// 计算 surface 尺寸
	inline void		OnSurfPosChange(); // pos 变化后必须执行

public:
	// "BMP" 映射 "surface" 选项
	bool			bClip;					// surface 是否 clip ( 截去客户区外区域 )

	// 映射状态
	bool			bPicClipped;			// 当前图片是否 clip ( bPicClipped 和 bOutClient 只在特定函数中计算，需要确保被更新 )
	bool			bOutClient;				// surface 和窗口客户区是否无交集 ( 在客户区外 )

	SurferBase();
	void			ClearPic();
	void			Clear();

	// 信息获取
	inline POINTi64	GetBase() const;
	inline double	GetZoom() const;
	inline float	GetRotate() const;
	inline INT64	GetZoomWidth() const;
	inline INT64	GetZoomHeight() const;

	// 参数级调整阶段 ( 平移、缩放、旋转、clip 设置)
	void			SetClip(bool clip);
	void			SetBasePoint(POINT ptB);
	void			SurfHomePR();		// surface 回原点
	void			SurfLocatePR(POINTi64 base);
	void			SurfCenterPR(int bufferW, int bufferH);
	void			SurfMovePR(INT64 dx, INT64 dy);
	void			SurfSetZoomPR(double zoom, bool bPosRefresh = false, bool stable = true);// 设置 surface 缩放倍率
	void			SurfSetRotatePR(float rotate, bool bPosRefresh = false);
	void			SurfAdjustZoom_DragPR(int wParam, bool bPosRefresh = false, bool stable = true);
	void			SurfAdjustZoom_WheelPR(int wParam, bool bPosRefresh = false, bool stable = true);
	void			SurfAdjustRotate_DragPR(int wParam, bool bPosRefresh = false);
};

inline POINTi64 SurferBase::GetBase() const
{
	return ptSurfBase;
}

inline double SurferBase::GetZoom() const
{
	return zoom;
}

inline float SurferBase::GetRotate() const
{
	return rotate;
}

inline INT64 SurferBase::GetZoomWidth() const
{
	return zoomW;
}

inline INT64 SurferBase::GetZoomHeight() const
{
	return zoomH;
}

inline void SurferBase::CalcSurfSize()
{
	if (pBufferW != NULL && pBufferH != NULL)
	{
		POINTi64 size;// TODO:转为 surfSize 会不会有溢出情况
		SETPOINT(size, zoomW, zoomH);
		if (bClip)
		{
			// 下列计算，原来没使用 min、max，只使用 min、max里第一个数，但是计算结果可能为负数
			if (ptSurfBase.x < 0)// surface 左侧 clip
				size.x += max(ptSurfBase.x, -zoomW);
			if (ptSurfBase.y < 0)// surface 上侧 clip
				size.y += max(ptSurfBase.y, -zoomH);
			if (ptSurfBase.x + zoomW > (INT64)*pBufferW)// surface 右侧 clip
				size.x -= min(ptSurfBase.x + zoomW - (INT64)*pBufferW, size.x);
			if (ptSurfBase.y + zoomH > (INT64)*pBufferH)// surface 下侧 clip
				size.y -= min(ptSurfBase.y + zoomH - (INT64)*pBufferH, size.y);
		}
		SETPOINT(surfSize, (LONG)size.x, (LONG)size.y);
	}
}

inline void SurferBase::GetZoomSize(double z, double r, INT64 * pZoomW, INT64 * pZoomH)
{
	if (bHasPic)
	{
		if (r == 0)
		{
			if (pZoomW != NULL)
				*pZoomW = (INT64)(z*pBmp->width);
			if (pZoomH != NULL)
				*pZoomH = (INT64)(z*pBmp->height);
		}
		else
		{
			float angleInnerV = atan((float)pBmp->width / pBmp->height);
			float angleL = angleInnerV + r;
			float angleR = angleInnerV - r;
			double diagLen = sqrt((double)SQ(pBmp->width) + (double)SQ(pBmp->height));

			// rotate 情况取 ceil，因为 rotate 采样像素计算直接用 zoom，而没用精确缩放值
			//  不 ceil 可能导致某些情况图片显示不完全，如 24*22 图片放大 70.84 倍旋转 14'40'3.79"
			if (pZoomW != NULL)
				*pZoomW = (INT64)ceil(z*diagLen*max(abs(sin(angleL)), abs(sin(angleR))));
			if (pZoomH != NULL)
				*pZoomH = (INT64)ceil(z*diagLen*max(abs(cos(angleL)), abs(cos(angleR))));
		}
	}
}

inline void SurferBase::OnSurfPosChange()
{
	//bNeedRenew = true;
	
	// 更新 ptSurfOffset
	ptSurfOffset = { 0, 0 };
	if (bClip)
	{
		if (ptSurfBase.x < 0)// surface 左侧需 clip
			ptSurfOffset.x = -ptSurfBase.x;
		if (ptSurfBase.y < 0)// surface 上侧需 clip
			ptSurfOffset.y = -ptSurfBase.y;
	}

	if (pBufferW != NULL && pBufferH != NULL)
	{
		// 计算 picture clip、OutClient 状态
		bPicClipped = (
			ptSurfBase.x < 0 || ptSurfBase.y < 0 ||			// surface 左上上侧需 clip
			ptSurfBase.x + zoomW > (INT64)*pBufferW || 		// surface 右侧需 clip
			ptSurfBase.y + zoomH > (INT64)*pBufferH			// surface 下侧需 clip
			);
		bOutClient = (
			ptSurfBase.x < - zoomW || ptSurfBase.y < -zoomH ||// 区域超出 surface 右或下
			ptSurfBase.x >= (INT64)*pBufferW || 			// 区域右下不够 surface 左或上
			ptSurfBase.y >= (INT64)*pBufferH
			);

		// 计算需生成的 surface 尺寸
		CalcSurfSize();
	}
}

inline void	SurferBase::OnZoomChange()
{
	//bNeedRenew = true;

	if (bHasPic)
	{
		// 更新假象 surface 尺寸
		GetZoomSize(zoom, rotate, &zoomW, &zoomH);

		// 更新精确缩放值
		zoomXActual = (double)zoomW / pBmp->width;
		zoomYActual = (double)zoomH / pBmp->height;
	}
}


class Surfer :public SurferBase {
protected:
	LPDIRECT3DSURFACE9 surf;				// surface，生成的图片
	LPDIRECT3DDEVICE9 pDevice;				// 设备指针·捆绑
	bool bHasDevice, bHasBoth;				// 是否具备捆绑值 ( 捆绑对象 )

	/******************************************************************************
	ptSurfBase、zoom、rotate 逻辑上可分离，作为属于 BMP 的信息。
	*******************************************************************************/
	// "源 surface" 映射 "目标 surface" 直接参数
	POINT			ptSurfDest;				// surface 拷贝到 backbuffer, backbuffer 的起始点
	RECT			rcSurfSrc;				// surface 要拷贝到 backbuffer 的区域
	// "BMP" 映射 "surface" 状态
	bool			bSurfFailed;			// surface 是否创建失败【暂时只用来查看】
	bool			bSurfClipped;			// 当前 surface 是否 clip

	// 鼠标信息
	POINT			ptCursorPixel;			// 鼠标当前位置对应原图的像素
	DWORD			cursorColor;			// 鼠标当前位置对应原图的像素颜色
	UINT8			cursorPos;				// 当前鼠标位置 （ 处于哪个区域： 图片、空白、窗口外 ）

	// 计时 (性能) 测试
	double time;							// 采样计时
	double timeAvg;							// 采样计时
	double timeMin;							// 采样计时
	double timeMax;							// 采样计时
	int count;
	//int surfrenewcount;					// surface 更新计数
	float renewTime;						// surface 更新耗时

	// 信息
	SAMPLE_METHOD sampleMethod;				// 采样策略
	WCHAR strSurfInfo[512];					// 信息字符串

protected:
	void			CalcBindState();		// 更新捆绑标志 ( 捆绑的状态 )

	// 第 3 层：ptSurfDest、rcSurfSrc 参数更新 ( 前两层见 SurferBase )
	inline void		CalcMapInfo();			// 【bHasDevice】计算映射到目标 surface 参数
	// 第 4 层：bSurfFailed、bSurfClipped 参数更新
	//  见 SurfRenew() 函数

public:
	Surfer();
	~Surfer();

// 基本操作
	void			Clear();
	void			ClearTimeInfo();
	inline bool		IsSurfNull() const
	{
		return surf == NULL;
	}
	inline bool		IsSurfNotNull() const
	{
		return surf != NULL;
	}
	inline bool		GetBoolSurfClipped() const
	{
		return bSurfClipped;
	}

// 设备
	bool			BindDevice(LPDIRECT3DDEVICE9 dev);	// 捆绑设备
	bool			BindBuf(UINT * pBufW, UINT *pBufH);	// 捆绑目标 surface 尺寸

// 图片
	bool			BindPic(PicPack *pPack, bool renew = true);// 捆绑图片 ( 可以捆绑 NULL 来清除，或加载/切换图片 )
	bool			DeBindPic(PicPack *pPack);			// 解绑图片
	void			ClearPic();							// 清除，释放与图片的捆绑

	void			SetBasePoint(POINT ptB);			// 设置 ( 缩放 ) 基准点

// surface 刷新
	inline bool		SurfRenew(bool acce = false);		// 【bHasBoth】surface 刷新，返回 surface 是否刷新
	inline HRESULT	Render() const;						// 【surf、bHasDevice】渲染

// surface 定制操作
	// 【bHasBoth】拖动事件处理，返回 surface 是否刷新
	bool			OnDrag_Custom(POINTi64 offSet, bool bAcce = true);
	// 【bHasBoth】窗口拉伸事件处理，返回 surface 是否刷新
	bool			OnWinsize_Custom();					// 和 OnDrag_Custom 差不多
	bool			SurfSuit(int w, int h);

// 信息
	void			UpdateInfo();						// 更新信息字符串
	// 【bHasBoth：更新信息需要】更新当前信息 ( 根据当前鼠标位置对应图片的像素 ) ，并更新信息字符串
	void			GetCurInfo(POINT *cursor);
	inline const WCHAR *GetInfoStr() const				// 获得信息字符串
	{
		return strSurfInfo;
	}
};

#define SAMPLE_TIMETEST

inline bool Surfer::SurfRenew(bool acce)
{
	bool bRenewed = false;// surface 是否成功更新
	if (bHasBoth)
	{
		LARGE_INTEGER start, end;// surface 更新计时
		QueryPerformanceCounter(&start);

		// 计算 bSurfClipped	：实际 surface 是否 clip
		//  放在 SurfRenew() 中而不是 OnSurfPosChange() 中，
		//  是为了使 surface 相关参数更新跟随 surface 的更新
		bSurfClipped = (bClip && bPicClipped);

		// 生成 surface
		//  如果原 surface 为空，或者 surface 尺寸更新
		if (surf == NULL || lastSurfSize.x != surfSize.x || lastSurfSize.y != surfSize.y)
		{
			// 清除原 surface
			SAFE_RELEASE(surf);
			// 新建 surface
			if (FAILED(pDevice->CreateOffscreenPlainSurface(
				surfSize.x, surfSize.y
				, D3DFMT_A8R8G8B8
				, D3DPOOL_SYSTEMMEM// D3DPOOL_DEFAULT 的话不能显示
				, &surf
				, NULL)))
			{
				SAFE_RELEASE(surf);
				bSurfFailed = true;
				return false;
			}
		}

		// 锁定 surface 区域
		D3DLOCKED_RECT lockedRect;
		if (FAILED(surf->LockRect(&lockedRect, NULL, NULL)))
		{
			SAFE_RELEASE(surf);
			bSurfFailed = true;
			return false;
		}
		DWORD *surfData = (DWORD*)lockedRect.pBits;

		// 采样，分成加速和正常两种模式。多种映射策略的选择
		bool bSampled = false;
		if (rotate != 0)
			bSampled = Sample_NNRotate(pBmp, surfData, surfSize, (ZOOMTYPE)zoom,rotate, ptSurfOffset);
		else
		{
			if (acce)
			{
				if (zoom > NN_MIN || surfSize.x > MIN_NORMALRENEWSIZE || surfSize.y > MIN_NORMALRENEWSIZE)
				{
					bSampled = Sample_NN(pBmp, surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);
					sampleMethod = SAMPLE_NN;
				}
				else
				{
					bSampled = Sample_BiLinear(pBmp, surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);
					sampleMethod = SAMPLE_BILINEAR;
				}
			}
			else
			{
				if (zoom > NN_MIN)
				{
					// 近邻重采样
					bSampled = Sample_NN(pBmp, surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);
					sampleMethod = SAMPLE_NN;
				}
				else if (zoom >= 1)
				{
					// 平滑近邻重采样
					bSampled = pBmp->Sample_Single(surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);
					sampleMethod = SAMPLE_SINGLE;
				}
				else if (zoom > 0.5
					|| pBmp->width > MAXSAMPLE_WIDTH_UPPERLIMIT || pBmp->height > MAXSAMPLE_HEIGHT_UPPERLIMIT)
				{
#ifdef SAMPLE_TIMETEST
					LARGE_INTEGER f, s, e;
					QueryPerformanceFrequency(&f);
					QueryPerformanceCounter(&s);
#endif // SAMPLE_TIMETEST
					// 双线性重采样
					bSampled = Sample_BiLinear(pBmp, surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);

					//bSampled = pBmp->Sample_Area(surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);
					//bSampled = pBmp->Sample_Single(surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);
					//bSampled = Sample_NN(pBmp, surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);
					
#ifdef SAMPLE_TIMETEST
					QueryPerformanceCounter(&e);
					if (zoom < 1)
					{
						time = (e.QuadPart - s.QuadPart) / (double)f.QuadPart;
						if (timeAvg == 0)
						{
							timeMin = time;
							timeMax = time;
							timeAvg = time;
						}
						else
						{
							if (time < timeMin)
								timeMin = time;
							if (time > timeMax)
								timeMax = time;
							if (timeMax > 1.3 * time)// timeMax纠正
								timeMax = time;
							timeAvg = timeAvg*0.95 + time*0.05;
						}
						count++;
					}
#endif // SAMPLE_TIMETEST
					sampleMethod = SAMPLE_BILINEAR;
				}
				else
				{
					// 区域重采样
					bSampled = pBmp->Sample_Area(surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);
					sampleMethod = SAMPLE_AREA;
				}
			}
		}
		surf->UnlockRect();// 解锁 surface

		if (bSampled) // 采样失败成功
		{
			// 设置相关标志
			bRenewed = true;
			bSurfFailed = false;
			lastSurfSize = surfSize;// 更新已更新过的 surface 尺寸

			CalcMapInfo();// 更新 surface 拷贝到 backbuffer 的参数

			// surface 更新耗时
			QueryPerformanceCounter(&end);
			renewTime = (float)(end.QuadPart - start.QuadPart) / freq.QuadPart;
			
		}
		else // 采样失败
		{
			SAFE_RELEASE(surf);
			bSurfFailed = true;
			return false;
		}
	}

	//if (bRenewed)
	//	bNeedRenew = false;

	return bRenewed;
}

inline HRESULT Surfer::Render() const
{
	if (surf)
	{
		if (!bOutClient && bHasDevice)
		{
			HRESULT hr;
			// surface 存在且 surface 与窗口客户区有交集才渲染
			// 获得 backbuffer
			LPDIRECT3DSURFACE9 backbuffer = NULL;
			hr = pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
			if (FAILED(hr))
				return hr;

			// 拷贝到 backbuffer
			hr = pDevice->UpdateSurface(surf, &rcSurfSrc, backbuffer, &ptSurfDest);
			if (FAILED(hr))
				return hr;

			// 释放 backbuffer
			SAFE_RELEASE(backbuffer);

			return S_OK;
		}
	}
	else
		return E_FAIL;
}