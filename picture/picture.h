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

// �û���Ϣ
#define WM_RESETDEVICE					(WM_USER + 1)	// ���� D3D �豸��Ϣ
#define WM_SURFFORCERENEW				(WM_USER + 2)	// surface ������Ϣ
#define WM_TOGGLEFULLSCREEN				(WM_USER + 3)	// �л�ȫ����Ϣ
#define WM_MOUSEWHEELEND				(WM_USER + 4)	// FLAGDELAY_WHEEL �趨�ӳ�ʱ�䣬���ֽ����󾭹����ʱ��󣬷���һ�� "���ֽ���" �¼�
#define WM_SIZEEND						(WM_USER + 5)	// FLAGDELAY_SIZE

// ������
#define CMDRECT_ALPHA					180
#define CMDRECT_COLOR					D3DCOLOR_ARGB(CMDRECT_ALPHA, 25, 220, 255)
#define CMDRECT_COLOR2					D3DCOLOR_ARGB(CMDRECT_ALPHA, 23, 24, 36) 
#define CMDRECT_COLOR3					0xB0CFF5DB 
#define CMDRECT_COLOR_USING				CMDRECT_COLOR

// ���ڳߴ�
#define WINDOWPOSX_INIT					CW_USEDEFAULT // Ĭ��ֵ
#define WINDOWPOSY_INIT					100
#define WINDOWWIDTH_INIT				656
#define WINDOWHEIGHT_INIT				518

// ���ڿ���
#define SIZE_NEAR_PIXEL					8	// �����߿����Ͼ��루�������죩

// ��λת��
#define B_TO_MB							1048576.0f

// ʱ��
#define ICONIC_SLEEP					50
#define NORMAL_FPS						60
#define NORMAL_FRAMETIME				(1000.0f / NORMAL_FPS)
#define STATIC_FPS_NORMAL				NORMAL_FPS // ��ֹ״̬����֡��
#define STATIC_FPS_SILENT				16
#define STATIC_FPS_SLEEP				0

// ��־
#define FLAGDELAY_WHEEL					400		// ����΢���Ϊ�� zoom Ч��
#define FLAGDELAY_SIZE					1		// WM_SIZE ʱ��������ѭ����������� size ��־�����ֵ��ʾ size �������־������Ӧ����
#define FLAGDELAY_SURFREFRESH			160		// ����΢���Ϊ���ܹ����������־����
#define FLAGDELAY_DRAG					100		// ����΢���Ϊ�� drag Ч��
#define FLAGDELAY_DRAGZOOM				100		// ����΢���Ϊ�� dragzoom Ч��

// ����ģʽ
#define MODE_PIC						0
#define MODE_CMD						1		// TODO������Ϊģʽ����Ϊ״̬
//#define MODE_PICVIEWER					2	// ͼƬ���ģʽ
//#define MODE_VIDEO						3	// ��Ƶ����ģʽ

// ����ģʽ
#define WINMODE_NORMAL					0
#define WINMODE_ROUND					1
#define WINMODE_INIT					WINMODE_NORMAL

// �ı���ʾ
// ���봰�ڱ�Ե����
#define TEXTMARGIN_SIDE					3
#define TEXTMARGIN_TOP					0
#define TEXTMARGIN_BOTTOM				2
#define TEXTMARGIN_ROW					2
// ��ɫ
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
// GUI ϵͳ
#define BUTTON_ID_OPEN					1
#define BUTTON_ID_SAVE					2
#define BUTTON_ID_AID					3
#define BUTTON_ID_FULLSCREEN			11
#define INPUT_IN_CMD					21


/*
* ����������Ϣ����־
*/
extern D3DWnd *startup;
extern HWND hWndStartup;
extern bool bDrag;
extern POINT ptCurStartup;
extern LRESULT CALLBACK    StartWndProc(HWND, UINT, WPARAM, LPARAM);

extern HWND hWndAid;
extern PROCESS_INFORMATION pi;
extern STARTUPINFO si;// ����ָ���½��̵����������Ե�һ���ṹ


/*
 * ������Ϣ&��־
 */
// ����״̬
HWND hWndMain;							// �����ھ��
RECT rcWindow, rcClient;				// ���������򣬿ͻ�����
RECT rcOriginalWnd;						// ��¼ȫ��֮ǰ�����������ڴ�ȫ��״̬�ָ�
HCURSOR hCursorMain;					// �Զ�����
HCURSOR hCursorG;
POINT ptLastCursor;						// ��һ���λ�á�

