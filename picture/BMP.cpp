#include "stdafx.h"
#include "BMP.h"
#include "generalgeo.h"

DWORD ALPHABLEND::lastcolor1 = 0;
DWORD ALPHABLEND::lastresult1 = 0;
DWORD ALPHABLEND::lastcolor2 = 0;
DWORD ALPHABLEND::lastresult2 = 0;

byte **ALPHABLEND::alphablendtable1 = NULL;
byte **ALPHABLEND::alphablendtable2 = NULL;

byte ***ALPHABLEND::alphablendcube1 = NULL;


COLOR4f::COLOR4f()
{
	r = g = b = 0;
	w = 0;
}

COLOR4f::COLOR4f(COLORTYPE f)
{
	r = g = b = f;
	w = 0;
}

COLOR4f * COLOR4fNormalize(COLOR4f * c4f)
{
	if (c4f != NULL)
	{
		if (c4f->a > (COLORTYPE)255.0)
			c4f->a = (COLORTYPE)255.0;
		if (c4f->r > (COLORTYPE)255.0)
			c4f->r = (COLORTYPE)255.0;
		if (c4f->g > (COLORTYPE)255.0)
			c4f->g = (COLORTYPE)255.0;
		if (c4f->b > (COLORTYPE)255.0)
			c4f->b = (COLORTYPE)255.0;

		c4f->a = (COLORTYPE)(byte)(c4f->a + (COLORTYPE)0.5);
		c4f->r = (COLORTYPE)(byte)(c4f->r + (COLORTYPE)0.5);
		c4f->g = (COLORTYPE)(byte)(c4f->g + (COLORTYPE)0.5);
		c4f->b = (COLORTYPE)(byte)(c4f->b + (COLORTYPE)0.5);
	}
	return c4f;
}

COLOR4f * COLOR4fWNormalize(COLOR4f * c4f)
{
	if (c4f != NULL)
	{
		if (c4f->w > 0)
		{
			c4f->a /= c4f->w;
			c4f->r /= c4f->w;
			c4f->g /= c4f->w;
			c4f->b /= c4f->w;
			c4f->w = 1;
		}
	}
	return c4f;
}

COLOR4f * COLOR4fAccum(COLOR4f *c4f, DWORD dw, const COLORTYPE weight)
{
	if (c4f != NULL)
	{
		c4f->a += ((dw >> 24) & 0xFF) *weight;
		c4f->r += (((dw >> 16) & 0xFF)) *weight;
		c4f->g += ((dw >> 8) & 0xFF) *weight;
		c4f->b += (dw & 0xFF) *weight;
		c4f->w += weight;
	}
	return c4f;
}

COLOR4f *COLOR4fAssign(COLOR4f *c4f, COLORTYPE c)
{
	if (c4f != NULL)
	{
		c4f->a = c4f->r = c4f->g = c4f->b = c;
		c4f->w = 0;
	}
	return c4f;
}

DWORD COLOR4fGetDW(COLOR4f * c4f)
{
	DWORD dwColor = 0;
	if (c4f != NULL)
	{
		dwColor += (byte)(c4f->a + (COLORTYPE)0.5) << 24;
		dwColor += (byte)(c4f->r + (COLORTYPE)0.5) << 16;
		dwColor += (byte)(c4f->g + (COLORTYPE)0.5) << 8;
		dwColor += (byte)(c4f->b + (COLORTYPE)0.5);
	}

	return dwColor;
}

void ALPHABLEND::InitAlphBlendTable(byte color1, byte color2)
{
	if (alphablendtable1)
	{
		for (int i = 0; i < 256; i++)
		{
			if (alphablendtable1[i])
				delete[] alphablendtable1[i];
		}
		delete[] alphablendtable1;
	}
	if (alphablendtable2)
	{
		for (int i = 0; i < 256; i++)
		{
			if (alphablendtable2[i])
				delete[] alphablendtable2[i];
		}
		delete[] alphablendtable2;
	}

	alphablendtable1 = new byte*[256];
	alphablendtable2 = new byte*[256];
	for (int i = 0; i < 256; i++)
	{
		alphablendtable1[i] = new byte[256];
		alphablendtable2[i] = new byte[256];
		for (int j = 0; j < 256; j++)//i是源通道值，j是alpha，color1，color2是目标通道值
		{

			alphablendtable1[i][j] = (i*j + (255 - j)*color1) / 255;
			alphablendtable2[i][j] = (i*j + (255 - j)*color2) / 255;
		}
	}
	lastcolor1 = 0;
	lastresult1 = color1;
	lastcolor2 = 0;
	lastresult2 = color2;
}

void ALPHABLEND::InitAlphBlendTCube()
{
	const int alphalevels = 20;//alpha分为多少级（考虑到输入alpha是float形式）

	alphablendcube1 = new byte**[alphalevels];
	for (int i = 0; i < alphalevels; i++)//i,j:src、dest。k:alpha
	{
		alphablendcube1[i] = new byte*[256];
		for (int j = 0; j < 256; j++)
		{
			alphablendcube1[i][j] = new byte[256];

			for (int k = 0; k < 256; k++)
			{
				alphablendcube1[i][j][k]
					= (byte)((i*j + (float)(alphalevels - 1 - i)*k) / (alphalevels - 1));
			}
		}
	}
}

COLOR4f RGB2HSV_F3(DWORD c1)
{
	byte *ppix = (byte*)&c1;
	COLOR4f res = 0;
	res.a = ppix[3];

	byte r = ppix[2];
	byte g = ppix[1];
	byte b = ppix[0];
	byte max = 0;
	byte min = 1;
	if (ppix[1] > ppix[0])
	{
		max = 1;
		min = 0;
	}
	if (ppix[2] > ppix[max])
		max = 2;
	else if (ppix[2] < ppix[min])
		min = 2;
	byte minvalue = ppix[min];
	byte maxvalue = ppix[max];

	res.b = maxvalue;//v
	res.g = (maxvalue == 0 ? 0 : 1 - minvalue / (COLORTYPE)maxvalue);//s

	if (maxvalue == minvalue)
		res.r = 0;
	else
	{
		res.r = (maxvalue - r + g - minvalue + b - minvalue) / (COLORTYPE)(maxvalue - minvalue)*(COLORTYPE)60.0;
		/*if (max == 2)
		{
		result.r = ((COLORTYPE)60.0 * (g - b) / (maxvalue - minvalue));
		}
		else if (max == 1)
		{
		result.r = ((COLORTYPE)120.0 + (COLORTYPE)60.0 * (b - r) / (maxvalue - minvalue));
		}
		else
		{
		result.r = ((COLORTYPE)240.0 + (COLORTYPE)60.0 * (r - g) / (maxvalue - minvalue));
		}*/
	}
	if (g < b)
		res.r = 360 - res.r;

	return res;
}

DWORD RGB2HSV(DWORD color)
{
	byte *pSrc = (byte*)&color;
	DWORD result = 0;
	BYTE* pResult = (byte*)&result;

	byte r = pSrc[2];
	byte g = pSrc[1];
	byte b = pSrc[0];
	byte max = 0;
	byte min = 1;
	if (pSrc[1] > pSrc[0])
	{
		max = 1;
		min = 0;
	}
	if (pSrc[2] > pSrc[max])
		max = 2;
	else if (pSrc[2] < pSrc[min])
		min = 2;
	byte minvalue = pSrc[min];
	byte maxvalue = pSrc[max];

	pResult[0] = maxvalue;//v
	pResult[1] = (maxvalue == 0 ? 0 : ROUND_BYTE((COLORTYPE)255.0*(maxvalue - minvalue) / maxvalue));//s

	if (maxvalue == minvalue)
		pResult[2] = 0;
	else
	{
		if (max == 2)
		{
			pResult[2] = ROUND_BYTE((COLORTYPE)255.0 * ((COLORTYPE)000.0
				+ (COLORTYPE)60.0 * (g - b) / (maxvalue - minvalue)) / (COLORTYPE)360.0);
		}
		else if (max == 1)
		{
			pResult[2] = ROUND_BYTE((COLORTYPE)255.0 * ((COLORTYPE)120.0
				+ (COLORTYPE)60.0 * (b - r) / (maxvalue - minvalue)) / (COLORTYPE)360.0);
		}
		else
		{
			pResult[2] = ROUND_BYTE((COLORTYPE)255.0 * ((COLORTYPE)240.0
				+ (COLORTYPE)60.0 * (r - g) / (maxvalue - minvalue)) / (COLORTYPE)360.0);
		}
	}
	pResult[3] = pSrc[3];

	return result;
}

DWORD HSV2RGB_F3(COLOR4f color)
{
	DWORD result = 0;
	byte* pResult = (byte*)&result;

	COLORTYPE maxvalue = color.b;
	if (color.g == 0)
	{
		pResult[0] = ROUND_BYTE(maxvalue);
		pResult[1] = ROUND_BYTE(maxvalue);
		pResult[2] = ROUND_BYTE(maxvalue);
	}
	else
	{
		COLORTYPE ρ = color.g * color.b;
		COLORTYPE minvalue = maxvalue - ρ;
		byte nh = ROUND_BYTE(color.r);
		if (color.r > 180)
			nh = 360 - nh;
		COLORTYPE λ = nh / (COLORTYPE)60.0 * ρ;
		COLORTYPE r, g, b;
		r = 2 * ρ - λ;
		if (r < 0)
			r = 0;
		if (r > ρ)
			r = ρ;
		g = λ;
		if (g > ρ)
			g = ρ;

		b = λ - 2 * ρ;
		if (b < 0)
			b = 0;

		if (color.r > 180)
		{
			COLORTYPE temp = b;
			b = g;
			g = temp;
		}

		pResult[0] = ROUND_BYTE(b + minvalue);
		pResult[1] = ROUND_BYTE(g + minvalue);
		pResult[2] = ROUND_BYTE(r + minvalue);

	}
	pResult[3] = ROUND_BYTE(color.a);

	return result;
}

DWORD HSV2RGB(DWORD color)
{
	byte *pSrc = (byte*)&color;
	DWORD result = 0;
	byte* pResult = (byte*)&result;

	byte h = pSrc[2];
	byte s = pSrc[1];
	byte v = pSrc[0];

	byte maxvalue = v;
	if (s == 0)
	{
		pResult[0] = maxvalue;
		pResult[1] = maxvalue;
		pResult[2] = maxvalue;
	}
	else
	{
		COLORTYPE ρ = s * v / (COLORTYPE)255.0;
		COLORTYPE minvalue = maxvalue - ρ;
		byte nh = h;
		if (h >= 128)
			nh = 255 - h;
		COLORTYPE λ = nh * 6 / (COLORTYPE)255.0 * ρ;
		COLORTYPE r, g, b;
		r = 2 * ρ - λ;
		if (r < 0)
			r = 0;
		if (r > ρ)
			r = ρ;
		g = λ;
		if (g > ρ)
			g = ρ;

		b = λ - 2 * ρ;
		if (b < 0)
			b = 0;

		if (h >= 128)
		{
			COLORTYPE temp = b;
			b = g;
			g = temp;
		}

		pResult[0] = ROUND_BYTE(b + minvalue);
		pResult[1] = ROUND_BYTE(g + minvalue);
		pResult[2] = ROUND_BYTE(r + minvalue);

	}
	pResult[3] = pSrc[3];

	return result;
}

const char* GetSampleSchemaStr(SAMPLE_METHOD sm)
{
	const char sampleStr[6][16] = { "Unknown","NN","BiLinear","Single","Cube","Area" };
	return sampleStr[sm];
}

BMP::BMP()
{
	//set_new_handler(newhandlerfunc);
	width = 0;
	height = 0;

	data = NULL;
	bAlpha = false;
}

BMP::~BMP()
{
	if (data != NULL)//释放
	{
		for (int i = 0; i < height; i++)
		{
			SAFE_DELETE_LIST(data[i]);
		}
		SAFE_DELETE_LIST(data);
	}
}

void BMP::SetAlpha(bool alpha)
{
	bAlpha = alpha;
}

bool BMP::Load(LPDIRECT3DSURFACE9 surf)
{
	if (surf == NULL)
		return false;

	//surf数据
	D3DSURFACE_DESC surfaceDesc;
	surf->GetDesc(&surfaceDesc);

	//获得surf长宽
	int oldheight = height;//用于释放原图
	width = surfaceDesc.Width;
	height = surfaceDesc.Height;
	if (width <= 0 || height <= 0)
		return false;

	//清除原图
	if (data != NULL)
	{
		for (int i = 0; i < oldheight; i++)
		{
			SAFE_DELETE_LIST(data[i]);
		}
		DELETE_LIST(data);
	}

	//分配新BMP图
	try
	{
		data = new byte*[height];
		for (int i = 0; i < height; i++)
		{
			data[i] = new byte[4 * width];
		}
	}
	catch (bad_alloc &memExp)
	{
		//失败以后，要么abort要么重分配
		cerr << memExp.what() << endl;
		data = NULL;

		return false;
	}

	//锁定surface区域
	D3DLOCKED_RECT lockedRect;
	if (FAILED(surf->LockRect(&lockedRect, NULL, NULL)))
		return false;
	DWORD *surfData = (DWORD*)lockedRect.pBits;

	//拷贝
	int pitch = lockedRect.Pitch >> 2;
	for (int i = 0; i < height; i++)
	{
		int stride = i*pitch;
		DWORD *dataRow = (DWORD*)data[i];
		for (int j = 0; j < width; j++)
		{
			dataRow[j] = surfData[stride + j];
		}
	}

	//解锁surface区域
	surf->UnlockRect();

	return true;
}

