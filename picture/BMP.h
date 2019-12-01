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

// ͸������Ч��
#define TRANSPARENTBACK_MASK			0b1000
#define TRANSPARENTBACK_HOLLOWCOLOR		D3DCOLOR_XRGB(255, 255, 255)
#define TRANSPARENTBACK_FILLCOLOR		D3DCOLOR_XRGB(204, 204, 204)
#define TRANSPARENTBACK_HOLLOWDENSITY	255
#define TRANSPARENTBACK_FILLDENSITY		230

#define RGB2GRAY(R, G, B)				(((R)*38 + (G)*75 + (B)*15) >> 7)

// ͨ��
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

// ͼƬ���š���ת
#define ZOOM_MAX						INT64_MAX
#define ZOOM_MIN						0.0

#define ZOOMFACTOR_DRAG					0.010	// ����������
#define ZOOMFACTOR_WHEEL				0.025
#define ROTATEFACTOR_DRAG				0.008	// ����������

#define ZOOM_MAXCONTROLED				0.618	// surfaceС�ڴ��ڼ�����С�����зŴ���λ

#define NN_MIN							2.0		// ��������Ŵ���ʹ�� NN ����
#define MAXSAMPLE_WIDTH_UPPERLIMIT		20000	// ��������ߴ�ͼƬ������������ MAXSAMPLE
#define MAXSAMPLE_HEIGHT_UPPERLIMIT		MAXSAMPLE_WIDTH_UPPERLIMIT

#define MIN_NORMALRENEWSIZE				300		// ͼƬС������ߴ磬�ڼ���ʱ������ʹ�� SingleSample ( ̫�����Ź��̿����п��� )

#define MIN_FORCECLIPSIZE				1500	// ͼƬ��������ߴ磬������ bClip�����������ٶ�

#define MAX_NOCLIPSIZE_DRAG				3000	// ͼƬС������ߴ磬�϶�ͼƬʱȡ�� bClip�������϶��ٶ�

// ͼƬ�б�
#define PICPACKLIST_SIZE_INIT			3
#define PICPACKLIST_SIZE_INCREMENT		2

// һЩ Surfer ����
#define PICMAXSCALE_INIT				0.8

// ���λ��
#define CURSORPOS_PIC					0
#define CURSORPOS_BLANK					1
#define CURSORPOS_OUTWINDOW				2

// ��ɫ
#define BACKCOLOR_INIT					0xFF888888
#ifndef SETALPHA
#define SETALPHA(C, A)					((C & 0xFFFFFF) | (A << 24))
#endif

using std::string;
using std::bad_alloc;
using std::endl;
using std::cerr;

// ��ɫ���ͣ�������ɫ��ȷ����
typedef float	COLORTYPE;
// AUXTYPEΪ�ز��� ( ���ֵ ) �㷨��Ӱ�켫��״̬���ȵĹؼ�����
//  ����Ϊ float �Ļ������������ţ�Area��Single �㷨����ɫ�����н��������
typedef double	AUXTYPE;
// ZOOMTYPEΪ�ز��� ( ���ֵ ) �㷨��Ӱ�켫��״̬����λ�þ��ȵĹؼ�����
//  ����Ϊ float ʱ�����������ţ����ر߽�λ��������ƫ��
typedef double	ZOOMTYPE;

struct COLOR4f {
	COLORTYPE a;
	COLORTYPE r;
	COLORTYPE g;
	COLORTYPE b;

	COLORTYPE w;// Ȩ�أ��ۼӣ�

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
	// �Ե�һ���� 2 �֣�destcolor ( ֻ���ǻҶȣ�������������ɫ ) �������� srccolor �� alpha �� alphablend ��ѯ��
	static DWORD lastcolor1, lastresult1;
	static DWORD lastcolor2, lastresult2;
	static byte **alphablendtable1, **alphablendtable2;

	// ������ destcolor������ srccolor ���� alphablend �Ĳ�ѯ��
	static byte ***alphablendcube1;

	ALPHABLEND() {}

	// ����ʾ��
	/*static inline DWORD AlphaBlend1(COLOR4f src) {
		byte alpha = roundf2b(src.a);
		DWORD result = 0;
		result += (DWORD)alphablendtable1[(byte)(src.r)][alpha] << 16;
		result += (DWORD)alphablendtable1[(byte)(src.g)][alpha] << 8;
		result += (DWORD)alphablendtable1[(byte)(src.b)][alpha];

		return result;
	}*/
	// ����ʾ��
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

// ��������
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
	bool bAlpha;		//�Ƿ���͸��ͨ��

	BMP();
	~BMP();

