#pragma once

#include <fstream>
#include "windows.h"

#include <d3d9.h>
#include <d3dx9tex.h>

using std::ifstream;
using std::ios;

#define PICTYPE_UNKNOWN		0
#define PICTYPE_BMP			1
#define PICTYPE_PNG			2
#define PICTYPE_JPG			3
#define PICTYPE_GIF			4
#define PICTYPE_TIFF		5

inline void WORD_L2BEndian(byte *bp)
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
	UINT32 width;
	UINT32 height;
	UINT8 depth;//图像文件中存储的深度
	UINT8 generaldepth;
	int poscount;
	UINT8 colortype;
	UINT8 channels;
	UINT64 bytecount;
	short pictype;

	PicInfo() { width = height = 0; depth = 0; bytecount = 0; pictype = 0; }

	bool GetFile(WCHAR file[]);
	bool GetSize(WCHAR file[]);
};