bool BMP::UpLoad(LPDIRECT3DSURFACE9 surf)
{
	if (surf == NULL || isEmpty())
		return false;

	//surf数据
	D3DSURFACE_DESC surfaceDesc;
	surf->GetDesc(&surfaceDesc);

	//锁定surface区域
	D3DLOCKED_RECT lockedRect;
	if (FAILED(surf->LockRect(&lockedRect, NULL, NULL)))
		return false;
	DWORD *surfData = (DWORD*)lockedRect.pBits;

	//拷贝
	int index = 0;
	int wrange = min((int)surfaceDesc.Width, width);//需要拷贝的区域
	int hrange = min((int)surfaceDesc.Height, height);
	int pitch = lockedRect.Pitch >> 2;
	for (int i = 0; i < hrange; i++)
	{
		int stride = i*pitch;
		DWORD *dataRow = (DWORD*)data[i];
		for (int j = 0; j < wrange; j++)
		{
			surfData[stride + j] = dataRow[j];
		}
	}
	surf->UnlockRect();

	return true;
}

bool MYCALL1 BMP::Sample_Area(DWORD * surfData, const POINT & surfSize, ZOOMTYPE zoom, const POINTi64 & ptSurfOffset)
{
	// 预期缩放后尺寸 ( 用于计算实际缩放倍数 )
	INT64 zoomW = (INT64)(width*zoom);
	INT64 zoomH = (INT64)(height*zoom);
	//AUXTYPE zoomActualX = (AUXTYPE)zoomW / width;// 行不通，会失去精度
	//AUXTYPE zoomActualY = (AUXTYPE)zoomH / height;

	// 内部参数错误
	if (width <= 0 || height <= 0)
		return false;
	// 外部参数错误
	if (surfData == NULL ||
		surfSize.x <= 0 || surfSize.y <= 0 ||
		zoom <= 0)
		return false;
	// 新像素所处原图像素越界判断 ( 对于像素边界 pixelX，pixelY 在边界处的越界判断 ) 
	if (
		// 判断首像素前边界越界
		//(AUXTYPE)(0 + ptSurfOffset.x) /*/ zoomActualX*/ < 0 ||
		//(AUXTYPE)(0 + ptSurfOffset.y) /*/ zoomActualY*/ < 0 ||
		// 同上
		ptSurfOffset.x < 0 || ptSurfOffset.y < 0 ||
		// 判断末尾像素边界越界，此处不能用 pixWidth，pixHeight，出于精度问题考虑
		(int)ceil(((AUXTYPE)(surfSize.x + ptSurfOffset.x)) *(AUXTYPE)width / zoomW - 1) >= width ||
		(int)ceil(((AUXTYPE)(surfSize.y + ptSurfOffset.y)) *(AUXTYPE)height / zoomH - 1) >= height
		// 同上
		/*|| (AUXTYPE)(surfSize.x + ptSurfOffset.x) / zoomActualX > width ||
		(AUXTYPE)(surfSize.y + ptSurfOffset.y) / zoomActualY > height*/
		)
		return false;

	// 新像素所处原图像素越界不返回 false，而是缩小合法区间，在原图空间内重采样
	// 使用方法： 把 i, j 循环起始值设为 pixHeadXY，把 SurfSize.xy 设为 pixTailXY
	//int pixHeadX = ptSurfOffset.x < 0 ? -ptSurfOffset.x : 0;
	//int pixHeadY = ptSurfOffset.y < 0 ? -ptSurfOffset.y : 0;
	//int pixTailX = min(surfSize.x, zoomW - ptSurfOffset.x);
	//int pixTailY = min(surfSize.y, zoomH - ptSurfOffset.y);

	// 新图片像素在原图中的像素
	AUXTYPE *pixelEdgeX = new AUXTYPE[surfSize.x + 1];// 新像素左上起始点数组，x 方向
	if (pixelEdgeX == NULL)
		return false;
	bool *bColorX = new bool[surfSize.x + 1];

	// 设置新像素起点值数组 (计算的是新像素边界)
	//  说明：ptSurfOffset 指新图像素偏移，原图只是参照物
	//  (i + ptSurfOffset.x) / zoomActualX 表示新图下标 i 像素左边界在原图中的位置 （相当于多少原图像素数）
	for (int i = 0; i < surfSize.x + 1; i++)
	{
		// 加速，内层 x 循环预计算
		pixelEdgeX[i] = (AUXTYPE)(i + ptSurfOffset.x)*(AUXTYPE)width / zoomW;
		bColorX[i] = (i & TRANSPARENTBACK_MASK) > 0;
	}

	COLOR4f pixelColor = 0;// 颜色累加器
	AUXTYPE pixelEdgeYNext = (AUXTYPE)(0 + ptSurfOffset.y)*(AUXTYPE)height / zoomH;// 像素后边界
	for (int i = 0; i < surfSize.y; i++)// 外层循环: y 方向像素
	{
		AUXTYPE pixelEdgeY = pixelEdgeYNext;// 像素前边界
		pixelEdgeYNext = (AUXTYPE)(i + 1 + ptSurfOffset.y)*(AUXTYPE)height / zoomH;
		bool bColorY = (i & TRANSPARENTBACK_MASK) > 0;

		int starty = (int)pixelEdgeY;// 像素前边界所在原图像素，y 方向
		int endy = (int)ceil(pixelEdgeYNext - 1);// 像素后边界所在原图像素，y 方向
												 /*if (endy > height)
												 endy = height;*/

		int stride = i*surfSize.x;
		for (int j = 0; j < surfSize.x; j++)// 内层循环: x 方向像素
		{
			int index = stride + j;

			// 像素前边界所在原图像素，x 方向
			int startx = (int)pixelEdgeX[j];
			// 像素后边界所在原图像素，x 方向 ( 使用 -1，ceil 因为是后边界的计算，还为了防止越界 )
			int endx = (int)ceil(pixelEdgeX[j + 1] - 1);
			/*if (endx > width)
			endx = width;*/

			COLOR4fAssign(&pixelColor, 0);

			// 范围内像素颜色累加
			AUXTYPE floatsy, floatey;// 新像素包含的原图亚像素的精确起点、终点
			for (int m = starty; m <= endy; m++)
			{
				if (m == starty)
					floatsy = pixelEdgeY;
				else
					floatsy = (AUXTYPE)m;
				if (m == endy)
					floatey = pixelEdgeYNext;
				else
					floatey = m + 1;

				AUXTYPE floatsx, floatex;
				for (int n = startx; n <= endx; n++)
				{
					if (n == startx)
						floatsx = pixelEdgeX[j];
					else
						floatsx = (AUXTYPE)n;
					if (n == endx)
						floatex = pixelEdgeX[j + 1];
					else
						floatex = n + 1;

					COLOR4fAccum(&pixelColor, ((DWORD*)data[m])[n]
						, (COLORTYPE)((floatey - floatsy) * (floatex - floatsx)));
				}
			}

			COLOR4fWNormalize(&pixelColor);

			// alphablend 方法
			if (pixelColor.a < (COLORTYPE)255.0)
			{
				if (bColorY ^ bColorX[j])
				{// 两种背景色
					byte *dest = (byte*)&(surfData[index]);
					byte alpha;
					if (pixelColor.a > 0)
						alpha = (byte)(pixelColor.a + (COLORTYPE)0.5);
					else
						alpha = (byte)(pixelColor.a - (COLORTYPE)0.5);
					dest[3] = ALPHABLEND::alphablendtable2[(byte)(pixelColor.a + (COLORTYPE)0.5)][alpha];
					dest[2] = ALPHABLEND::alphablendtable2[(byte)(pixelColor.r + (COLORTYPE)0.5)][alpha];
					dest[1] = ALPHABLEND::alphablendtable2[(byte)(pixelColor.g + (COLORTYPE)0.5)][alpha];
					dest[0] = ALPHABLEND::alphablendtable2[(byte)(pixelColor.b + (COLORTYPE)0.5)][alpha];
				}
				else
				{
					byte *dest = (byte*)&(surfData[index]);
					byte alpha;
					if (pixelColor.a > 0)
						alpha = (byte)(pixelColor.a + (COLORTYPE)0.5);
					else
						alpha = (byte)(pixelColor.a - (COLORTYPE)0.5);
					dest[3] = ALPHABLEND::alphablendtable1[(byte)(pixelColor.a + (COLORTYPE)0.5)][alpha];
					dest[2] = ALPHABLEND::alphablendtable1[(byte)(pixelColor.r + (COLORTYPE)0.5)][alpha];
					dest[1] = ALPHABLEND::alphablendtable1[(byte)(pixelColor.g + (COLORTYPE)0.5)][alpha];
					dest[0] = ALPHABLEND::alphablendtable1[(byte)(pixelColor.b + (COLORTYPE)0.5)][alpha];
				}
			}
			else
				surfData[index] = COLOR4fGetDW(&pixelColor);
		}
	}
	delete[] pixelEdgeX;
	//delete[] pixelY;

	return true;
}