// ������Ϣ
HANDLE hProcess;						// ���̾��
float memoryIn, memoryOut;				// �ڴ�ռ����

// ѡ���־
UINT8 mode;								// ����ģʽ
UINT8 winMode;							// ����ģʽ
D3DCOLOR BackgroundColor;				// ����ɫ
bool bFlagsShow;						// �Ƿ���ʾ״̬��־��Ϣ		
bool bInfoShow;							// �Ƿ���ʾ������Ϣ
bool bFpsLimited;						// �Ƿ�����֡��

// ״̬��־
bool bPureWnd;							// �Ƿ��Ǵ�����״̬ ( �ޱ����� )
bool bWindowedFullscreen;				// �Ƿ���αȫ��״̬

bool bSurfRenew;						// surface �Ƿ���ˢ�£��������־��ʱֻ�����鿴��
bool bNeedForceRenew;					// ��Ҫǿ�Ƹ��� surface ��־ ( �����ж��Ƿ��� WM_SURFFORCERENEW )
// TODO: ����ͳһ�������¼�����
bool bOnZoom;							// �Ƿ��ڷŴ�
bool bOnSize;							// �Ƿ������Ŵ���
bool bOnDrag;							// �Ƿ�ʼ���϶�ͼƬ״̬
bool bDragging;							// �Ƿ����϶�ͼƬ״̬
bool bOnDragzoom;						// �Ƿ�ʼ���϶��Ŵ�ͼƬ״̬
bool bDragzooming;						// �Ƿ����϶��Ŵ�ͼƬ״̬
bool bOnDragRotate;

// ���ñ�־ :
#define SF_SR(RETVAL)	{ bSurfRenew = RETVAL;if(bSurfRenew) surfRenewTick = GetTickCount(); }
#define SF_SRFR(RETVAL) { bSurfRenew = RETVAL;if(bSurfRenew) surfRenewTick = GetTickCount(); bNeedForceRenew |= bSurfRenew;}
#define SF_DR()			{ bDragging = true;dragTick = GetTickCount(); }
#define SF_OZ()			{ bOnZoom = true;zoomTick = GetTickCount(); }

/*
 * ��ʱ
 */
LONGLONG nLoops;						// ��ѭ������
float fps, avgFps, cvgFps;				// ֡��
ULONGLONG fpsCount;						// ����֡�ʴ��� ( ���ڼ�������֡�� cvgFps )
float frameTime;						// ֡ʱ��
float procTime;							// ����ʱ��
short staticFps; float staticFrameTime;	// ֡������
float normalFps; float normalFrameTime;
time_t zoomTick, sizeTick\
	, surfRenewTick, dragTick\
	, dragzoomTick, decorateTick;		// ��־��ʱ

/*
 * D3D
 */
D3DWnd *pD3DWnd;						// D3D ���� ��װ��
LPDIRECT3DDEVICE9 mainDevice;			// ���豸���� D3DWnd ��
UINT *pBufferW, *pBufferH;				// �豸�������ߴ磬�� D3DWnd �� backbuffer ��
//LPD3DXSPRITE  m_sprite;				// ��֪����ô��!

bool bDecorateOn;
LPD3DXMESH decorate;
D3DLIGHT9 light;
D3DMATERIAL9 material;
D3DXMATRIX matWorld;

Surfer surfer;							// ͼƬ��ʾ������ D3D �豸��ͼƬ BMP ��
/*
* ͼƬ��
*/
extern bool bPicOn;
extern PicPackList picList;				// ͼƬ�б�
extern PicPack *pLastPicpack;
extern PicPack *pLivePicpack;			// ��ͼƬ��
extern BMP *pLiveBmp;
extern WCHAR picInfoStr[256];			// ͼƬ��Ϣ����

/*
 * ��Ϣ��ʾ
 */
// ������Ϣ		@��װ����WndDev
LPD3DXFONT pFontPic;					// ��ʾ����
LPD3DXFONT pFontFlags;
LPD3DXFONT pFontPicState;
//LPCD3DFont d3dfont1;					// ���ٵ�Ч��һ������������
RECT rcPic;								// ͼƬ��Ϣ����
RECT rcSurface;							// surface ��Ϣ����
RECT rcFlag;							// ״̬��Ϣ����
RECT rcPicState;						// ͼƬ״̬��ʾ����

/*
 * ����
 */
void BeginDragWindow_Inner(POINT ptCur);// �Զ������촰��
void BeginDragWindow_Custom(POINT ptCur);// �Զ������촰��
void KeyDownProc(WPARAM wParam);		// WM_KEYDOWN ��Ϣ������
void SetWindowMode(UINT8 wmode);		// ���ô���ģʽ
inline void MaintainWindowStyle();		// ά�ִ���ģʽ ( �������δ��� )
inline void OnWinChange();				// ��ô��ںͿͻ�����С
inline void RefreshTextRect();			// ����������ʾ����
bool FullScreen_Windowed(bool tofull, bool restore = true);
bool PureWindow(bool topure);
void SetStaticFps(float sfps);
void ToggleNight();

// ���
inline short GetSizeType();	// ���ݵ�ǰ���꣬���������� ( �ı䴰�ڴ�С )
// ��ʼ��
bool Init();							// ��ʼ��
// D3D��ʼ��
bool D3DInit();							// D3D ��ʼ��
// �ļ�
void OnSave();
void OnSaveAs();
bool OnWinInitFile(LPWSTR cmdline);		// ��ʼ��ʱ����ͼƬ
void OpenFileWin();						// ���ļ�����
void SaveFileWin(const WCHAR file[] = L"");	// �����ļ�����

bool SaveFile(WCHAR file[]);			// ����ͼƬ
bool LoadFile(WCHAR file[]);			// ����ͼƬ

// ��־
inline void DelayFlag();				// �ӳٱ�־������
void ClearFlag();						// �����Ծ��־
// ״̬�л�
void BeginDragPic();					// ��ʼͼƬ�϶�
void EndDragPic();						// ����ͼƬ�϶�
void BeginDragZoomPic();				// ��ʼͼƬ�϶��Ŵ�
void EndDragZoomPic();					// ����ͼƬ�϶��Ŵ�
void BeginDragRotatePic();
void EndDragRotatePic();
// ģʽ
void EnterCMDMode();					// ����������ģʽ
void ExitCMDMode();						// �˳�������ģʽ ( �ص�����ģʽ )

/*
 * ͼƬ ( ͼƬ�б� PicPackList һϵ�в�����
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

// ���� picpack �䶯���� ��Ϣ�䶯����
void UpdateTitle();	// ���´��ڱ���
// PostPicPackChange �в��� PostPicPackInfoChange ����Ϊ picpack ģ�� PostPicEvent() ��ʵ��
void PostPicPackChange(bool bRenew = true);// picpack �л����� ( �������� picpack ģ�� PostPicEvent() ����)
void PostPicPackInfoChange();// picpack ��Ϣ�仯���� ( ����� picpack û�л�������Ϣ��Ҫ���µ����)

// ���� surface �� PicPack �İ�
void SwitchSurface(PicPack *newpicpack, PicPack *oldpicpack, bool renew = true);// ����ͼƬ֮������ surface
void UpdateSurfaceInfo();// ���� surface ��Ϣ��ͼƬ���ġ�������Ϣ���ģ�

//ͼƬ��surface�������ɣ�
void PicMove(int dx, int dy);
void PicRestore();
void PicClipWindow();
void PicDock();
void PicCenter();
void PicFit();

/*
 * D3D
 */
inline void SetRenderState();			// ������Ⱦ״̬
inline void SetView();
inline void SetLight();
inline bool InfoRender();				// ��Ⱦ������Ϣ
inline void Render();					// ��Ⱦ
// �豸����
inline bool ResetDevice();				// �����豸
inline void OnLostDevice();				// �豸��ʧ
inline bool OnResetDevice();			// �豸����
inline bool OnResetDevice2(int clientw, int clienth);	// �豸����
inline void PostResetDevice();			// �豸���ú�

// GUI ϵͳ
CD3DGUISystem *g_gui;
int fontID1;

// �ص�����
void CALLBACK CMDProc(WCHAR *wstr);		// �����лص�
void CALLBACK GUICallback(int ID, WPARAM wp, LPARAM lp = NULL);// GUI �ص�
//MMRESULT TimerID_main;
//void CALLBACK TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);// ��ʱ������