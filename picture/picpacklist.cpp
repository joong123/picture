#include "stdafx.h"
#include "picpacklist.h"

/*
* 图片组
*/
//_finddata_t filegroup;
bool bPicOn;
PicPackList picList;					// 图片列表
PicPack *pLastPicpack;
PicPack *pLivePicpack;					// 主图片组
BMP *pLiveBmp;							// 当前图片 linked with "PicPack"。加速运算
WCHAR picInfoStr[256];					// 图片信息缓存，（存储基本信息:更新少，访问多，所以使用缓存）

void UpdateLocalPicStr()
{
	if (pLivePicpack)
		StringCchCopy(picInfoStr, 256, pLivePicpack->GetPicInfoStr());// 图片信息字符串更新
	else
		picInfoStr[0] = L'\0';
}

void PostPicEvent()
{
	// 更新本地指针
	pLivePicpack = picList.pLivePicPack;
	pLiveBmp = picList.pLiveBMP;

	UpdateLocalPicStr();// 更新本地信息字符串

	bPicOn = HasPic();// 更新标志
}

void InitNonPic()
{
	pLastPicpack = NULL;
	pLivePicpack = NULL;
	pLiveBmp = NULL;

	StringCchPrintf(picInfoStr, 256, L"");

	bPicOn = HasPic();
}

bool SetPic(short picidx)
{
	pLastPicpack = pLivePicpack;
	picList.SetPicPack(picidx);

	PostPicEvent();

	return true;
}

bool SetNewPic()
{
	pLastPicpack = pLivePicpack;
	picList.SetTailPicPack();

	PostPicEvent();

	return true;
}

bool SetTailPic()
{
	pLastPicpack = pLivePicpack;
	picList.SetTailPicPack();

	PostPicEvent();

	return true;
}

bool SetPrevPic()
{
	pLastPicpack = pLivePicpack;
	picList.SetPrev();

	PostPicEvent();

	return true;
}

BMP * GetPrevPic()
{
	PicPack *pPre = picList.GetPrev();
	if (pPre != NULL)
		return pPre->GetPBMP();
	else
		return NULL;
}

bool SetNextPic()
{
	pLastPicpack = pLivePicpack;
	picList.SetNext();

	PostPicEvent();

	return true;
}

void Drop()
{
	pLastPicpack = NULL;
	picList.Drop();

	PostPicEvent();
}

inline bool HasPic()
{
	if (pLiveBmp)
		if (pLiveBmp->isNotEmpty())
			return true;
	return false;
}