bool MYCALL1 BMP::Sample_Single(DWORD * surfData, const POINT & surfSize, ZOOMTYPE zoom, const POINTi64 & ptSurfOffset)
{
	// 预期缩放后尺寸 ( 用于计算实际缩放倍数 )
	INT64 zoomW = (INT64)(width*zoom);
	INT64 zoomH = (INT64)(height*zoom);
	//AUXTYPE zoomActualX = (AUXTYPE)zoomW / width;// 行不通，会失去精度
	//AUXTYPE zoomActualY = (AUXTYPE)zoomH / height;
	// 加速的临时变量
	AUXTYPE tmpOffX = ptSurfOffset.x + (AUXTYPE)0.5;
	AUXTYPE tmpOffY = ptSurfOffset.y + (AUXTYPE)0.5;

	// 内部参数错误
	if (width <= 0 || height <= 0)
		return false;
	// 外部参数错误
	if (surfData == NULL ||
		surfSize.x <= 0 || surfSize.y <= 0 ||
		zoom <= 0)
		return false;
	// 新像素所处原图像素越界判断
	AUXTYPE tempZW = (AUXTYPE)width / zoomW;
	AUXTYPE tempZH = (AUXTYPE)height / zoomH;
	if (
		// 判断首像素越界
		/*(0 + tmpOffX) / zoomActualX - (AUXTYPE)0.5 < -(AUXTYPE)0.5||
		(0 + tmpOffY) / zoomActualY - (AUXTYPE)0.5 < -(AUXTYPE)0.5 ||*/
		// 同上
		ptSurfOffset.x < 0 || ptSurfOffset.y < 0 ||
		// 判断末尾像素越界
		(int)((surfSize.x - 1 + tmpOffX)*tempZW) >= width ||
		(int)((surfSize.y - 1 + tmpOffY)*tempZH) >= height
		// 同上
		/*(surfSize.x - 1 + tmpOffX)*(AUXTYPE)width / zoomW - (AUXTYPE)0.5 > width - 1 + (AUXTYPE)0.5 ||
		(surfSize.y - 1 + tmpOffY)*(AUXTYPE)height / zoomH - (AUXTYPE)0.5 > height - 1 + (AUXTYPE)0.5*/
		)
		//Beep(531, 1000);
		return false;

	// 采样辅助参数
	const AUXTYPE auxz = (AUXTYPE)(0.5 / zoom);//(AUXTYPE)(0.5 / zoom)
	const AUXTYPE aux = (AUXTYPE)0.5 / auxz;
	const AUXTYPE auxfront = (AUXTYPE)0.5 - auxz, auxrear = (AUXTYPE)0.5 + auxz;

	AUXTYPE *pixelPosX = new AUXTYPE[surfSize.x];
	bool *alphaX = new bool[surfSize.x];
	if (pixelPosX == NULL || alphaX == NULL)
	{
		SAFE_DELETE_LIST(pixelPosX);
		SAFE_DELETE_LIST(alphaX);
		return false;
	}
	// 设置新像素位置数组 ( 计算的是新像素中点在原图中像素位置 )
	for (int j = 0; j < surfSize.x; j++)
	{
		// 加速，内层 x 循环预计算
		pixelPosX[j] = (j + tmpOffX)*tempZW - (AUXTYPE)0.5;
		alphaX[j] = (j & TRANSPARENTBACK_MASK) > 0;
	}

	if (bAlpha)// 图片有 alpha 通道
	{
		for (int i = 0; i < surfSize.y; i++)
		{
			// y 方向新像素位置，y 是外层循环不使用数组存储
			AUXTYPE pixY = (i + tmpOffY)*tempZH - (AUXTYPE)0.5;
			float ratioYf = (float)(pixY - (int)pixY);
			if (ratioYf < auxfront)
				ratioYf = 0;
			else if (ratioYf > auxrear)
				ratioYf = 1;
			else
				ratioYf = (float)((ratioYf - auxfront) * aux);
			/*if (pixY < 0)
			ratioYf = 0;*/

			int fy, ry;
			fy = (int)pixY;
			ry = fy + 1;
			if (ry >= height)
				ry -= 1;
			DWORD *srcYf = (DWORD*)data[fy];
			DWORD *srcYr = (DWORD*)data[ry];
			/*if (pixY < 0)
			{
				WCHAR str[256] = L"";
				StringCchPrintf(str, 256, L" %lf", ratioYf);
				MessageBox(NULL, str, NULL, 0);
			}*/
			bool alphaY = (i & TRANSPARENTBACK_MASK) > 0;

			int stride = i*surfSize.x;
			for (int j = 0; j < surfSize.x; j++)
			{
				int index = stride + j;

				AUXTYPE pixX = pixelPosX[j];
				float ratioXf = (float)(pixX - (int)pixX);
				if (ratioXf < auxfront)
					ratioXf = 0;
				else if (ratioXf > auxrear)
					ratioXf = 1;
				else
					ratioXf = (float)((ratioXf - auxfront) * aux);
				/*if (pixX < 0)// 不加不会报错
				ratioXf = 0;*/

				byte pixel[4];
				if (!isDecimal2(ratioXf) && !isDecimal2(ratioYf))
				{
					*(DWORD*)pixel = ((DWORD*)data[(int)(pixY + ratioYf)])[(int)(pixX + ratioXf)];
				}
				else
				{
					// 对应原图周围4像素 x 下标
					int fx, rx;
					fx = (int)pixX;
					rx = fx + 1;
					if (rx >= width)// 修正，防越界
						rx -= 1;
					// 对应原图周围4像素
					byte *lt, *rt, *lb, *rb;
					lt = (byte*)&srcYf[fx];
					rt = (byte*)&srcYf[rx];
					lb = (byte*)&srcYr[fx];
					rb = (byte*)&srcYr[rx];

					// 2次插值，可以消除色差
					//  因为一个像素最终混合值不仅和所处原图像素位置相关，还和原图周围像素 alpha 值相关
					float ratioXr = 1 - ratioXf;
					float ratioYr = 1 - ratioYf;
					float fxb, rxb, fxb2, rxb2, fyb, ryb, norm1, norm2, norm3;
					fxb = ratioXf * rt[3];
					rxb = ratioXr * lt[3];
					norm1 = fxb + rxb;
					if (norm1 == 0.0f)
					{
						fxb = rxb = 0.5f;
					}
					else
					{
						fxb /= norm1;
						rxb /= norm1;
					}

					//if (rx >= width || ry>=height)
					//{
					//	//WCHAR str[256] = L"";
					//	//StringCchPrintf(str, 256, L"%lf, %lf", ratioXf, ratioYf);
					//	//MessageBox(NULL, str, NULL, 0);
					//	//rxb /= 4.4f;
					//}
					fxb2 = ratioXf * rb[3];
					rxb2 = ratioXr * lb[3];
					norm2 = fxb2 + rxb2;
					if (norm2 == 0.0f)
					{
						fxb2 = rxb2 = 0.5f;
					}
					else
					{
						fxb2 /= norm2;
						rxb2 /= norm2;
					}

					fyb = ratioYf * norm2;
					ryb = ratioYr * norm1;
					norm3 = fyb + ryb;
					if (norm3 == 0.0f)
					{
						fyb = ryb = 0.5f;
					}
					else
					{
						fyb /= norm3;
						ryb /= norm3;
					}
					// --#3--加速尝试，略快
					float auxrxry = rxb*ryb, auxfxry = fxb*ryb
						, auxrxfy = rxb2*fyb, auxfxfy = fxb2*fyb;
					pixel[0] = (byte)(lt[0] * auxrxry + rt[0] * auxfxry
						+ lb[0] * auxrxfy + rb[0] * auxfxfy);
					pixel[1] = (byte)(lt[1] * auxrxry + rt[1] * auxfxry
						+ lb[1] * auxrxfy + rb[1] * auxfxfy);
					pixel[2] = (byte)(lt[2] * auxrxry + rt[2] * auxfxry
						+ lb[2] * auxrxfy + rb[2] * auxfxfy);

					// alpha 只根据位置比例分配
					pixel[3] = (byte)(((lt[3] * ratioXr + rt[3] * ratioXf) * ratioYr
						+ (lb[3] * ratioXr + rb[3] * ratioXf) * ratioYf));
				}
				//bmppixel = *((DWORD*)&pixel);// 可能效率降低

				if (pixel[3] < (byte)255)
				{// alphablend 方法
					if (alphaY ^ alphaX[j])
					{// 两种背景色
						if (*((DWORD*)&pixel) == ALPHABLEND::lastcolor2)// 缓存加速
						{
							surfData[index] = ALPHABLEND::lastresult2;
						}
						else
						{
							byte *dest = ((byte*)&surfData[index]);

							dest[3] = pixel[3];
							dest[2] = ALPHABLEND::alphablendtable2[pixel[2]][pixel[3]];
							dest[1] = ALPHABLEND::alphablendtable2[pixel[1]][pixel[3]];
							dest[0] = ALPHABLEND::alphablendtable2[pixel[0]][pixel[3]];

							ALPHABLEND::lastcolor2 = *((DWORD*)&pixel);
							ALPHABLEND::lastresult2 = surfData[index];
						}
					}
					else
					{
						if (*((DWORD*)&pixel) == ALPHABLEND::lastcolor1)//缓存加速
						{
							surfData[index] = ALPHABLEND::lastresult1;
						}
						else
						{
							byte *dest = ((byte*)&surfData[index]);

							dest[3] = pixel[3];
							dest[2] = ALPHABLEND::alphablendtable1[pixel[2]][pixel[3]];
							dest[1] = ALPHABLEND::alphablendtable1[pixel[1]][pixel[3]];
							dest[0] = ALPHABLEND::alphablendtable1[pixel[0]][pixel[3]];

							ALPHABLEND::lastcolor1 = *((DWORD*)&pixel);
							ALPHABLEND::lastresult1 = surfData[index];
						}
					}
				}
				else
					surfData[index] = *((DWORD*)&pixel);
			}
		}
	}
	else//无alpha通道
	{
		for (int i = 0; i < surfSize.y; i++)
		{
			// y 方向新像素位置，y 是外层循环不使用数组存储
			AUXTYPE pixY = (i + tmpOffY)*tempZH - (AUXTYPE)0.5;
			float ratioYf = (float)(pixY - (int)pixY);
			if (ratioYf < auxfront)
				ratioYf = 0;
			else if (ratioYf > auxrear)
				ratioYf = 1;
			else
				ratioYf = (float)((ratioYf - auxfront) * aux);
			/*if (pixY < 0)
			ratioYf = 0;*/

			// 原图周围4像素下标
			int fy = (int)pixY;
			int ry = fy + 1;
			if (ry >= height)
				ry -= 1;
			DWORD* srcYf = (DWORD*)data[fy];
			DWORD* srcYr = (DWORD*)data[ry];

			int stride = i*surfSize.x;
			for (int j = 0; j < surfSize.x; j++)
			{
				AUXTYPE pixX = pixelPosX[j];

				float ratioXf = (float)(pixX - (int)pixX);
				if (ratioXf < auxfront)
					ratioXf = 0;
				else if (ratioXf > auxrear)
					ratioXf = 1;
				else
					//TOPLAY: 此行的 aux 改为 zoom，auxz 改为 0.3，AUXTYPE 改成 float 可以体验魔幻效果
					ratioXf = (float)((ratioXf - auxfront) * aux);//*aux
				/*if (pixX < 0)//不加不会报错
				ratioXf = 0;*/

				byte pixel[4];
				// 如果 ratioXf 和 ratioYf 都是 0 或 1，不需要对周围像素混合计算（好像反而更慢）
				//if (!isDecimal2(ratioXf) && !isDecimal2(ratioYf))
				//{
				//	// > 90% 时间花费在这句话和下面else中像素4分量计算的地方
				//	*(DWORD*)pixel = ((DWORD*)data[(int)(pixY + ratioYf)])[(int)(pixX + ratioXf)];
				//}
				//else
				//{
					byte *lt, *rt, *lb, *rb;
					// 原图周围4像素下标
					int fx = (int)pixX;
					int rx = fx + 1;
					if (rx >= width)// 修正，防越界
						rx -= 1;
					// 原图周围4像素
					lt = (byte*)&srcYf[fx];
					rt = (byte*)&srcYf[rx];
					lb = (byte*)&srcYr[fx];
					rb = (byte*)&srcYr[rx];

					// 2次插值-不去色差法
					float ratioXr = 1 - ratioXf;// TODO: 有利于效率？
					float ratioYr = 1 - ratioYf;
					// --#1--不去色差法，rgb 值仅根据位置比例分配
					float auxrxry = ratioXr*ratioYr, auxfxry = ratioXf*ratioYr
						, auxrxfy = ratioXr*ratioYf, auxfxfy = ratioXf*ratioYf;
					pixel[0] = (byte)(lt[0] * auxrxry + rt[0] * auxfxry
						+ lb[0] * auxrxfy + rb[0] * auxfxfy);
					pixel[1] = (byte)(lt[1] * auxrxry + rt[1] * auxfxry
						+ lb[1] * auxrxfy + rb[1] * auxfxfy);
					pixel[2] = (byte)(lt[2] * auxrxry + rt[2] * auxfxry
						+ lb[2] * auxrxfy + rb[2] * auxfxfy);
					pixel[3] = (byte)(lt[3] * auxrxry + rt[3] * auxfxry
						+ lb[3] * auxrxfy + rb[3] * auxfxfy);
					// 整数方法反而更慢一点点
					/*const int scale = 2048;
					int xf = (ratioXf * scale);
					int xr = scale - xf;
					int yf = (ratioYf * scale);
					int yr = scale - yf;
					int auxrxry = xr*yr, auxfxry = xf*yr
					, auxrxfy = xr*ratioYf, auxfxfy = xf*yf;
					int sum = auxrxry + auxfxry + auxrxfy + auxfxfy;
					pixel[0] = (byte)((lt[0] * auxrxry + rt[0] * auxfxry
					+ lb[0] * auxrxfy + rb[0] * auxfxfy)>>22);
					pixel[1] = (byte)((lt[1] * auxrxry + rt[1] * auxfxry
					+ lb[1] * auxrxfy + rb[1] * auxfxfy) >> 22);
					pixel[2] = (byte)((lt[2] * auxrxry + rt[2] * auxfxry
					+ lb[2] * auxrxfy + rb[2] * auxfxfy) >> 22);
					pixel[3] = (byte)((lt[3] * auxrxry + rt[3] * auxfxry
					+ lb[3] * auxrxfy + rb[3] * auxfxfy) >> 22);*/
				//}

				surfData[stride + j] = *((DWORD*)&pixel);
			}
		}
	}
	delete[] pixelPosX;
	delete[] alphaX;

	return true;
}

