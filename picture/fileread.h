#pragma once

#include "generalgeo.h"
#include "picfile.h"
#include <atlimage.h>
#include <fstream>

using std::ifstream;
using std::ofstream;
using std::ios;
using std::ios_base;

#define PICTYPE_UNKNOWN				0
#define PICTYPE_BMP					1
#define PICTYPE_PNG					2
#define PICTYPE_JPG					3
#define PICTYPE_GIF					4
#define PICTYPE_TIFF				5

#define ISPICIDENTIFIER_BMP(ID64)	(( ID64 & 0xFFFF ) == (WORD)0x4D42 )
#define ISPICIDENTIFIER_PNG(ID64)	(( ID64 & 0xFFFF ) == (WORD)0x5089\
									&& ((ID64 >> 16) & 0xFFFF) == (WORD)0x474E\
									&& ((ID64 >> 32) & 0xFFFF) == (WORD)0x0A0D\
									&& ((ID64 >> 48) & 0xFFFF) == (WORD)0x0A1A)
#define ISPICIDENTIFIER_JPG(ID64)	(( ID64 & 0xFFFF ) == (WORD)0xD8FF\
									&& ((ID64 >> 16) & (WORD)0xF0FF) == (WORD)0xE0FF)
#define ISPICIDENTIFIER_GIF(ID64)	(( ID64 & 0xFFFF ) == (WORD)0x4947\
									&& ((ID64 >> 16) & 0xFFFF) == (WORD)0x3846\
									&& (((ID64 >> 32) & 0xFFFF) == (WORD)0x6137\
										|| ((ID64 >> 32) & 0xFFFF) == (WORD)0x6139))

#define SAFE_CLOSEFSTREAM(FS)		{ if((FS).is_open()) (FS).close(); }

#define TRANSLATION_CIMAGE(I, DX, DY)		(I).BitBlt((I).GetDC(), 0, 0, (I).GetWidth() + (DX), (I).GetHeight() + (DY), -(DX), -(DY), SRCCOPY);//平移
#define TRANSLATION_PDC(PDC, W, H, DX, DY)	(PDC)->BitBlt(0, 0, (W) + (DX), (H) + (DY), (PDC), -(DX), -(DY), SRCCOPY);//平移


#ifdef USE_GDIGUI

//对带alpha通道的图片进行像素值转换，用于CImage显示
void CrossImage(CImage &img);

struct memPic {
	PicInfo info;
	//#1 第一种存储方案
	memDCBMP *draft;

	//#2 第二种存储方案
	CImage m_Image;

	memPic() {
		memset(&info, 0, sizeof(PicInfo));
		draft = NULL;
	}
	~memPic() {
		DeleteMemDCBMP(draft);
		m_Image.Destroy();
	}

	//图片缩放到设定尺寸
	void SetSize(int width, int height);

	bool Read_1(WCHAR *filename, CDC *pDC);
	bool Read(WCHAR *filename);
	bool Show_1(CDC *pDC, int x = 0, int y = 0, BYTE alpha = 255);
	bool Show(CDC *pDC, int x = 0, int y = 0, BYTE alpha = 255, bool redrawback = false, RECT srcrect = { 0,0,0,0 });

	bool LoadImg(WCHAR *file, int w = 0, int h = 0);
};

class PicShow {
private:
	CDC *showpDC;		//目标DC
	memDCBMP *mDC;

	memPic *piclist;	//存储图片列表[从1下标开始存储]
	CPoint base;		//图片在DC中的位置（左上角）

	memPic *back;		//背景图
	POINT backbias;		//背景图偏移
	RECT backrect;		//背景图矩形
	RECT interrect;		//背景图与图片交集

	int listlen;		//图片列表长度
	int piccounts;		//实际存储图片数

	int livepic;		//当前活跃图片下标
	byte livealpha;		//当前活跃图片显示的alpha

public:
	int width, height;	//基本尺寸
	bool onenter;

	PicShow();
	PicShow(int listlen, CPoint b = { 0, 0 });

	memPic *GetOne(int index);
	bool SetBack(PicShow *ps2, int index);

	void SetBase(int x, int y);
	void SetDC(CDC *pDC);

	void SetList(int listlen);//设置图片列表长度，将清空已有图片

	bool AddPic(WCHAR *filename);

	bool Show(int picnum = 1, BYTE alpha = 255, bool redrawback = false);

	void OnPaint();
};


#endif // USE_GDIGUI

class File {
private:
	ifstream fin;
	ofstream fout;

	WCHAR name[128];

	WCHAR dir[MAX_PATH];
	WCHAR suffix[32];
	WCHAR type[6];
public:
	File() { dir[0] = L'\0'; suffix[0] = L'\0'; type[0] = L'\0'; }
	~File() {
		Close();
	}

	void SetDir(WCHAR *);
	void SetSuffix(WCHAR *);
	void SetType(WCHAR *);

	bool Open(WCHAR *file, ios_base::openmode _Mode = ios_base::in);
	void Close();
	inline bool Out(char *txt)
	{
		if (!fout.is_open() || !txt)
			return false;

		//fout.write(txt, strlen(txt));
		fout << txt;

		return true;
	}
};