	// ����
	void SetAlpha(bool alpha);
	bool Load(LPDIRECT3DSURFACE9 surf);			//�� surface ���ɶ�Ӧ��СͼƬ
	bool UpLoad(LPDIRECT3DSURFACE9 surf);		//������ surface���� {0, 0} ���뿪ʼ�������ܶ�Ŀ���

	/*
	 * �����Ѷ��Ƶ�surface�Ŀ���
	 */
	/***************************************************
	surfData: Ŀ���ַ�ռ䣬surfSize: Ŀ��ߴ�
	ptSurfOffset: surface ���ԭͼƫ�ƣ�����ԭͼѡ����

	˫���Բ�ֵ�����ֲ��� ( ��ɫ���ɫ�� ) �� Sample_Single ����ϸ����
	****************************************************/
	// �������ʧ���� ( ��СЧ���Ϻã���С�ٶ����� )
	bool MYCALL1 Sample_Area(DWORD *surfData, const POINT &surfSize
		, ZOOMTYPE zoom, const POINTi64 &ptSurfOffset = POINTi64(0,0));
	// NN �� bilinear ��ϣ��罻�紦����ʹ�û��
	bool MYCALL1 Sample_Single(DWORD *surfData, const POINT &surfSize
		, ZOOMTYPE zoom, const POINTi64 &ptSurfOffset = POINTi64(0, 0));