bool MYCALL1 Sample_NNRotate(BMP *pBmp, DWORD * surfData, const POINT & surfSize, ZOOMTYPE zoom, float rotate, POINTi64 &ptSurfOffset)
{
	// 源图片为空
	if (pBmp == NULL || pBmp->isEmpty())
		return false;

	// 源图数据
	int width = pBmp->width;
	int height = pBmp->height;
	byte **data = pBmp->data;

	// 内部参数错误
	if (width <= 0 || height <= 0)
		return false;
	// 外部参数错误
	if (surfData == NULL ||
		surfSize.x <= 0 || surfSize.y <= 0 ||
		zoom <= 0)
		return false;
	// 新像素所处原图像素越界判断，旋转重采样不用判断
	//if (
	//	// 判断首像素越界
	//	//(int)((0 + ptSurfOffset.x + (AUXTYPE)0.5) *(AUXTYPE)width / zoomW) < 0 ||
	//	//(int)((0 + ptSurfOffset.y + (AUXTYPE)0.5) *(AUXTYPE)height / zoomH) < 0 ||
	//	// 同上
	//	ptSurfOffset.x < 0 || ptSurfOffset.y < 0 ||
	//	// 判断末尾像素越界
	//	(int)((surfSize.x - 1 + ptSurfOffset.x + (AUXTYPE)0.5) *(AUXTYPE)width / zoomW) > width - 1 ||
	//	(int)((surfSize.y - 1 + ptSurfOffset.y + (AUXTYPE)0.5) *(AUXTYPE)height / zoomH) > height - 1
	//	)
	//	return false;


	float angleInnerV = atan((float)width / height);
	float angleL = angleInnerV + rotate;
	float angleR = angleInnerV - rotate;
	double diagLen = sqrt((double)SQ(width) + (double)SQ(height));
	INT64 zoomW = (INT64)ceil(zoom*diagLen*max(abs(sin(angleL)), abs(sin(angleR))));
	INT64 zoomH = (INT64)ceil(zoom*diagLen*max(abs(cos(angleL)), abs(cos(angleR))));//TODO：zoomHW参数传入

	// 加速的临时变量
	AUXTYPE tmpCosR = (AUXTYPE)cos(-rotate);
	AUXTYPE tmpSinR = (AUXTYPE)sin(-rotate);
	AUXTYPE tmpZWOff = ptSurfOffset.x + (AUXTYPE)0.5 - zoomW*(AUXTYPE)0.5;
	AUXTYPE tmpZHOff = zoomH*(AUXTYPE)0.5 - ptSurfOffset.y - (AUXTYPE)0.5;
	AUXTYPE halfw = width*(AUXTYPE)0.5;
	AUXTYPE halfh = height*(AUXTYPE)0.5;

	AUXTYPE *pixelPosXcos = new AUXTYPE[surfSize.x];
	AUXTYPE *pixelPosXsin = new AUXTYPE[surfSize.x];
	bool *alphaX = new bool[surfSize.x];
	if (pixelPosXcos == NULL || pixelPosXsin == NULL || alphaX == NULL)
	{
		SAFE_DELETE_LIST(pixelPosXcos);
		SAFE_DELETE_LIST(pixelPosXsin);
		SAFE_DELETE_LIST(alphaX);
		return false;
	}
	// 设置新像素位置数组 ( 计算的是新像素中点在原图中像素位置，直接计算距离最近的原像素 )
	for (int j = 0; j < surfSize.x; j++)
	{
		// 加速，内层 x 循环预计算
		AUXTYPE tmpX = (j + tmpZWOff) / zoom;
		pixelPosXcos[j] = tmpX*tmpCosR;
		pixelPosXsin[j] = tmpX*tmpSinR;
		
		alphaX[j] = (j & TRANSPARENTBACK_MASK) > 0;
	}
	if (pBmp->bAlpha)// 图片有 alpha 通道
	{
		for (int i = 0; i < surfSize.y; i++)
		{
			AUXTYPE midY = (tmpZHOff - i) / zoom;// TODO加速
			AUXTYPE sinYtmp = midY*tmpSinR + halfw;// 加速临时变量
			AUXTYPE cosYtmp = midY*tmpCosR - halfh;
			bool alphaY = (i & TRANSPARENTBACK_MASK) > 0;

			int stride = i*surfSize.x;
			for (int j = 0; j < surfSize.x; j++)
			{
				int index = stride + j;

				// 计算对应原图中像素 ( 已包含 2 次相逆坐标翻折平移和 1 次坐标旋转 )
				AUXTYPE pixX = pixelPosXcos[j] + sinYtmp;
				AUXTYPE pixY = -cosYtmp + pixelPosXsin[j];

				//DWORD *srcY = (DWORD*)data[pixY];

				if (pixX >= 0 && pixX < width
					&& pixY >= 0 && pixY < height)
				{
					DWORD bmppixel = ((DWORD*)data[(int)pixY])[(int)pixX];//TODO是否需要提取data到外层

					// alphablend 方法
					if ((byte)(bmppixel >> 24) < 255)
					{
						if (alphaY ^ alphaX[j])
						{// 两种背景色
							if (bmppixel == ALPHABLEND::lastcolor2)// 缓存加速
							{
								surfData[index] = ALPHABLEND::lastresult2;
							}
							else
							{
								byte *src = (byte*)&bmppixel;
								byte *dest = (byte*)&(surfData[index]);

								dest[3] = src[3];
								dest[2] = (DWORD)ALPHABLEND::alphablendtable2[src[2]][src[3]];
								dest[1] = (DWORD)ALPHABLEND::alphablendtable2[src[1]][src[3]];
								dest[0] = (DWORD)ALPHABLEND::alphablendtable2[src[0]][src[3]];

								ALPHABLEND::lastcolor2 = bmppixel;
								ALPHABLEND::lastresult2 = *(DWORD*)dest;
							}
						}
						else
						{
							if (bmppixel == ALPHABLEND::lastcolor1)// 缓存加速
							{
								surfData[index] = ALPHABLEND::lastresult1;
							}
							else
							{
								byte *src = (byte*)&bmppixel;
								byte *dest = (byte*)&(surfData[index]);

								dest[3] = src[3];
								dest[2] = (DWORD)ALPHABLEND::alphablendtable1[src[2]][src[3]];
								dest[1] = (DWORD)ALPHABLEND::alphablendtable1[src[1]][src[3]];
								dest[0] = (DWORD)ALPHABLEND::alphablendtable1[src[0]][src[3]];

								ALPHABLEND::lastcolor1 = bmppixel;
								ALPHABLEND::lastresult1 = *(DWORD*)dest;
							}
						}
					}
					else
						surfData[index] = bmppixel;
				}
				else
				{
					surfData[stride + j] = 0;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < surfSize.y; i++)
		{
			AUXTYPE midY = (tmpZHOff - i) / zoom;
			AUXTYPE sinYtmp = midY*tmpSinR + halfw;
			AUXTYPE cosYtmp = midY*tmpCosR - halfh;
			int stride = i*surfSize.x;
			for (int j = 0; j < surfSize.x; j++)
			{
				// 不能先转化为整数，因为比如 -0.6 取整结果是 0
				AUXTYPE pixX = pixelPosXcos[j] + sinYtmp;
				AUXTYPE pixY = -cosYtmp + pixelPosXsin[j];

				// 直接拷贝像素
				if (pixX >= 0 && pixX < width
					&& pixY >= 0 && pixY < height)
				{
					surfData[stride + j] = ((DWORD*)data[(int)pixY])[(int)pixX];
				}
				else
				{
					surfData[stride + j] = 0;
				}
			}
		}
	}
	delete[] pixelPosXcos;
	delete[] pixelPosXsin;
	delete[] alphaX;

	return true;
}

bool MYCALL1 Sample_NN(BMP *pBmp, DWORD * surfData, const POINT & surfSize, ZOOMTYPE zoom, POINTi64 &ptSurfOffset)
{
	// 源图片为空
	if (pBmp == NULL || pBmp->isEmpty())
		return false;

	// 源图数据
	int width = pBmp->width;
	int height = pBmp->height;
	byte **data = pBmp->data;

	// 预期缩放后尺寸 ( 用于计算实际缩放倍数 )
	INT64 zoomW = (INT64)(width*zoom);
	INT64 zoomH = (INT64)(height*zoom);
	//AUXTYPE zoomActualX = (AUXTYPE)zoomW / width;// 行不通，会失去精度
	//AUXTYPE zoomActualY = (AUXTYPE)zoomH / height;
	// 加速的临时变量
	AUXTYPE tmpOffX = ptSurfOffset.x + (AUXTYPE)0.5;
	AUXTYPE tmpOffY = ptSurfOffset.y + (AUXTYPE)0.5;

	// 内部参数错误
	if (width <= 0 || height <= 0)
		return false;
	// 外部参数错误
	if (surfData == NULL ||
		surfSize.x <= 0 || surfSize.y <= 0 ||
		zoom <= 0)
		return false;
	// 新像素所处原图像素越界判断
	AUXTYPE tempZW = (AUXTYPE)width / zoomW;
	AUXTYPE tempZH = (AUXTYPE)height / zoomH;
	if (
		// 判断首像素越界
		//(int)((0 + tmpOffX)*tempZW) < 0 ||
		//(int)((0 + tmpOffY)*tempZH) < 0 ||
		// 同上
		ptSurfOffset.x < 0 || ptSurfOffset.y < 0 ||
		// 判断末尾像素越界
		(int)((surfSize.x - 1 + tmpOffX)*tempZW) >= width ||
		(int)((surfSize.y - 1 + tmpOffY)*tempZH) >= height
		)
		return false;

	int *pixelPosX = new int[surfSize.x];
	bool *alphaX = new bool[surfSize.x];
	if (pixelPosX == NULL || alphaX == NULL)
	{
		SAFE_DELETE_LIST(pixelPosX);
		SAFE_DELETE_LIST(alphaX);
		return false;
	}
	// 设置新像素位置数组 ( 计算的是新像素中点在原图中像素位置，直接计算距离最近的原像素 )
	//int xrIntFloat_16 = (width << 16) / zoomW; //16.16格式定点数
	for (int x = 0; x < surfSize.x; x++)
	{
		// 加速，内层 x 循环预计算
		pixelPosX[x] = (int)((x + tmpOffX)*tempZW);
		alphaX[x] = (x & TRANSPARENTBACK_MASK) > 0;
		// 这个方法需满足(surfSize.x - 1 + ptSurfOffset.x)*xrIntFloat_16 + 1 不超出类型范围
		// 速度很快-3 倍，效果有差，像素不平分全图，该轴末尾像素被截去部分
		//pixelPosX[x] = (((int)((x + ptSurfOffset.x)*xrIntFloat_16) + 1) >> 16);
	}

	if (pBmp->bAlpha)
	{// 图片有 alpha 通道
		for (int y = 0; y < surfSize.y; y++)
		{
			int pixY = (int)((y + tmpOffY)*tempZH);
			bool alphaY = (y & TRANSPARENTBACK_MASK) > 0;
			DWORD *srcY = (DWORD*)data[pixY];
			
			int stride = y*surfSize.x;// 可能略快
			for (int x = 0; x < surfSize.x; x++)
			{
				int index = stride + x;
				DWORD bmppixel = srcY[pixelPosX[x]];

				// alphablend 方法
				if ((bmppixel >> 24) < 255)
				{
					if (alphaY ^ alphaX[x])
					{// 两种背景色
						if (bmppixel == ALPHABLEND::lastcolor2)// 缓存加速
						{
							surfData[index] = ALPHABLEND::lastresult2;
						}
						else
						{
							byte *src = (byte*)&bmppixel;
							byte *dest = (byte*)&(surfData[index]);

							dest[3] = src[3];
							dest[2] = (DWORD)ALPHABLEND::alphablendtable2[src[2]][src[3]];
							dest[1] = (DWORD)ALPHABLEND::alphablendtable2[src[1]][src[3]];
							dest[0] = (DWORD)ALPHABLEND::alphablendtable2[src[0]][src[3]];

							ALPHABLEND::lastcolor2 = bmppixel;
							ALPHABLEND::lastresult2 = *(DWORD*)dest;
						}
					}
					else
					{
						if (bmppixel == ALPHABLEND::lastcolor1)// 缓存加速
						{
							surfData[index] = ALPHABLEND::lastresult1;
						}
						else
						{
							byte *src = (byte*)&bmppixel;
							byte *dest = (byte*)&(surfData[index]);

							dest[3] = src[3];
							dest[2] = (DWORD)ALPHABLEND::alphablendtable1[src[2]][src[3]];
							dest[1] = (DWORD)ALPHABLEND::alphablendtable1[src[1]][src[3]];
							dest[0] = (DWORD)ALPHABLEND::alphablendtable1[src[0]][src[3]];

							ALPHABLEND::lastcolor1 = bmppixel;
							ALPHABLEND::lastresult1 = *(DWORD*)dest;
						}
					}
				}
				else
					surfData[index] = bmppixel;
			}
		}
	}
	else
	{// 无 alpha 通道
		for (int i = 0; i < surfSize.y; i++)
		{
			int pixY = (int)((i + tmpOffY)*tempZH);
			DWORD *srcY = (DWORD*)data[pixY];
			int stride = i*surfSize.x;// 可能略快
			for (int j = 0; j < surfSize.x; j++)
			{
				// 直接拷贝像素
				surfData[stride + j] = srcY[pixelPosX[j]];
			}
		}
	}
	delete[] pixelPosX;
	delete[] alphaX;

	return true;
}

bool MYCALL1 Sample_BiLinear(BMP *pBmp, DWORD * surfData, const POINT & surfSize, ZOOMTYPE zoom, const POINTi64 & ptSurfOffset)
{
	// 源图片为空
	if (pBmp == NULL || pBmp->isEmpty())
		return false;

	// 源图数据
	int width = pBmp->width;
	int height = pBmp->height;
	byte **data = pBmp->data;

	// 预期缩放后尺寸（用于计算实际缩放倍数）
	INT64 zoomW = (INT64)(width*zoom);
	INT64 zoomH = (INT64)(height*zoom);
	// 加速的临时变量
	AUXTYPE tmpOffX = ptSurfOffset.x + (AUXTYPE)0.5;
	AUXTYPE tmpOffY = ptSurfOffset.y + (AUXTYPE)0.5;

	// 内部参数错误
	if (width <= 0 || height <= 0)
		return false;
	// 外部参数错误
	if (surfData == NULL ||
		surfSize.x <= 0 || surfSize.y <= 0 ||
		zoom <= 0)
		return false;
	// 新像素所处原图像素越界判断 ( 应该和 Sample_Single 一样 )
	AUXTYPE tempZW = (AUXTYPE)width / zoomW;
	AUXTYPE tempZH = (AUXTYPE)height / zoomH;
	if (
		// 判断首像素越界
		ptSurfOffset.x < 0 || ptSurfOffset.y < 0 ||
		// 判断末尾像素越界
		(int)((surfSize.x - 1 + tmpOffX)*tempZW) >= width ||
		(int)((surfSize.y - 1 + tmpOffY)*tempZH) >= height
		// 同上
		/*(surfSize.x - 1 + tmpOffX)*(AUXTYPE)width / zoomW - (AUXTYPE)0.5 > width - 1 + (AUXTYPE)0.5 ||
		(surfSize.y - 1 + tmpOffY)*(AUXTYPE)height / zoomH - (AUXTYPE)0.5 > height - 1 + (AUXTYPE)0.5*/
		)
		return false;

	AUXTYPE *pixelPosX = new AUXTYPE[surfSize.x];
	bool *alphaX = new bool[surfSize.x];
	if (pixelPosX == NULL || alphaX == NULL)
	{
		SAFE_DELETE_LIST(pixelPosX);
		SAFE_DELETE_LIST(alphaX);
		return false;
	}
	// 设置新像素位置数组 ( 计算的是新像素中点在原图中像素位置 )
	for (int j = 0; j < surfSize.x; j++)
	{
		// 加速，内层 x 循环预计算
		pixelPosX[j] = (j + tmpOffX)*tempZW - (AUXTYPE)0.5;
		alphaX[j] = (j & TRANSPARENTBACK_MASK) > 0;
	}

	if (pBmp->bAlpha)// 图片有 alpha 通道
	{
		for (int i = 0; i < surfSize.y; i++)
		{
			AUXTYPE pixY = (i + tmpOffY)*tempZH - (AUXTYPE)0.5;
			float ratioYf = (float)(pixY - (int)pixY);
			/*if (pixY < 0)
			ratioYf = 0;*/
			float ratioYr = 1 - ratioYf;

			int fy = (int)pixY;
			int ry = fy + 1;
			if (ry >= height)// 修正，防越界
				ry -= 1;
			DWORD* srcYf = (DWORD*)data[fy];
			DWORD* srcYr = (DWORD*)data[ry];

			bool alphaY = (i & TRANSPARENTBACK_MASK) > 0;

			int stride = i*surfSize.x;
			for (int j = 0; j < surfSize.x; j++)
			{
				int index = stride + j;

				// 对应原图片素位置
				AUXTYPE pixX = pixelPosX[j];
				float ratioXf = (float)(pixX - (int)pixX);
				/*if (pixX < 0)
				ratioXf = 0;*/
				float ratioXr = 1 - ratioXf;

				byte *lt, *rt, *lb, *rb;
				// 对应原图周围4像素下标
				int fx = (int)pixX;
				int rx = fx + 1;
				if (rx >= width)//修正，防越界
					rx -= 1;
				// 对应原图周围4像素
				lt = (byte*)&srcYf[fx];
				rt = (byte*)&srcYf[rx];
				lb = (byte*)&srcYr[fx];
				rb = (byte*)&srcYr[rx];

				// 2次插值，可以消除色差
				//  因为一个像素最终混合值不仅和所处原图像素位置相关，还和原图周围像素 alpha 值相关
				float fxb, rxb, fxb2, rxb2, fyb, ryb, norm1, norm2, norm3;
				fxb = ratioXf * rt[3];
				rxb = ratioXr * lt[3];
				norm1 = fxb + rxb;
				if (norm1 != 0.0f)
				{
					fxb /= norm1;
					rxb /= norm1;
				}
				else
				{
					fxb = rxb = 0.5;
				}

				fxb2 = ratioXf * rb[3];
				rxb2 = ratioXr * lb[3];
				norm2 = fxb2 + rxb2;
				if (norm2 != 0.0f)
				{
					fxb2 /= norm2;
					rxb2 /= norm2;
				}
				else
				{
					fxb2 = rxb2 = 0.5f;
				}

				fyb = ratioYf * norm2;
				ryb = ratioYr * norm1;
				norm3 = fyb + ryb;
				if (norm3 != 0.0f)
				{
					fyb /= norm3;
					ryb /= norm3;
				}
				else
				{
					fyb = ryb = 0.5f;
				}
				// 90 % 的时间花费在这 5 句
				byte pixel[4];
				float auxrxry = rxb*ryb, auxfxry = fxb*ryb, auxrxfy = rxb2*fyb, auxfxfy = fxb2*fyb;
				pixel[0] = (byte)(lt[0] * auxrxry + rt[0] * auxfxry
					+ lb[0] * auxrxfy + rb[0] * auxfxfy);
				pixel[1] = (byte)(lt[1] * auxrxry + rt[1] * auxfxry
					+ lb[1] * auxrxfy + rb[1] * auxfxfy);
				pixel[2] = (byte)(lt[2] * auxrxry + rt[2] * auxfxry
					+ lb[2] * auxrxfy + rb[2] * auxfxfy);

				// alpha 只根据位置比例分配
				pixel[3] = (byte)(((lt[3] * ratioXr + rt[3] * ratioXf) * ratioYr
					+ (lb[3] * ratioXr + rb[3] * ratioXf) * ratioYf));

				// alphablend 方法
				if (pixel[3] < 255)
				{
					if (alphaY ^ alphaX[j])
					{// 两种背景色
						if (*((DWORD*)&pixel) == ALPHABLEND::lastcolor2)// 缓存加速
						{
							surfData[index] = ALPHABLEND::lastresult2;
						}
						else
						{
							byte *dest = (byte*)&(surfData[index]);

							dest[3] = pixel[3];
							dest[2] = ALPHABLEND::alphablendtable2[pixel[2]][pixel[3]];
							dest[1] = ALPHABLEND::alphablendtable2[pixel[1]][pixel[3]];
							dest[0] = ALPHABLEND::alphablendtable2[pixel[0]][pixel[3]];

							ALPHABLEND::lastcolor2 = *((DWORD*)&pixel);
							ALPHABLEND::lastresult2 = surfData[index];
						}
					}
					else
					{
						if (*((DWORD*)&pixel) == ALPHABLEND::lastcolor1)// 缓存加速
						{
							surfData[index] = ALPHABLEND::lastresult1;
						}
						else
						{
							byte *dest = (byte*)&(surfData[index]);

							dest[3] = pixel[3];
							dest[2] = ALPHABLEND::alphablendtable1[pixel[2]][pixel[3]];
							dest[1] = ALPHABLEND::alphablendtable1[pixel[1]][pixel[3]];
							dest[0] = ALPHABLEND::alphablendtable1[pixel[0]][pixel[3]];

							ALPHABLEND::lastcolor1 = *((DWORD*)&pixel);
							ALPHABLEND::lastresult1 = surfData[index];
						}
					}
				}
				else
					surfData[index] = *((DWORD*)&pixel);
			}
		}
	}
	else
	{
		for (int i = 0; i < surfSize.y; i++)
		{
			AUXTYPE pixY = (i + tmpOffY)*tempZH - (AUXTYPE)0.5;
			float ratioYf = (float)(pixY - (int)pixY);
			if (pixY < 0)// TODO
			ratioYf = 0;
			float ratioYr = 1 - ratioYf;

			int fy = (int)pixY;
			int ry = fy + 1;
			if (ry >= height)// 修正，防越界
				ry -= 1;
			DWORD* srcYf = (DWORD*)data[fy];
			DWORD* srcYr = (DWORD*)data[ry];

			int stride = i*surfSize.x;
			for (int j = 0; j < surfSize.x; j++)
			{
				// 对应原图片像素位置
				AUXTYPE pixX = pixelPosX[j];
				float ratioXf = (float)(pixX - (int)pixX);
				if (pixX < 0)
				ratioXf = 0;
				float ratioXr = 1 - ratioXf;

				byte *lt, *rt, *lb, *rb;
				// 对应原图周围 4 像素下标
				int fx = (int)pixX;
				int rx = fx + 1;
				if (rx >= width)//修正，防越界
					rx -= 1;
				// 对应原图周围 4 像素
				lt = (byte*)&srcYf[fx];
				rt = (byte*)&srcYf[rx];
				lb = (byte*)&srcYr[fx];
				rb = (byte*)&srcYr[rx];

				// 2次插值-不去色差法  > 90% 的时间花费在这几句里
				byte *pixel= (byte*)&surfData[stride + j];
				float auxrxry = ratioXr*ratioYr, auxfxry = ratioXf*ratioYr
					, auxrxfy = ratioXr*ratioYf, auxfxfy = ratioXf*ratioYf;
				pixel[0] = (byte)((lt[0] * auxrxry + rt[0] * auxfxry
					+ lb[0] * auxrxfy + rb[0] * auxfxfy));
				pixel[1] = (byte)((lt[1] * auxrxry + rt[1] * auxfxry
					+ lb[1] * auxrxfy + rb[1] * auxfxfy));
				pixel[2] = (byte)((lt[2] * auxrxry + rt[2] * auxfxry
					+ lb[2] * auxrxfy + rb[2] * auxfxfy));
				/*pixel[3] = 255;*/

				/*float mt0 = (lt[0] + (rt[0] - lt[0])*ratioXf);
				float mb0 = (lb[0] + (rb[0] - lb[0])*ratioXf);
				pixel[0] = (byte)(mt0 + (mb0 - mt0)*ratioYf);
				float mt1 = (lt[1] + (rt[1] - lt[1])*ratioXf);
				float mb1 = (lb[1] + (rb[1] - lb[1])*ratioXf);
				pixel[1] = (byte)(mt1 + (mb1 - mt1)*ratioYf);
				float mt2 = (lt[2] + (rt[2] - lt[2])*ratioXf);
				float mb2 = (lb[2] + (rb[2] - lb[2])*ratioXf);
				pixel[2] = (byte)(mt2 + (mb2 - mt2)*ratioYf);
				float mt3 = (lt[3] + (rt[3] - lt[3])*ratioXf);
				float mb3 = (lb[3] + (rb[3] - lb[3])*ratioXf);
				pixel[3] = (byte)(mt3 + (mb3 - mt3)*ratioYf);*/
			}
		}
	}
	delete[] pixelPosX;
	delete[] alphaX;

	return true;
}

void BMP::TestInc()
{
	if (isEmpty())
		return;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			byte *ppix = (byte*)&((DWORD*)data[i])[j];
			ppix[0]++;
			ppix[1]++;
			ppix[2]++;
		}
	}
}

