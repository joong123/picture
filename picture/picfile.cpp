#include "stdafx.h"
#include "picfile.h"

bool PicInfo::ReadFile(WCHAR file[])
{
	if (!file)
		return false;

	ifstream in(file, ios::_Nocreate | ios::binary | 0x02);
	if (!in.is_open())
		return false;

	in.unsetf(ios::skipws);

	memset(this, 0, sizeof(*this));//清空
	width = height = 0;

	in.seekg(0, ios::end);
	bytecount = in.tellg();

	in.seekg(0, ios::beg);
	byte identifier[8] = { 0 };
	in.read((char*)identifier, 8);

	if (*(WORD*)identifier == (WORD)0x4D42)
	{
		//BMP
		//-width, height
		pictype = PICTYPE_BMP;

		in.seekg(18, ios::beg);
		in.read((char*)&(width), 4);
		in.read((char*)&(height), 4);

		in.seekg(2, ios::cur);
		in.read((char*)&(depthinfile), 2);
		//像素深度
		generaldepth = depthinfile;

		//计算通道数
		switch (depthinfile)
		{
		case 1:
			channels = 1;
			break;
		case 4:
		case 8:
		case 16:
		case 24:
		case 32:
			channels = 3;
			break;
		}
	}
	else if (((WORD*)identifier)[0] == (WORD)0x5089
		&& ((WORD*)identifier)[1] == (WORD)0x474E
		&& ((WORD*)identifier)[2] == (WORD)0x0A0D
		&& ((WORD*)identifier)[3] == (WORD)0x0A1A)
	{
		//PNG-BIGENDIAN
		//-width, height, depth, colortype
		pictype = PICTYPE_PNG;

		in.seekg(16, ios::beg);
		in.read((char*)&(width), 4);
		DWORD_L2BEndian((byte*)&(width));

		in.read((char*)&(height), 4);
		DWORD_L2BEndian((byte*)&(height));

		in.read((char*)&(depthinfile), 1);
		in.read((char*)&(colortype), 1);

		//计算通道数
		switch (colortype)
		{
		case 0:
			channels = 1;
			break;
		case 2:
		case 3:
			channels = 3;
			break;

		case 4:
		case 6:
			channels = 4;
			break;
		}
		//像素深度
		generaldepth = depthinfile*channels;
	}
	else if (((WORD*)identifier)[0] == (WORD)0xD8FF
		&& (((WORD*)identifier)[1] & (WORD)0xE0FF) == (WORD)0xE0FF)
	{
		//JPG
		//-width, height
		pictype = PICTYPE_JPG;

		//寻找标签
		long long FFC0pos = -1;
		int poscount = 0;
		in.seekg(0, ios::beg);
		//FFC0pos = in.tellg();
		while (true)
		{
			poscount++;
			if (in.eof())
			{
				in.clear();
				break;
			}

			byte f, r;
			in >> f;
			if (f == 0xFF)//搜索0xFFCX
			{
				in >> r;
				if (r == 0xC0 || r==0xC1 || r == 0xC2)
					FFC0pos = in.tellg();
				else if (r == 0xFF)
					in.seekg(-1, ios::cur);
			}
		}
		if (FFC0pos == -1)
			return false;
		else
		{
			in.seekg(FFC0pos, ios::beg);
			in.seekg(2, ios::cur);
			in.read((char*)&depthinfile, 1);

			in.read((char*)&height, 2);
			WORD_L2BEndian((byte*)&height);

			in.read((char*)&width, 2);
			WORD_L2BEndian((byte*)&width);

			in.read((char*)&channels, 1);
			generaldepth = depthinfile*channels;
		}
	}
	else if ((((WORD*)identifier)[0] == (WORD)0x4949 || ((WORD*)identifier)[0] == (WORD)0x4D4D)
		&& ((WORD*)identifier)[1] == (WORD)0x002A
		&& identifier[4] >= (byte)0x08)
	{
		//TIFF
		pictype = PICTYPE_TIFF;
	}
	else if (((WORD*)identifier)[0] == (WORD)0x4947
		&& ((WORD*)identifier)[1] == (WORD)0x3846
		&& (((WORD*)identifier)[2] == (WORD)0x6137 || ((WORD*)identifier)[2] == (WORD)0x6139))
	{
		//GIF
		//-width, height
		pictype = PICTYPE_GIF;

		in.seekg(6, ios::beg);
		in.read((char*)&(width), 2);
		in.seekg(8, ios::beg);
		in.read((char*)&(height), 2);
	}
	else
	{
		//CANNOT IDENTIFY
		pictype = PICTYPE_UNKNOWN;

		return false;
	}
	//in.seekg(0, ios::end);
	//filelength = (UINT32)in.tellg();

	if (in.eof())
		return false;

	in.close();

	return true;
}

bool PicInfo::GetSize(WCHAR file[])
{
	return false;
}

