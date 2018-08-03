#pragma once

#include "BMP.h"

//_finddata_t filegroup;
extern bool bPicOn;
extern PicPackList picList;				// 图片列表
extern PicPack *pLastPicpack;
extern PicPack *pLivePicpack;			// 主图片组
extern BMP *pLiveBmp;					// 当前图片 linked with "PicPack"。加速运算
extern WCHAR picInfoStr[];				// 图片信息缓存，（存储基本信息:更新少，访问多，所以使用缓存）

void UpdateLocalPicStr();				// 更新本地信息字符串
void PostPicEvent();					// 切换图片等操作后，需要进行的步骤
inline bool HasPic();					// 计算当前是否图片非空

void InitNonPic();						// 初始化无图片状态的参数
bool SetPic(short picidx = 1);			// 根据指定编号设置图片
bool SetNewPic();						// 定位到最后一张图，并居中
bool SetTailPic();						// 定位到最后一张图片
bool SetPrevPic();						// 定位到下一图片
BMP *GetPrevPic();
bool SetNextPic();						// 定位到下一图片
void Drop();							// 清除当前图片