void BMP::TestAlpha(BMP *pb)
{
	if (isEmpty() || pb == NULL)
		return;

	byte **pd = pb->data;
	for (int i = 0; i < height; i++)
	{
		DWORD *destRow = (DWORD*)data[i];
		DWORD *srcRow = (DWORD*)pd[i];
		for (int j = 0; j < width; j++)
		{
			byte *ppixs = (byte*)&srcRow[j];
			byte *ppixd = (byte*)&destRow[j];
			byte alpha = ppixs[0];
			byte aalpha = 255 - alpha;
			ppixd[3] = ppixs[3];
			ppixd[0] = ROUND_BYTE((ppixs[0] * alpha + ppixd[0] * aalpha) / 255.0f);
			ppixd[1] = ROUND_BYTE((ppixs[1] * alpha + ppixd[1] * aalpha) / 255.0f);
			ppixd[2] = ROUND_BYTE((ppixs[2] * alpha + ppixd[2] * aalpha) / 255.0f);
		}
	}
}

bool MYCALL1 BMP::Gray()
{
	if (isEmpty())
		return false;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			byte *ppix = (byte*)&((DWORD*)data[i])[j];
			byte gray = RGB2GRAY(ppix[0], ppix[1], ppix[2]);
			ppix[0] = ppix[1] = ppix[2] = gray;
		}
	}
	return true;
}

bool MYCALL1 BMP::Inverse()
{
	if (isEmpty())
		return false;

	for (int i = 0; i < height; i++)
	{
		byte *ppix = data[i];
		for (int j = 0; j < width; j++)
		{
			/*ppix[0] = ~ppix[0];
			ppix[1] = ~ppix[1];*/
			*(UINT16*)ppix = ~(*(UINT16*)ppix);
			ppix[2] = ~ppix[2];
			ppix += 4;
		}
	}
	return true;
}

bool MYCALL1 BMP::LOSE_R()
{
	if (isEmpty())
		return false;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			((byte*)&((DWORD*)data[i])[j])[2] = 0;
		}
	}
	return true;
}

bool MYCALL1 BMP::LOSE_G()
{
	if (isEmpty())
		return false;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			((byte*)&((DWORD*)data[i])[j])[1] = 0;
		}
	}
	return true;
}

bool MYCALL1 BMP::LOSE_B()
{
	if (isEmpty())
		return false;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			((byte*)&((DWORD*)data[i])[j])[0] = 0;
		}
	}
	return true;
}

bool MYCALL1 BMP::RGB2HSV()
{
	if (isEmpty())
		return false;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			byte *ppix = (byte*)&((DWORD*)data[i])[j];

			byte r = ppix[2];
			byte g = ppix[1];
			byte b = ppix[0];
			byte max = 0;
			byte min = 1;
			if (g > b)
			{
				max = 1;
				min = 0;
			}
			if (r > ppix[max])
				max = 2;
			else if (r < ppix[min])
				min = 2;
			byte minvalue = ppix[min];
			byte maxvalue = ppix[max];

			ppix[0] = maxvalue;//v
			ppix[1] = (maxvalue == 0 ? 0 : ROUNDF_BYTE(255.0f*(maxvalue - minvalue) / maxvalue));//s

			if (maxvalue == minvalue)
				ppix[2] = 0;
			else
			{
				if (max == 2)
				{
					ppix[2] = ROUNDF_BYTE(255.0f * (60 * (g - b) / (maxvalue - minvalue)) / 360);
				}
				else if (max == 1)
				{
					ppix[2] = ROUNDF_BYTE(255.0f * (120 + 60 * (b - r) / (maxvalue - minvalue)) / 360);
				}
				else
				{
					ppix[2] = ROUNDF_BYTE(255.0f * (240 + 60 * (r - g) / (maxvalue - minvalue)) / 360);
				}
			}
		}
	}
	return true;
}