	/*
	 * ͼƬ����
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

	// ��Ϣ
	inline bool MYCALL1 GetPixel(int x, int y, DWORD *dest)
	{
		if (data == NULL || data[y] == NULL || dest == NULL)
			return false;

		if (x < 0 || y < 0 || x >= width || y >= height)
			return false;

		*dest = ((DWORD*)data[y])[x];

		return true;
	}

	// ״̬
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

// ����ڽ� ( �Ŵ���С�����ԣ�Ч������̫�� )
bool MYCALL1 Sample_NN(BMP *pBmp, DWORD *surfData, const POINT &surfSize
	, ZOOMTYPE zoom, POINTi64 &ptSurfOffset = POINTi64(0, 0));
bool MYCALL1 Sample_NNRotate(BMP *pBmp, DWORD *surfData, const POINT &surfSize
	, ZOOMTYPE zoom, float rotate = 0, POINTi64 &ptSurfOffset = POINTi64(0, 0));
// ˫���� ( ��С�����Ŵ����Ч���� )
bool MYCALL1 Sample_BiLinear(BMP *pBmp, DWORD *surfData, const POINT &surfSize
	, ZOOMTYPE zoom, const POINTi64 &ptSurfOffset = POINTi64(0, 0));

string GetFMTStr(D3DFORMAT);

// surfer ������������ͼƬ����Ϣ
//  surfer �л�ͼƬ���� PicPack ��������Щ surfer ���õ�����Ϣ
struct PIC_LIVEINFO {
	POINTi64 src;
	double zoom;
	float rotate;
};

class PicPack {
private:
	BMP *pBmp;
	PicInfo myPicInfo;				// �Զ�ͼƬ��Ϣ
	D3DXIMAGE_INFO D3DPicInfo;		// ͼƬ��Ϣ
	WCHAR strFileName[MAX_PATH];	// �ļ���

	bool bSaved;
	bool bHasDirectory;

	// �� surface �������Ϣ
	PIC_LIVEINFO liveInfo;

	// ��Ϣ
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
	BMP *pBmp;								// BMP ָ�롤����
	UINT *pBufferW, *pBufferH;				// Ŀ�� surface �ߴ硤����TODO: UINT Խ�紦������ LONG
	bool bHasPic;							// �Ƿ���ͼƬ

	// "BMP" ӳ�� "surface" ����
	double			zoom;					// �Ŵ���
	double			lastZoom;				// ��һ���Ŵ���
	float			rotate;					// ��ת��
	float			lastRot;				// ��һ����ת��
	// ӳ��״̬
	INT64			zoomW, zoomH;			// �Ŵ� zoom ���� surface �ߴ磨�� clip �����������������ʱ����������Ҫ�ֶ����¡�(��Ҫ�ֶ����³��ϣ����ţ�����ͼƬ)
	double			zoomXActual, zoomYActual;// ʵ�� x,y �����ϵķŴ�������ʱֻ�����鿴��

	// "Դ surface" ӳ�� "Ŀ�� surface" ����
	// TODO: ptSurfBase �� INT64�������� LONG_MAX ʱ���������ݵ�Ӱ�죬���磺surfSize��ptSurfDest��rcSurfSrc
	POINTi64		ptSurfBase;				// �Ŵ� zoom ���ļ��� surface ��Դ��ڿͻ�����λ�ã����Ͻǣ�
	// ӳ��״̬
	POINTi64		ptSurfOffset;			// clip ��� surface ����ڼ��� surface ��ƫ��
	POINT			surfSize;				// surface �ߴ�
	POINT			lastSurfSize;			// �洢���һ�� surfRenew ʱ�ĳߴ�

	// ���ܲ���
	POINT			ptZoomBase;				// ���� dragzoom �����ĵ�
	double			ptScaleX;				// ptZoomBase ���ͼƬ�ı���ֵ
	double			ptScaleY;

	// ��ʱ
	LARGE_INTEGER wheeltick, lastwheeltick;	// ���ּ�ʱ
	LARGE_INTEGER freq;						// Ƶ��

protected:
	void			CalcBindState();	// ��������״̬ ( ��ͼƬ����״̬ )

	void			GetZoomSize(double z, double r, INT64 *pZoomW, INT64 *pZoomH);// ���㼴ʱ���� surface �ߴ�
	// ������ز����ĸ��£�surface ����ǰ׼��
	// ptSurfBase �� zoomWH Ψһȷ������ surface �����ʣ�λ�úʹ�С��
	// actualZoomXY��ptSurfOffset �������������ڸ��� surface ���ɣ�����Ϣ�鿴�����ݻ��������
	// �� 1 �㣺zoom��rotate��clip �ı������ zoomWH��actualZoomXY��ptSurfBase �ı�
	inline void		OnZoomChange();// zoom ���º���Ҫ����ִ�е�
	void			PostZoomChange(bool bPosRefresh = false, bool stable = true);
	void			PostRotateChange(bool bPosRefresh = false);
	// �� 2 �㣺ptSurfOffset��bPicClipped��bOutClient��surfSize ��������
	inline void		CalcSurfSize();// ���� surface �ߴ�
	inline void		OnSurfPosChange(); // pos �仯�����ִ��

public:
	// "BMP" ӳ�� "surface" ѡ��
	bool			bClip;					// surface �Ƿ� clip ( ��ȥ�ͻ��������� )

	// ӳ��״̬
	bool			bPicClipped;			// ��ǰͼƬ�Ƿ� clip ( bPicClipped �� bOutClient ֻ���ض������м��㣬��Ҫȷ�������� )
	bool			bOutClient;				// surface �ʹ��ڿͻ����Ƿ��޽��� ( �ڿͻ����� )

	SurferBase();
	void			ClearPic();
	void			Clear();

	// ��Ϣ��ȡ
	inline POINTi64	GetBase() const;
	inline double	GetZoom() const;
	inline float	GetRotate() const;
	inline INT64	GetZoomWidth() const;
	inline INT64	GetZoomHeight() const;

	// �����������׶� ( ƽ�ơ����š���ת��clip ����)
	void			SetClip(bool clip);
	void			SetBasePoint(POINT ptB);
	void			SurfHomePR();		// surface ��ԭ��
	void			SurfLocatePR(POINTi64 base);
	void			SurfCenterPR(int bufferW, int bufferH);
	void			SurfMovePR(INT64 dx, INT64 dy);
	void			SurfSetZoomPR(double zoom, bool bPosRefresh = false, bool stable = true);// ���� surface ���ű���
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
		POINTi64 size;// TODO:תΪ surfSize �᲻����������
		SETPOINT(size, zoomW, zoomH);
		if (bClip)
		{
			// ���м��㣬ԭ��ûʹ�� min��max��ֻʹ�� min��max���һ���������Ǽ���������Ϊ����
			if (ptSurfBase.x < 0)// surface ��� clip
				size.x += max(ptSurfBase.x, -zoomW);
			if (ptSurfBase.y < 0)// surface �ϲ� clip
				size.y += max(ptSurfBase.y, -zoomH);
			if (ptSurfBase.x + zoomW > (INT64)*pBufferW)// surface �Ҳ� clip
				size.x -= min(ptSurfBase.x + zoomW - (INT64)*pBufferW, size.x);
			if (ptSurfBase.y + zoomH > (INT64)*pBufferH)// surface �²� clip
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

			// rotate ���ȡ ceil����Ϊ rotate �������ؼ���ֱ���� zoom����û�þ�ȷ����ֵ
			//  �� ceil ���ܵ���ĳЩ���ͼƬ��ʾ����ȫ���� 24*22 ͼƬ�Ŵ� 70.84 ����ת 14'40'3.79"
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
	
	// ���� ptSurfOffset
	ptSurfOffset = { 0, 0 };
	if (bClip)
	{
		if (ptSurfBase.x < 0)// surface ����� clip
			ptSurfOffset.x = -ptSurfBase.x;
		if (ptSurfBase.y < 0)// surface �ϲ��� clip
			ptSurfOffset.y = -ptSurfBase.y;
	}

	if (pBufferW != NULL && pBufferH != NULL)
	{
		// ���� picture clip��OutClient ״̬
		bPicClipped = (
			ptSurfBase.x < 0 || ptSurfBase.y < 0 ||			// surface �����ϲ��� clip
			ptSurfBase.x + zoomW > (INT64)*pBufferW || 		// surface �Ҳ��� clip
			ptSurfBase.y + zoomH > (INT64)*pBufferH			// surface �²��� clip
			);
		bOutClient = (
			ptSurfBase.x < - zoomW || ptSurfBase.y < -zoomH ||// ���򳬳� surface �һ���
			ptSurfBase.x >= (INT64)*pBufferW || 			// �������²��� surface �����
			ptSurfBase.y >= (INT64)*pBufferH
			);

		// ���������ɵ� surface �ߴ�
		CalcSurfSize();
	}
}

inline void	SurferBase::OnZoomChange()
{
	//bNeedRenew = true;

	if (bHasPic)
	{
		// ���¼��� surface �ߴ�
		GetZoomSize(zoom, rotate, &zoomW, &zoomH);

		// ���¾�ȷ����ֵ
		zoomXActual = (double)zoomW / pBmp->width;
		zoomYActual = (double)zoomH / pBmp->height;
	}
}


class Surfer :public SurferBase {
protected:
	LPDIRECT3DSURFACE9 surf;				// surface�����ɵ�ͼƬ
	LPDIRECT3DDEVICE9 pDevice;				// �豸ָ�롤����
	bool bHasDevice, bHasBoth;				// �Ƿ�߱�����ֵ ( ������� )

	/******************************************************************************
	ptSurfBase��zoom��rotate �߼��Ͽɷ��룬��Ϊ���� BMP ����Ϣ��
	*******************************************************************************/
	// "Դ surface" ӳ�� "Ŀ�� surface" ֱ�Ӳ���
	POINT			ptSurfDest;				// surface ������ backbuffer, backbuffer ����ʼ��
	RECT			rcSurfSrc;				// surface Ҫ������ backbuffer ������
	// "BMP" ӳ�� "surface" ״̬
	bool			bSurfFailed;			// surface �Ƿ񴴽�ʧ�ܡ���ʱֻ�����鿴��
	bool			bSurfClipped;			// ��ǰ surface �Ƿ� clip

