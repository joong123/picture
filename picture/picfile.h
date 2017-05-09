#pragma once

#include <basetsd.h>
#include <fstream>

using std::ifstream;
using std::ios;

#define PICTYPE_UNKNOWN			0
#define PICTYPE_BMP				1
#define PICTYPE_PNG				2
#define PICTYPE_JPG				3
#define PICTYPE_GIF				4
#define PICTYPE_TIFF			5

typedef unsigned char byte;
typedef wchar_t WCHAR;

inline void WORD_L2BEndian(byte *bp)//WORD_L2BEndian奇特问题
{
	byte temp = bp[0];
	bp[0] = bp[1];
	bp[1] = temp;
}
inline void DWORD_L2BEndian(byte *bp)
{
	byte temp = bp[0];
	bp[0] = bp[3];
	bp[3] = temp;
	temp = bp[1];
	bp[1] = bp[2];
	bp[2] = temp;
}

struct PicInfo {
	short pictype;		//图片文件类型（需要解析）
	UINT32 width;		//图片尺寸
	UINT32 height;
	UINT8 depthinfile;	//图片文件中存储的深度
	UINT8 generaldepth;	//位深度（需要解析）
	UINT8 colortype;	//颜色类型（灰度/彩色/索引/α通道等）
	UINT8 channels;		//通道数（需要解析）
	UINT64 bytecount;	//字节数

	PicInfo() { 
		width = 0; 
		height = 0; 
		depthinfile = 0; 
		generaldepth = 0; 
		colortype = 0;
		channels = 0;
		bytecount = 0;
		pictype = PICTYPE_UNKNOWN;
	}

	bool ReadFile(WCHAR file[]);
	bool GetSize(WCHAR file[]);
};