bool BMP::Diff(BMP * pre)
{
	if (pre == NULL)
		return false;
	if (isEmpty() || pre->isEmpty())
		return false;
	if (width != pre->width || height != pre->height)
		return false;

	for (int y = 0; y < height; y++)
	{
		DWORD *row = (DWORD*)data[y];
		DWORD *rowB = (DWORD*)pre->data[y];
		for (int x = 0; x < width; x++)
		{
			byte *ppix = (byte*)&row[x];
			byte *ppixB = (byte*)&rowB[x];
			ppix[0] = abs(ppix[0] - ppixB[0]);
			ppix[1] = abs(ppix[1] - ppixB[1]);
			ppix[2] = abs(ppix[2] - ppixB[2]);
		}
	}
	return true;
}

void MYCALL1 BMP::Clear()
{
	if (data != NULL)//释放二维空间
	{
		for (int i = 0; i < height; i++)
		{
			SAFE_DELETE_LIST(data[i]);
		}

		DELETE_LIST(data);
	}
	width = 0;
	height = 0;
}

string GetFMTStr(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_R8G8B8:
		return "D3DFMT_R8G8B8";
	case D3DFMT_A8R8G8B8:
		return "D3DFMT_A8R8G8B8";
	case D3DFMT_X8R8G8B8:
		return "D3DFMT_X8R8G8B8";
	case D3DFMT_R5G6B5:
		return "D3DFMT_R5G6B5";
	case D3DFMT_X1R5G5B5:
		return "D3DFMT_X1R5G5B5";
	case D3DFMT_A1R5G5B5:
		return "D3DFMT_A1R5G5B5";
	case D3DFMT_A4R4G4B4:
		return "D3DFMT_A4R4G4B4";
	case D3DFMT_R3G3B2:
		return "D3DFMT_R3G3B2";
	case D3DFMT_A8:
		return "D3DFMT_A8";
	case D3DFMT_A8R3G3B2:
		return "D3DFMT_A8R3G3B2";
	case D3DFMT_X4R4G4B4:
		return "D3DFMT_X4R4G4B4";
	case D3DFMT_A2B10G10R10:
		return "D3DFMT_A2B10G10R10";
	case D3DFMT_A8B8G8R8:
		return "D3DFMT_A8B8G8R8";
	case D3DFMT_X8B8G8R8:
		return "D3DFMT_X8B8G8R8";
	case D3DFMT_G16R16:
		return "D3DFMT_G16R16";
	case D3DFMT_A2R10G10B10:
		return "D3DFMT_A2R10G10B10";
	case D3DFMT_A16B16G16R16:
		return "D3DFMT_A16B16G16R16";
	case D3DFMT_A8P8:
		return "D3DFMT_A8P8";
	case D3DFMT_P8:
		return "D3DFMT_P8";
	case D3DFMT_L8:
		return "D3DFMT_L8";
	case D3DFMT_A8L8:
		return "D3DFMT_A8L8";
	case D3DFMT_A4L4:
		return "D3DFMT_A4L4";
	case D3DFMT_V8U8:
		return "D3DFMT_V8U8";
	case D3DFMT_L6V5U5:
		return "D3DFMT_L6V5U5";
	case D3DFMT_X8L8V8U8:
		return "D3DFMT_X8L8V8U8";
	case D3DFMT_Q8W8V8U8:
		return "D3DFMT_Q8W8V8U8";
	case D3DFMT_V16U16:
		return "D3DFMT_V16U16";
	case D3DFMT_A2W10V10U10:
		return "D3DFMT_A2W10V10U10";
	default:
		return "UNKNOWN";
	}
}

PicPack::PicPack()
{
	pBmp = new BMP;

	ZeroMemory(&myPicInfo, sizeof(myPicInfo));
	ZeroMemory(&D3DPicInfo, sizeof(D3DPicInfo));
	strFileName[0] = L'\0';

	ZeroMemory(&liveInfo, sizeof(liveInfo));
	liveInfo.rotate = 0;
	liveInfo.zoom = 1;

	bSaved = false;
	bHasDirectory = false;
}

PicPack::~PicPack()
{
	SAFE_DELETE(pBmp);
	pBmp = NULL;
}

const WCHAR * PicPack::GetFileName() const
{
	return (const WCHAR*)strFileName;
}

HRESULT PicPack::LoadFile(LPDIRECT3DDEVICE9 pDevice, WCHAR file[])
{
	if (pDevice == NULL)
		return E_FAIL;
	if (pBmp == NULL)
	{
		// 如果图片指针空，尝试 new 一次，失败则返回
		pBmp = new BMP;
		if (pBmp == NULL)
			return E_FAIL;
	}

	// 获取图片信息
	HRESULT hr;
	ZeroMemory(&D3DPicInfo, sizeof(D3DXIMAGE_INFO));
	D3DXGetImageInfoFromFileW(file, &D3DPicInfo);
	// 获取基本信息
	ZeroMemory(&myPicInfo, sizeof(PicInfo));
	if (!myPicInfo.ReadFile(file))
	{
		// 获取信息失败
		ZeroMemory(&D3DPicInfo, sizeof(D3DXIMAGE_INFO));
		ZeroMemory(&myPicInfo, sizeof(PicInfo));
		return E_FAIL;
	}

	// 创建与图片匹配表面
	LPDIRECT3DSURFACE9 tempsurf = NULL;
	hr = pDevice->CreateOffscreenPlainSurface(
		(LONG)myPicInfo.width, (LONG)myPicInfo.height, D3DFMT_A8R8G8B8
		, D3DPOOL_SYSTEMMEM, &tempsurf, NULL);
	if (FAILED(hr))
	{
		// 创建表面失败
		ZeroMemory(&D3DPicInfo, sizeof(D3DXIMAGE_INFO));
		ZeroMemory(&myPicInfo, sizeof(PicInfo));
		return hr;
	}
	// 装载图片
	hr = D3DXLoadSurfaceFromFileW(
		tempsurf, NULL, NULL, file
		, NULL, D3DX_FILTER_NONE, 0x00000000, NULL);
	if (FAILED(hr))
	{
		// 装载图片失败
		ZeroMemory(&D3DPicInfo, sizeof(D3DXIMAGE_INFO));
		ZeroMemory(&myPicInfo, sizeof(PicInfo));
		return hr;
	}

	// 存入 BMP
	if (!pBmp->Load(tempsurf))
	{
		// 存入 BMP 失败
		ZeroMemory(&D3DPicInfo, sizeof(D3DXIMAGE_INFO));
		ZeroMemory(&myPicInfo, sizeof(PicInfo));
		return E_FAIL;// 从 tempsurface 加载图片
	}
	pBmp->SetAlpha(myPicInfo.channels == 4);// 设置 alpha

	// 文件名
	StringCchCopy(strFileName, MAX_PATH, file);

	// 状态更新
	bHasDirectory = true;
	bSaved = true;

	// 清除 surface
	SAFE_RELEASE(tempsurf);

	// 更新信息字符串
	UpdatePicInfoStr();

	return S_OK;
}

bool PicPack::SaveFile(LPDIRECT3DDEVICE9 pDevice, WCHAR file[])
{
	if (pDevice == NULL ||
		pBmp == NULL || pBmp->isEmpty())
		return false;

	HRESULT hr;
	LPDIRECT3DSURFACE9 tempsurf = NULL;
	hr = pDevice->CreateOffscreenPlainSurface(
		pBmp->width, pBmp->height
		, D3DFMT_A8R8G8B8
		, D3DPOOL_SYSTEMMEM
		, &tempsurf
		, NULL);
	FAILED_RETURN_FALSE(hr);

	FALSE_RETURN(pBmp->UpLoad(tempsurf));//从tempsurface加载图片
	hr = D3DXSaveSurfaceToFile(file, D3DPicInfo.ImageFileFormat, tempsurf, NULL, NULL);//保存到文件

	// 更新文件名
	StringCchCopyW(strFileName, MAX_PATH, file);

	// 状态更新
	bSaved = true;

	// 更新信息字符串
	UpdatePicInfoStr();

	return !FAILED(hr);
}

void PicPack::UpdatePicInfoStr()
{
	//图片尺寸、格式
	StringCchPrintf(picInfoStr, 256, L"SIZE: %d × %d\n\
		FORMAT: %S\n\
		FILE: %lld Bytes  [%.3fMB]\n\
		DEPTH: %d\n\
		CHANNEL: %d\n\
		ALPHA: %d"
		, pBmp->width, pBmp->height
		, GetFMTStr(D3DPicInfo.Format).c_str()
		, myPicInfo.bytecount, myPicInfo.bytecount / 1048576.0f
		, myPicInfo.generaldepth
		, myPicInfo.channels
		, pBmp->bAlpha);
}

const WCHAR * PicPack::GetPicInfoStr() const
{
	return (const WCHAR*)picInfoStr;
}

bool PicPackList::SetSizeInternal(int newSize)
{
	if (newSize < 0 || (newSize > INT_MAX / sizeof(PicPack*)))
	{
		return false;
	}

	if (newSize == 0)
	{
		// Shrink to 0 size & cleanup
		if (data)
		{
			free(data);
			data = NULL;
		}

		size = 0;
		count = 0;
	}
	else if (data == NULL || newSize > size)
	{
		// Grow array
		int nGrowBy = (size == 0) ? 8 : size;

		// Limit nGrowBy to keep m_nMaxSize less than INT_MAX
		if ((UINT)size + (UINT)nGrowBy > (UINT)INT_MAX)
			nGrowBy = INT_MAX - size;

		newSize = __max(newSize, size + nGrowBy);

		// Verify that (nNewMaxSize * sizeof(TYPE)) is not greater than UINT_MAX or the realloc will overrun
		if (sizeof(PicPack*) > UINT_MAX / (UINT)newSize)
			return E_INVALIDARG;

		PicPack** dataNew = (PicPack**)realloc(data, newSize * sizeof(PicPack*));
		if (dataNew == NULL)
			return false;

		data = dataNew;
		size = newSize;
	}

	return true;
}

PicPackList::PicPackList()
{
	SetSizeInternal(8);
	//size = PICPACKLIST_SIZE_INIT;
	//data = MALLOC(PicPack*, size);
	count = 0;
	cur = 0;

	pLivePicPack = NULL;
	pLiveBMP = NULL;
}

int PicPackList::GetCurPos() const
{
	return cur;
}

int PicPackList::GetCount() const
{
	return count;
}

PicPack * PicPackList::SetPicPack(int idx)
{
	if (count < 1)
		return NULL;

	if (idx <= count && idx > 0)
	{
		cur = idx;

		pLivePicPack = data[cur - 1];
		pLiveBMP = pLivePicPack->GetPBMP();

		return data[cur - 1];
	}
	else
	{
		cur = 0;
		return NULL;
	}
}

PicPack * PicPackList::SetTailPicPack()
{
	if (count < 1)
		return NULL;

	cur = count;

	pLivePicPack = data[cur - 1];
	pLiveBMP = pLivePicPack->GetPBMP();

	return data[cur - 1];
}

PicPack * PicPackList::SetPrev()
{
	if (count < 1)
		return NULL;

	int oldcur = cur;
	cur--;
	if (cur < 1)
		cur = count;

	if (cur <= count && cur > 0)
	{
		pLivePicPack = data[cur - 1];
		pLiveBMP = pLivePicPack->GetPBMP();

		return data[cur - 1];
	}
	else
	{
		cur = oldcur;

		return NULL;
	}
}

PicPack * PicPackList::GetPrev()
{
	if (count < 1)
		return NULL;

	int pre = cur;
	pre--;
	if (pre < 1)
		pre = count;

	if (pre <= count && pre > 0)
	{
		return data[pre - 1];
	}
	else
	{
		return NULL;
	}
}

PicPack * PicPackList::SetNext()
{
	if (count < 1)
		return NULL;

	int oldcur = cur;
	cur++;
	if (cur > count)
		cur = 1;

	if (cur <= count && cur > 0)
	{
		pLivePicPack = data[cur - 1];
		pLiveBMP = pLivePicPack->GetPBMP();

		return data[cur - 1];
	}
	else
	{
		cur = oldcur;

		return NULL;
	}
}

PicPack * PicPackList::Drop()
{
	if (count < 1 || cur>count || cur <= 0)
		return NULL;

	delete data[cur - 1];
	for (int i = cur; i <= count - 1; i++)
	{
		data[i - 1] = data[i];
	}
	if (cur == count)
		cur--;
	count--;

	if (count >= 1 && cur <= count && cur > 0)//删除后还有图片
	{
		pLivePicPack = data[cur - 1];
		pLiveBMP = pLivePicPack->GetPBMP();

		return data[cur - 1];
	}
	else//删除后列表中没图片了
	{
		cur = 0;

		pLivePicPack = NULL;
		pLiveBMP = NULL;

		return NULL;
	}
}