	// �����Ϣ
	POINT			ptCursorPixel;			// ��굱ǰλ�ö�Ӧԭͼ������
	DWORD			cursorColor;			// ��굱ǰλ�ö�Ӧԭͼ��������ɫ
	UINT8			cursorPos;				// ��ǰ���λ�� �� �����ĸ����� ͼƬ���հס������� ��

	// ��ʱ (����) ����
	double time;							// ������ʱ
	double timeAvg;							// ������ʱ
	double timeMin;							// ������ʱ
	double timeMax;							// ������ʱ
	int count;
	//int surfrenewcount;					// surface ���¼���
	float renewTime;						// surface ���º�ʱ

	// ��Ϣ
	SAMPLE_METHOD sampleMethod;				// ��������
	WCHAR strSurfInfo[512];					// ��Ϣ�ַ���

protected:
	void			CalcBindState();		// ���������־ ( �����״̬ )

	// �� 3 �㣺ptSurfDest��rcSurfSrc �������� ( ǰ����� SurferBase )
	inline void		CalcMapInfo();			// ��bHasDevice������ӳ�䵽Ŀ�� surface ����
	// �� 4 �㣺bSurfFailed��bSurfClipped ��������
	//  �� SurfRenew() ����

public:
	Surfer();
	~Surfer();

// ��������
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

// �豸
	bool			BindDevice(LPDIRECT3DDEVICE9 dev);	// �����豸
	bool			BindBuf(UINT * pBufW, UINT *pBufH);	// ����Ŀ�� surface �ߴ�

// ͼƬ
	bool			BindPic(PicPack *pPack, bool renew = true);// ����ͼƬ ( �������� NULL ������������/�л�ͼƬ )
	bool			DeBindPic(PicPack *pPack);			// ���ͼƬ
	void			ClearPic();							// ������ͷ���ͼƬ������

	void			SetBasePoint(POINT ptB);			// ���� ( ���� ) ��׼��

// surface ˢ��
	inline bool		SurfRenew(bool acce = false);		// ��bHasBoth��surface ˢ�£����� surface �Ƿ�ˢ��
	inline HRESULT	Render() const;						// ��surf��bHasDevice����Ⱦ

// surface ���Ʋ���
	// ��bHasBoth���϶��¼��������� surface �Ƿ�ˢ��
	bool			OnDrag_Custom(POINTi64 offSet, bool bAcce = true);
	// ��bHasBoth�����������¼��������� surface �Ƿ�ˢ��
	bool			OnWinsize_Custom();					// �� OnDrag_Custom ���
	bool			SurfSuit(int w, int h);

// ��Ϣ
	void			UpdateInfo();						// ������Ϣ�ַ���
	// ��bHasBoth��������Ϣ��Ҫ�����µ�ǰ��Ϣ ( ���ݵ�ǰ���λ�ö�ӦͼƬ������ ) ����������Ϣ�ַ���
	void			GetCurInfo(POINT *cursor);
	inline const WCHAR *GetInfoStr() const				// �����Ϣ�ַ���
	{
		return strSurfInfo;
	}
};

#define SAMPLE_TIMETEST

inline bool Surfer::SurfRenew(bool acce)
{
	bool bRenewed = false;// surface �Ƿ�ɹ�����
	if (bHasBoth)
	{
		LARGE_INTEGER start, end;// surface ���¼�ʱ
		QueryPerformanceCounter(&start);

		// ���� bSurfClipped	��ʵ�� surface �Ƿ� clip
		//  ���� SurfRenew() �ж����� OnSurfPosChange() �У�
		//  ��Ϊ��ʹ surface ��ز������¸��� surface �ĸ���
		bSurfClipped = (bClip && bPicClipped);

		// ���� surface
		//  ���ԭ surface Ϊ�գ����� surface �ߴ����
		if (surf == NULL || lastSurfSize.x != surfSize.x || lastSurfSize.y != surfSize.y)
		{
			// ���ԭ surface
			SAFE_RELEASE(surf);
			// �½� surface
			if (FAILED(pDevice->CreateOffscreenPlainSurface(
				surfSize.x, surfSize.y
				, D3DFMT_A8R8G8B8
				, D3DPOOL_SYSTEMMEM// D3DPOOL_DEFAULT �Ļ�������ʾ
				, &surf
				, NULL)))
			{
				SAFE_RELEASE(surf);
				bSurfFailed = true;
				return false;
			}
		}

		// ���� surface ����
		D3DLOCKED_RECT lockedRect;
		if (FAILED(surf->LockRect(&lockedRect, NULL, NULL)))
		{
			SAFE_RELEASE(surf);
			bSurfFailed = true;
			return false;
		}
		DWORD *surfData = (DWORD*)lockedRect.pBits;

		// �������ֳɼ��ٺ���������ģʽ������ӳ����Ե�ѡ��
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
					bSampled = pBmp->Sample_Single(surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);
					sampleMethod = SAMPLE_SINGLE;
				}
			}
			else
			{
				if (zoom > NN_MIN)
				{
					// �����ز���
					bSampled = pBmp->Sample_Single(surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);
					sampleMethod = SAMPLE_SINGLE;
				}
				else if (zoom >= 1)
				{
					// ƽ�������ز���
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
					// ˫�����ز���
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
							if (timeMax > 1.3 * time)// timeMax����
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
					// �����ز���
					bSampled = pBmp->Sample_Area(surfData, surfSize, (ZOOMTYPE)zoom, ptSurfOffset);
					sampleMethod = SAMPLE_AREA;
				}
			}
		}
		surf->UnlockRect();// ���� surface

		if (bSampled) // ����ʧ�ܳɹ�
		{
			// ������ر�־
			bRenewed = true;
			bSurfFailed = false;
			lastSurfSize = surfSize;// �����Ѹ��¹��� surface �ߴ�

			CalcMapInfo();// ���� surface ������ backbuffer �Ĳ���

			// surface ���º�ʱ
			QueryPerformanceCounter(&end);
			renewTime = (float)(end.QuadPart - start.QuadPart) / freq.QuadPart;
			
		}
		else // ����ʧ��
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
			// surface ������ surface �봰�ڿͻ����н�������Ⱦ
			// ��� backbuffer
			LPDIRECT3DSURFACE9 backbuffer = NULL;
			hr = pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
			if (FAILED(hr))
				return hr;

			// ������ backbuffer
			hr = pDevice->UpdateSurface(surf, &rcSurfSrc, backbuffer, &ptSurfDest);
			if (FAILED(hr))
				return hr;

			// �ͷ� backbuffer
			SAFE_RELEASE(backbuffer);

			return S_OK;
		}
	}
	else
		return E_FAIL;
}