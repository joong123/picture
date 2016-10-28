#pragma once
#include <fstream>

#include <d3d9.h>
#include <d3dx9tex.h>

#include "BMP.h"
#include "windows.h"

using std::ifstream;
using std::ios;

struct PicInfo {
	/*UINT64 width;
	UINT64 height;*/
	UINT64 bytecount;

	PicInfo() { bytecount = 0; }

	bool GetFile(WCHAR file[]);
};

struct PicPack {
	PicInfo info0;
	D3DXIMAGE_INFO info;
	BMP pic;
	PicPack() {
		memset(&info0, 0, sizeof(info0));
		memset(&info, 0, sizeof(info));
	}
	bool Read(LPDIRECT3DDEVICE9 dev, WCHAR file[]);
};