bool PicPackList::Add(PicPack * newpp)
{
	if (newpp)
	{
		if (count >= size)
		{
			if (!SetSizeInternal(size + 1))
				return false;
		}
		count++;
		data[count - 1] = newpp;
		return true;
	}
	else
	{
		return false;
	}
}

SurferBase::SurferBase()
{
	Clear();
}

void SurferBase::ClearPic()
{
	pBmp = NULL;
	//pBufferW = NULL;// 清除图片，不清除背景数据
	//pBufferH = NULL;
	//bHasPic = false; // CalcBindState() 中计算 
	//bNeedRenew = false;

	zoom = 1.0;
	lastZoom = zoom;
	rotate = 0.0f;
	lastRot = rotate;

	zoomW = 0;
	zoomH = 0;
	zoomXActual = 1.0;
	zoomYActual = 1.0;

	ZEROPOINT(ptSurfBase);
	ZEROPOINT(ptSurfOffset);
	ZEROPOINT(surfSize);
	ASIGNPOINT(lastSurfSize, surfSize);

	ZEROPOINT(ptZoomBase);
	ptScaleX = 0.5;
	ptScaleY = 0.5;

	QueryPerformanceFrequency(&freq);
	wheeltick.QuadPart = 0;
	lastwheeltick.QuadPart = 0;

	bClip = true;
	bPicClipped = false;
	bOutClient = false;

	CalcBindState();
}

void SurferBase::Clear()
{
	ClearPic();

	pBufferW = NULL;
	pBufferH = NULL;
}

void SurferBase::SetClip(bool clip)
{
	if (bClip != clip)
	{
		bClip = clip;

		OnSurfPosChange();// clip 改变，会导致 offset、surfSize 改变
	}
}

void SurferBase::SetBasePoint(POINT ptB)
{
	if (bHasPic)
	{
		ptZoomBase = ptB;

		POINTi64 surf2cursor;// 鼠标相对于 surface 起点偏移
		surf2cursor.x = -ptSurfBase.x + ptZoomBase.x;
		surf2cursor.y = -ptSurfBase.y + ptZoomBase.y;

		//  这里不直接用 zoomW、zoomH ，为了精度考虑 ( 中点缩放不需要 )
		float angleInnerV = atan((float)pBmp->width / pBmp->height);
		float angleL = angleInnerV + rotate;
		float angleR = angleInnerV - rotate;
		double diagLen = sqrt((double)SQ(pBmp->width) + (double)SQ(pBmp->height));
		double zoomWF, zoomHF;// zoomW、zoomH 精确值
		/*if (rotate == 0)
		{
		zoomWF = zoom*pBmp->width;
		zoomHF = zoom*pBmp->height;
		}
		else
		{*/
		zoomWF = zoom*diagLen*max(abs(sin(angleL)), abs(sin(angleR)));
		zoomHF = zoom*diagLen*max(abs(cos(angleL)), abs(cos(angleR)));
		//}

		if (zoomW != 0)// 用整数版本值对比 0，防止分母太大
			ptScaleX = surf2cursor.x / (double)zoomWF;
		else
			ptScaleX = 0;
		if (zoomH != 0)
			ptScaleY = surf2cursor.y / (double)zoomHF;
		else
			ptScaleY = 0;
	}
}

void SurferBase::SurfHomePR()
{
	ptSurfBase.x = 0;
	ptSurfBase.y = 0;

	OnSurfPosChange();
}

void SurferBase::SurfLocatePR(POINTi64 base)
{
	ptSurfBase.x = base.x;
	ptSurfBase.y = base.y;

	OnSurfPosChange();
}

void SurferBase::SurfCenterPR(int bufferW, int bufferH)
{
	ptSurfBase.x = (INT64)(bufferW / 2.0 - zoomW / 2.0);
	ptSurfBase.y = (INT64)(bufferH / 2.0 - zoomH / 2.0);

	OnSurfPosChange();
}

void SurferBase::SurfMovePR(INT64 dx, INT64 dy)
{
	ptSurfBase.x += dx;
	ptSurfBase.y += dy;

	OnSurfPosChange();
}

void SurferBase::SurfSetZoomPR(double zoom, bool bPosRefresh, bool stable)
{
	this->zoom = zoom;

	PostZoomChange(bPosRefresh, stable);
}

void SurferBase::SurfSetRotatePR(float rotate, bool bPosRefresh)
{
	this->rotate = rotate;

	PostRotateChange(bPosRefresh);
}

void SurferBase::SurfAdjustZoom_DragPR(int wParam, bool bPosRefresh, bool stable)
{
	double dZoom = 0;// zoom 变化量

	// 放大缩小速度平衡
	double coef = wParam*ZOOMFACTOR_DRAG;
	if (coef >= 0)
	{
		dZoom = coef*zoom;
	}
	else
	{
		double zoomNew = zoom / (1.0 - coef);
		dZoom = zoomNew - zoom;
	}

	zoom += dZoom;
	// zoom上下限
	if (bHasPic)
	{
		int maxSize = max(pBmp->width, pBmp->height);
		int minSize = min(pBmp->width, pBmp->height);
		if (INT64_MAX / zoom < maxSize)
			zoom = (double)(INT64_MAX / maxSize - 1);
		if (zoom*minSize < 1)
			zoom = 1.00001 / minSize;//控制surface尺寸>0
	}
	else
	{
		if (zoom > ZOOM_MAX)
			zoom = (double)ZOOM_MAX;
		if (zoom < ZOOM_MIN)
			zoom = ZOOM_MIN;
	}

	PostZoomChange(bPosRefresh, stable);
}

void SurferBase::SurfAdjustZoom_WheelPR(int wParam, bool bPosRefresh, bool stable)
{
	double dZoom = 0;// zoom 变化量

	//变化量相关于时间和当前放大倍率
	QueryPerformanceCounter(&wheeltick);
	double coefTime = freq.QuadPart / (double)(wheeltick.QuadPart - lastwheeltick.QuadPart);
	lastwheeltick.QuadPart = wheeltick.QuadPart;
	//滚轮幅度倍率（正负表示滚轮方向）
	double coefWheel = wParam / 120.0f;
	//放大缩小速度平衡
	double coef = sqrt(coefTime)*coefWheel*ZOOMFACTOR_WHEEL;
	if (coef >= 0)
	{
		dZoom = coef*zoom;
	}
	else
	{
		double zoomNew = zoom / (1.0 - coef);
		dZoom = zoomNew - zoom;
	}

	zoom += dZoom;
	//zoom上下限
	if (bHasPic)
	{
		int maxSize = max(pBmp->width, pBmp->height);
		int minSize = min(pBmp->width, pBmp->height);
		if (INT64_MAX / zoom < maxSize)
			zoom = (double)(INT64_MAX / maxSize - 1);
		if (zoom*minSize < 1)
			zoom = 1.00001 / minSize;//控制surface尺寸>0
	}
	else
	{
		if (zoom > ZOOM_MAX)
			zoom = (double)ZOOM_MAX;
		if (zoom < ZOOM_MIN)
			zoom = ZOOM_MIN;
	}

	PostZoomChange(bPosRefresh, stable);
}

void SurferBase::SurfAdjustRotate_DragPR(int wParam, bool bPosRefresh)
{
	float dRotate = (float)(ROTATEFACTOR_DRAG*wParam);// rotate 变化量

	// 最小变化值
	/*if (wParam > 0 && dRotate < ROTATE_MINDELTA)
	dRotate = ROTATE_MINDELTA;
	else if (wParam < 0 && dRotate > -ROTATE_MINDELTA)
	dRotate = -ROTATE_MINDELTA;*/

	rotate += dRotate;
	// rotate上下限
	if (rotate >= 2 * PI)
	{
		rotate -= (float)(2 * PI);
	}
	else if (rotate < 0)
	{
		rotate += (float)(2 * PI);
	}

	PostRotateChange(bPosRefresh);
}

void SurferBase::PostZoomChange(bool bPosRefresh, bool stable)
{
	OnZoomChange();

	if (bHasPic)
	{
		if (bPosRefresh && lastZoom != zoom)
		{
			//  这里不直接用 zoomW、zoomH ，为了精度考虑 ( 中点缩放不需要 )
			float angleInnerV = atan((float)pBmp->width / pBmp->height);
			float angleL = angleInnerV + rotate;
			float angleR = angleInnerV - rotate;
			double diagLen = sqrt((double)SQ(pBmp->width) + (double)SQ(pBmp->height));
			double zoomWF, zoomHF;// zoomW、zoomH 精确值
			/*if (rotate == 0)
			{
			zoomWF = zoom*pBmp->width;
			zoomHF = zoom*pBmp->height;
			}
			else
			{*/
			zoomWF = zoom*diagLen*max(abs(sin(angleL)), abs(sin(angleR)));
			zoomHF = zoom*diagLen*max(abs(cos(angleL)), abs(cos(angleR)));
			//}

			// #1 调整起始点
			if (stable)
			{
				// 采用整数值 zoomWH 为了保持中点不变性
				INT64 lastZW = 0, lastZH = 0;
				GetZoomSize(lastZoom, lastRot, &lastZW, &lastZH);
				ptSurfBase.x = (ptSurfBase.x + (lastZW / 2 - zoomW / 2));
				ptSurfBase.y = (ptSurfBase.y + (lastZH / 2 - zoomH / 2));
				SetBasePoint(ptZoomBase);// 更新缩放基准点，使缩放效果自然
			}
			else
			{
				ptSurfBase.x = (INT64)(ptZoomBase.x - zoomWF*ptScaleX);
				ptSurfBase.y = (INT64)(ptZoomBase.y - zoomHF*ptScaleY);
			}

			// 如果在放大阶段，窗口客户区可以完全容纳 surface，则控制 surface 显示区域
			//  防止小图片放大迅速偏离窗口客户区
			if (pBufferW != NULL &&
				pBufferH != NULL &&
				zoom > lastZoom &&
				zoomW < (INT64)*pBufferW*ZOOM_MAXCONTROLED &&
				zoomH < (INT64)*pBufferH*ZOOM_MAXCONTROLED &&
				zoomW <= (INT64)*pBufferW &&
				zoomH <= (INT64)*pBufferH )
			{
				bool bAdjust = false;
				if (ptSurfBase.x < 0)
				{
					bAdjust = true;
					ptSurfBase.x = 0;
				}
				if (ptSurfBase.y < 0)
				{
					bAdjust = true;
					ptSurfBase.y = 0;
				}
				if (ptSurfBase.x + zoomW > (INT64)*pBufferW)
				{
					bAdjust = true;
					ptSurfBase.x = (INT64)*pBufferW - zoomW;
				}
				if (ptSurfBase.y + zoomH > (INT64)*pBufferH)
				{
					bAdjust = true;
					ptSurfBase.y = (INT64)*pBufferH - zoomH;
				}

				if (bAdjust)
					SetBasePoint(ptZoomBase);// 更新缩放基准点，使缩放效果自然
			}
		}
	}

	// #2 更新标志、变量
	// (zoomW > MIN_FORCECLIP_WIDTH || zoomH > MIN_FORCECLIP_HEIGHT);
	SetClip(true);
	lastZoom = zoom;

	OnSurfPosChange();
}

void SurferBase::PostRotateChange(bool bPosRefresh)
{
	OnZoomChange();

	if (bHasPic)
	{
		if (bPosRefresh && lastRot != rotate)
		{
			// #1 调整起始点
			INT64 lastZW = 0, lastZH = 0;
			GetZoomSize(zoom, lastRot, &lastZW, &lastZH);

			// 采用整数值 zoomWH 为了保持中点不变性
			ptSurfBase.x = (ptSurfBase.x + lastZW / 2 - zoomW / 2);
			ptSurfBase.y = (ptSurfBase.y + lastZH / 2 - zoomH / 2);
		}
	}
	// #2 更新标志、变量
	SetClip(true);
	lastRot = rotate;

	OnSurfPosChange();
}

void SurferBase::CalcBindState()
{
	bHasPic = (pBmp != NULL)/* && pBmpW && pBmpH*/;
}

void Surfer::CalcBindState()
{
	SurferBase::CalcBindState();

	bHasDevice = (pDevice != NULL) && (pBufferW != NULL) && (pBufferH != NULL);
	bHasBoth = bHasDevice && bHasPic;
}

inline void	Surfer::CalcMapInfo()
{
	if (bHasDevice)
	{
		// 计算 ptSurfDest
		if (ptSurfBase.x < 0)
			ptSurfDest.x = 0;
		else
			ptSurfDest.x = (LONG)ptSurfBase.x;
		if (ptSurfBase.y < 0)
			ptSurfDest.y = 0;
		else
			ptSurfDest.y = (LONG)ptSurfBase.y;

		// 计算 rcSurfSrc
		if (bClip)
		{
			rcSurfSrc.left = 0;
			rcSurfSrc.top = 0;
			rcSurfSrc.right = surfSize.x;
			rcSurfSrc.bottom = surfSize.y;
		}
		else
		{
			rcSurfSrc.left = (LONG)(max(-ptSurfBase.x, 0));
			rcSurfSrc.top = (LONG)(max(-ptSurfBase.y, 0));
			rcSurfSrc.right = (LONG)(min(zoomW, rcSurfSrc.left + (int)*pBufferW - ptSurfDest.x));// surf 右下区域不够窗口客户区，则削减右侧和下侧
			rcSurfSrc.bottom = (LONG)(min(zoomH, rcSurfSrc.top + (int)*pBufferH - ptSurfDest.y));
		}
	}
}

Surfer::Surfer() {
	Clear();
	
	// alphablend表初始化
	ALPHABLEND::InitAlphBlendTable(TRANSPARENTBACK_FILLDENSITY, TRANSPARENTBACK_HOLLOWDENSITY);
}

Surfer::~Surfer()
{
	SAFE_RELEASE(surf);
	SAFE_RELEASE(pDevice);
}

void Surfer::Clear()
{
	ClearPic();

	pDevice = NULL;
	//bHasDevice = false;

	CalcBindState();
	UpdateInfo();
}

void Surfer::ClearTimeInfo()
{
	count = 0;
	time = 0;
	timeAvg = 0;
	timeMin = 0;
	timeMax = 0;
}

void Surfer::ClearPic()
{
	SurferBase::ClearPic();

	SAFE_RELEASE(surf);
	//bHasBoth = false;

	ZEROPOINT(ptSurfDest);
	ZERORECT(rcSurfSrc);

	bSurfFailed = true;
	bSurfClipped = false;

	ZEROPOINT(ptCursorPixel);
	cursorColor = 0;
	cursorPos = CURSORPOS_BLANK;//更新标志

	time = 0;
	count = 0;

	renewTime = 0;
	timeAvg = 0;
	timeMin = 0;
	timeMax = 0;
	sampleMethod = SAMPLE_UNKNOWN;

	CalcBindState();
	UpdateInfo();// 更新信息字符串
}

bool Surfer::BindDevice(LPDIRECT3DDEVICE9 dev)
{
	SAFE_RELEASE(pDevice);//解除引用
	pDevice = dev;
	if(pDevice)
		pDevice->AddRef();//增加引用计数

	//LPDIRECT3DSURFACE9 pSurf;
	//pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_LEFT, &pSurf);
	CalcBindState();

	return pDevice != NULL;
}

bool Surfer::BindBuf(UINT * pBufW, UINT * pBufH)
{
	pBufferW = pBufW;
	pBufferH = pBufH;

	CalcBindState();

	return pBufferW && pBufferH;
}

bool Surfer::BindPic(PicPack * pPack, bool renew)
{
	if (pPack)
	{
		// 捆绑指针
		pBmp = pPack->GetPBMP();
		/*pBmpW = &pBmp->width;
		pBmpH = &pBmp->height;*/

		// 更新捆绑状态
		CalcBindState();

		// 信息导入
		const PIC_LIVEINFO &liveInfo = pPack->GetLiveInfo();
		SurfLocatePR(liveInfo.src);
		SurfSetZoomPR(liveInfo.zoom);
		SurfSetRotatePR(liveInfo.rotate);

		if (renew)
			SurfRenew(true);

		UpdateInfo();// 更新信息字符串
	}
	else
	{
		ClearPic();
	}

	return pBmp != NULL;
}

bool Surfer::DeBindPic(PicPack *pPack)
{
	if (pPack)
	{
		PIC_LIVEINFO liveInfo;
		liveInfo.rotate = rotate;
		liveInfo.src = ptSurfBase;
		liveInfo.zoom = zoom;
		pPack->SetLiveInfo(liveInfo);

		//CalcBindState();

		return true;
	}
	else
		return false;
}

void Surfer::SetBasePoint(POINT ptB)
{
	if (/*!bSurfFailed*/true)
	{
		SurferBase::SetBasePoint(ptB);
	}
}

bool Surfer::OnDrag_Custom(POINTi64 offSet, bool bAcce)
{
	if (bHasBoth)
	{
		bool oldPicClipped = bPicClipped;
		bool oldOutClient = bOutClient;

		SurfMovePR(offSet.x, offSet.y);

		bool bRenewed = false;
		if (bClip)
		{
			// 移动时如果有 bClip 标志，并且图片小于一定尺寸，则取消 clip。
			// 如果先前 surface 被 clip，就重新生成 surface，增加拖动速度。
			if (zoomW <= MAX_NOCLIPSIZE_DRAG && zoomH <= MAX_NOCLIPSIZE_DRAG)
			{
				SetClip(false);
				if (bSurfClipped)// 这里用 bPicClipped 应该也一样
				{
					bRenewed = SurfRenew(bAcce);
				}
			}
			// 如果图片始终全部在窗口客户区范围内未被 clip ( 移动前后 bPicClipped 状态都是 0 )
			// 或者始终在客户区外 （ 前后 bOutClient 状态都是 1 )，就不更新
			else if ((oldPicClipped || bPicClipped) && (!oldOutClient || !bOutClient))
			{
				bRenewed = SurfRenew(bAcce);
			}
		}

		// 本次没更新 surface，则更新 surface 映射信息，否则导致 surface 不动
		if (!bRenewed)
		{
			CalcMapInfo();
		}

		return bRenewed;
	}
	else
		return false;
}

bool Surfer::OnWinsize_Custom()
{
	if (bHasBoth)
	{
		bool oldPicClipped = bPicClipped;
		bool oldOutClient = bOutClient;

		OnSurfPosChange();

		// clip 状态更新、surface 更新
		bool bRenewed = false;
		if (bClip)
		{
			// 移动时如果有 bClip 标志，并且图片小于一定尺寸，则取消 clip。
			// 如果先前 surface 被 clip，就重新生成 surface，增加拖动速度。
			if (zoomW <= MIN_FORCECLIPSIZE && zoomH <= MIN_FORCECLIPSIZE)
			{
				SetClip(false);
				if (bSurfClipped)
				{
					bRenewed = SurfRenew(true);
				}
			}
			// 如果图片始终全部在窗口客户区范围内 ( 移动前后 bSurfClipped 状态都是 0 )
			// 或者始终在客户区外 ( 前后 bOutClient 状态都是 1 )，或者不设置 bClip 标志，就不更新
			else if ((oldPicClipped || bPicClipped) && (!oldOutClient || !bOutClient))
			{
				bRenewed = SurfRenew(true);
			}
		}
		// 本次没更新 surface，则更新 surface 映射信息
		if (!bRenewed)
		{
			CalcMapInfo();// 更新 surface 拷贝到 backbuffer 参数
		}

		return bRenewed;
	}
	else
		return false;
}

bool Surfer::SurfSuit(int w, int h)
{
	if (bHasBoth)
	{
		if (w < 1 || h < 1)
			return false;

		if (zoom < 0)
			return false;

		// #1 调整起始点
		SurfMovePR(-ptSurfBase.x, -ptSurfBase.y);

		// #2 调整 zoom 值
		SurfSetZoomPR(min((double)w *zoom / zoomW, (double)h *zoom / zoomH));

		// #3 更新 surface
		return SurfRenew(false);
	}
	else
		return false;
}

void Surfer::UpdateInfo()
{
	WCHAR subinfo[256] = { 0 };
	strSurfInfo[0] = L'\0';

	// buffer 尺寸、surface 尺寸
	//  缩放倍率
	//  surface 起始点
	StringCchPrintf(subinfo, 256, L"SURFACE: %d × %d     RENEWTIME: %.3fms (%S)\n\
		ZOOM: %.3e\n\
		ROTATE: %s\n\
		BASE: %lld, %lld\n\
		SCALE: %.3lf, %.3lf\n\
		time: %.3lf/ %.3lf[%.3lf, %.3lf] ms\n"
		, surfSize.x, surfSize.y, renewTime*1000.0f, GetSampleSchemaStr(sampleMethod)
		, zoom
		, GetDegreeOfRadian(rotate)
		, ptSurfBase.x, ptSurfBase.y
		, ptScaleX, ptScaleY
		, 1000.0*time, 1000.0*timeAvg, 1000.0*timeMin, 1000.0*timeMax);
	wcscat_s(strSurfInfo, subinfo);

	// 鼠标像素位置
	if (pBmp)
	{
		if (pBmp->isNotEmpty())
			StringCchPrintf(subinfo, 256, L"PIXEL: %d, %d\n"
				, ptCursorPixel.x, ptCursorPixel.y);
		else
			StringCchPrintf(subinfo, 256, L"PIXEL:-, -\n");
	}
	else
		StringCchPrintf(subinfo, 256, L"PIXEL:-, -\n");
	wcscat_s(strSurfInfo, subinfo);

	// 鼠标像素颜色、屏幕像素颜色、背景色
	COLOR4f hsvc = RGB2HSV_F3(cursorColor);
	if (cursorPos == CURSORPOS_PIC)
		StringCchPrintf(subinfo, 256, L"COLOR: #%02X.%06X.ARGB\n\
			              %d, %d, %d, %d.ARGB\n\
			              %.1f, %.2f, %.0f.HSV\n"
			, (cursorColor >> 24), (cursorColor & 0xFFFFFF)
			, (cursorColor >> 24), (cursorColor >> 16) & 0xFF
			 , (cursorColor >> 8) & 0xFF, cursorColor & 0xFF
			, hsvc.r, hsvc.g, hsvc.b);
	else
		StringCchPrintf(subinfo, 256, L"COLOR: ??.??????.ARGB\n");
	wcscat_s(strSurfInfo, subinfo);

	// 附加信息
	//"intended surface: %d× %d\n
	//	clipsurface base: %d, %d\n
	//	PZ: X %.4f Y %.4f\n"
	//	, zoomW, zoomH
	//	, ptSurfDest.x, ptSurfDest.y
	//	, zoomXActual, zoomYActual
}

void Surfer::GetCurInfo(POINT *cursor)
{
	if (bHasBoth)
	{
		if (cursor == NULL)
			return;

		// 鼠标相对 surface 起点的偏移
		POINTi64 surf2cursor;
		surf2cursor.x = (INT64)cursor->x - ptSurfBase.x;
		surf2cursor.y = (INT64)cursor->y - ptSurfBase.y;
		// 获得当前鼠标位置
		if (cursor->x < 0 || cursor->x >(LONG)*pBufferW
			|| cursor->y < 0 || cursor->y >(LONG)*pBufferH)
			cursorPos = CURSORPOS_OUTWINDOW;
		else if (bSurfFailed)
			cursorPos = CURSORPOS_BLANK;
		//else if (OUTSIDE2(surf2cursor, zoomW - 1, zoomH - 1))// todo是否需要
		//	cursorPos = CURSORPOS_BLANK;
		else
		{
			if (rotate == 0)
			{
				// 所处像素
				ptCursorPixel.x = (LONG)((surf2cursor.x + 0.5) *(AUXTYPE)pBmp->width / zoomW);
				ptCursorPixel.y = (LONG)((surf2cursor.y + 0.5) *(AUXTYPE)pBmp->height / zoomH);
			}
			else//rotate != 0
			{
				AUXTYPE tmpCosR = (AUXTYPE)cos(-rotate);
				AUXTYPE tmpSinR = (AUXTYPE)sin(-rotate);
				AUXTYPE halfw = pBmp->width / (AUXTYPE)2.0;
				AUXTYPE halfh = pBmp->height / (AUXTYPE)2.0;
				AUXTYPE tmpZWOff = 0 + (AUXTYPE)0.5 - zoomW / (AUXTYPE)2.0;
				AUXTYPE tmpZHOff = zoomH / (AUXTYPE)2.0 - 0 - (AUXTYPE)0.5;

				AUXTYPE midX = (surf2cursor.x + tmpZWOff) / zoom;
				AUXTYPE midY = (tmpZHOff - surf2cursor.y) / zoom;
				AUXTYPE pixelPosXcos = midX*tmpCosR;
				AUXTYPE pixelPosXsin = midX*tmpSinR;
				AUXTYPE sinYtmp = midY*tmpSinR + halfw;
				AUXTYPE cosYtmp = midY*tmpCosR - halfh;

				if (pixelPosXcos + sinYtmp >= 0)
					ptCursorPixel.x = (int)(pixelPosXcos + sinYtmp);
				else
					ptCursorPixel.x = (int)(pixelPosXcos + sinYtmp) - 1;
				if (-cosYtmp + pixelPosXsin >= 0)
					ptCursorPixel.y = (int)(-cosYtmp + pixelPosXsin);
				else
					ptCursorPixel.y = (int)(-cosYtmp + pixelPosXsin) - 1;
			}

			if (!OUTSIDE2(ptCursorPixel, pBmp->width - 1, pBmp->height - 1))
			{
				cursorPos = CURSORPOS_PIC;
				// 获取所处像素颜色
				pBmp->GetPixel(ptCursorPixel.x, ptCursorPixel.y, &cursorColor);
			}
			else
				cursorPos = CURSORPOS_BLANK;

		}
	}

	// 更新信息字符串
	UpdateInfo();
}