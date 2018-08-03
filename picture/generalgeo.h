#pragma once

#include <windows.h>
#include <math.h>
#include <strsafe.h>
#include <iostream>
#include <iomanip>

using std::ostream;
using std::istream;
using std::setw;
using std::setprecision;

// GENERAL
#define DBL_INF						(1 / 0.0L)

#define PI_F						3.14159265f
#define PI							3.1415926535897932
#define R2D(R)						((R) * 180.0 / PI)
#define R2D_F(R)					((R) * 180.0f / PI_F)
#define XNOR(B1, B2)				!((B1) ^ (B2))

#define SQ(N)						((N)*(N))

#define ROUND(f)					(int)(f + 0.5)
#define ROUNDF(f)					(int)(f + 0.5f)
#define ROUND_LONG(f)				(LONG)(f + 0.5)
#define ROUND_LONGLONG(f)			(LONGLONG)(f + 0.5)
#define ROUND_BYTE(f)				(byte)(f + 0.5)
#define ROUNDF_BYTE(f)				(byte)(f + 0.5f)

#define MAKE_RANDOM(SEED, LL, UL)	((LL) == (UL)? LL : ((LL) > (UL)? MAKE_RANDOM_STRICT(SEED, UL, LL) : MAKE_RANDOM_STRICT(SEED, LL, UL)))	
#define MAKE_RANDOM_STRICT(SEED, LL, UL)	(((SEED) % (UL - LL + 1)) - (UL - LL)/2)

//RECT
#define WIDTHOF(R)					((R).right - (R).left)
#define HEIGHTOF(R)					((R).bottom - (R).top)

#define MIDDLEXOF(R)				(((R).left + (R).right)/2)
#define MIDDLEYOF(R)				(((R).top + (R).bottom)/2)

#define RECT(X, Y, W, H)			{X, Y, X + W, Y + H}
#define RECT_LB(X, Y, W, H)			{X, Y - H, X + W, Y}

#define ZERORECT(R)					{(R).left = 0; (R).top = 0; (R).right = 0; (R).bottom = 0;}


//POINT
#define ADDPOINT(P1, P2)			{(P1).x + (P2).x, (P1).y + (P2).y}
#define MINUSPOINT(P1, P2)			{(P1).x - (P2).x, (P1).y - (P2).y}
#define ASIGNPOINT(P1, P2)			{(P1).x = (P2).x; (P1).y = (P2).y;}

#define INSIDE(P, R)				((P).x >= (R).left && (P).x <= (R).right && (P).y >= (R).top && (P).y <= (R).bottom)
#define OUTSIDE(P, R)				((P).x < (R).left || (P).x > (R).right || (P).y < (R).top || (P).y > (R).bottom)

#define INSIDE2(P, W, H)			((P).x >= 0 && (P).y >= 0 && (P).x <= (W) && (P).y <= (H))
#define OUTSIDE2(P, W, H)			((P).x < 0 || (P).y < 0 || (P).x > (W) || (P).y > (H))

#define GET_X_LPARAM(lp)			((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)			((int)(short)HIWORD(lp)) 

#define ZEROPOINT(P)				{(P).x = 0; (P).y = 0;}
#define SETPOINT(P, X, Y)			{(P).x = X; (P).y = Y;}
#define OPPOSITEPOINT(P)			{-(P).x, -(P).y}

// 三阶向量运算
#define IS_VEC3_ZERO(V)				((V).x == 0 && (V).y == 0 && (V).z == 0)	// 判断向量全0
#define IS_PVEC3_ZERO(V)			((V)->x == 0 && (V)->y == 0 && (V)->z == 0)
#define VEC3_LEN(V)					sqrtf((FLOAT)(V).x*(V).x + (FLOAT)(V).y*(V).y + (FLOAT)(V).z*(V).z)		// 向量模
#define PVEC3_LEN(PV)				sqrtf((FLOAT)(PV)->x*(PV)->x + (FLOAT)(PV)->y*(PV)->y + (FLOAT)(PV)->z*(PV)->z)
#define VEC3_LEN_LF(V)				sqrt((double)(V).x*(V).x + (double)(V).y*(V).y + (double)(V).z*(V).z)
#define PVEC3_LEN_LF(PV)			sqrt((double)(PV)->x*(PV)->x + (double)(PV)->y*(PV)->y + (double)(PV)->z*(PV)->z)
#define VEC3_LENSQ(V)				((V).x*(V).x + (V).y*(V).y + (V).z*(V).z)			// 向量模平方
#define PVEC3_LENSQ(PV)				((PV)->x*(PV)->x + (PV)->y*(PV)->y + (PV)->z*(PV)->z)
#define VEC3_DOT(V1, V2)			((V1).x*(V2).x + (V1).y*(V2).y + (V1).z*(V2).z)		// 向量点积
#define PVEC3_DOT(PV1, PV2)			((PV1)->x*(PV2)->x + (PV1)->y*(PV2)->y + (PV1)->z*(PV2)->z)
#define VEC3_DOT_LF(V1, V2)			((double)(V1).x*(V2).x + (double)(V1).y*(V2).y + (double)(V1).z*(V2).z)	
#define PVEC3_DOT_LF(PV1, PV2)		((double)(PV1)->x*(PV2)->x + (double)(PV1)->y*(PV2)->y + (double)(PV1)->z*(PV2)->z)	
#define VEC3_PROJLEN(VS, VD)		(VEC3_DOT(VS, VD) / VEC3_LEN(VD))					// 向量到向量的投影长度
#define PVEC3_PROJLEN(PVS, PVD)		(PVEC3_DOT(PVS, PVD) / PVEC3_LEN(PVD))
#define VEC3_PROJ2PLANE(VS, VD)		()
#define VEC3_ANGLE(V1, V2)			acosf(VEC3_DOT(V1, V2) / VEC3_LEN(V1) / VEC3_LEN(V2))//向量夹角
#define PVEC3_ANGLE(PV1, PV2)		acosf(PVEC3_DOT(PV1, PV2) / PVEC3_LEN(PV1) / PVEC3_LEN(PV2))
#define VEC3_ANGLE_LF(V1, V2)		acos(VEC3_DOT_LF(V1, V2) / VEC3_LEN_LF(V1) / VEC3_LEN_LF(V2))
#define PVEC3_ANGLE_LF(PV1, PV2)	acos(PVEC3_DOT_LF(PV1, PV2) / PVEC3_LEN_LF(PV1) / PVEC3_LEN_LF(PV2))


typedef struct POINTi64 {
	INT64 x;
	INT64 y;
	POINTi64()
	{
		x = 0;
		y = 0;
	}
	POINTi64(INT64 a, INT64 b)
	{
		x = a;
		y = b;
	}
}POINTi64, *LPPOINTi64;

// 球坐标系中表示向量的两个角
struct SPHERICAL_ANGLE {
	float azimuth;	// 方位角
	float zenith;	// 天顶角
};

SPHERICAL_ANGLE GetSphericalAngle(short x, short y, short z);

const WCHAR *GetDegreeOfRadian(double angle);

inline bool isDecimal2(float f)
{
	INT8 index = (short)(((*((DWORD*)&f)) >> 23) & 0xFF) - 127;//*((DWORD*)&f)-base
	if (index > 0)
		return ((*((DWORD*)&f)) & (0x7FFFFF >> index)) != 0;
	else
		return ((*((DWORD*)&f)) & 0x7FFFFF) != 0;
}

typedef struct VECTOR3f
{
	FLOAT x;
	FLOAT y;
	FLOAT z;
public:
	VECTOR3f() {};
	VECTOR3f(const VECTOR3f&);
	VECTOR3f(FLOAT x, FLOAT y, FLOAT z);

	void Output(ostream &);

	// casting
	//operator FLOAT* ();
	//operator const FLOAT* () const;

	// assignment operators
	VECTOR3f& operator = (const VECTOR3f&);
	VECTOR3f& operator += (const VECTOR3f&);
	VECTOR3f& operator -= (const VECTOR3f&);
	VECTOR3f& operator *= (FLOAT);
	VECTOR3f& operator /= (FLOAT);

	// unary operators
	VECTOR3f operator + () const;
	VECTOR3f operator - () const;

	// binary operators
	VECTOR3f operator + (const VECTOR3f&) const;
	VECTOR3f operator - (const VECTOR3f&) const;
	VECTOR3f operator * (FLOAT) const;
	VECTOR3f operator / (FLOAT) const;

	friend VECTOR3f operator * (FLOAT, const struct VECTOR3f&);

	// relation operators
	BOOL operator == (const VECTOR3f&) const;
	BOOL operator != (const VECTOR3f&) const;

	// stream operators
	friend ostream& operator << (ostream&, const VECTOR3f&);
	friend istream& operator >> (istream&, VECTOR3f&);

	// 其他运算，和宏定义功能相同
	FLOAT Length() const;
	FLOAT LengthSq() const;

} VECTOR3f, *LPVECTOR3f;

// 对 binary operators 和其他成员函数的加速
inline VECTOR3f *Vec3Add(VECTOR3f *pOut, const VECTOR3f *pV1, const VECTOR3f *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;

	return pOut;
}
inline VECTOR3f *Vec3Subtract(VECTOR3f *pOut, const VECTOR3f *pV1, const VECTOR3f *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;

	return pOut;
}
inline VECTOR3f *Vec3Multiply(VECTOR3f *pOut, const VECTOR3f *pV1, FLOAT f)
{
	if (!pOut || !pV1)
		return NULL;

	pOut->x = pV1->x * f;
	pOut->y = pV1->y * f;
	pOut->z = pV1->z * f;

	return pOut;
}
inline VECTOR3f *Vec3Divide(VECTOR3f *pOut, const VECTOR3f *pV1, FLOAT f)
{
	if (!pOut || !pV1 || !f)
		return NULL;

	pOut->x = pV1->x / f;
	pOut->y = pV1->y / f;
	pOut->z = pV1->z / f;

	return pOut;
}
inline FLOAT Vec3Dot(const VECTOR3f *pV1, const VECTOR3f *pV2)
{
	if (!pV1 || !pV2)
		return 0.0f;

	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}
inline VECTOR3f *Vec3Cross(VECTOR3f *pOut, const VECTOR3f *pV1, const VECTOR3f *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	if (pOut == pV1 || pOut == pV2)
	{
		float a, b;
		a = pV1->y*pV2->z - pV1->z*pV2->y;
		b = pV1->z*pV2->x - pV1->x*pV2->z;
		pOut->z = pV1->x*pV2->y - pV1->y*pV2->x;
		pOut->x = a;
		pOut->y = b;
	}
	else
	{
		pOut->x = pV1->y*pV2->z - pV1->z*pV2->y;
		pOut->y = pV1->z*pV2->x - pV1->x*pV2->z;
		pOut->z = pV1->x*pV2->y - pV1->y*pV2->x;
	}

	return pOut;
}

inline bool Vec3Orthogonal(const VECTOR3f *pV1, const VECTOR3f *pV2) {
	if (!pV1 || !pV2)
		return false;

	return (PVEC3_DOT(pV1, pV2) == 0);
}
inline bool Vec3Orthogonal(const VECTOR3f *pV1, const VECTOR3f *pV2, double tolerance) {
	if (!pV1 || !pV2)
		return false;

	if (tolerance < 0)
		tolerance = -tolerance;
	double angle = PVEC3_ANGLE_LF(pV1, pV2);
	return (angle >= PI / 2 - tolerance
		&& angle <= PI / 2 + tolerance);
}
inline bool Vec3Parallel(const VECTOR3f *pV1, const VECTOR3f *pV2) {
	if (!pV1 || !pV2)
		return false;

	if (pV1->x != 0 && pV1->y != 0 && pV1->z)
	{
		float judge = pV2->x / pV1->x;
		if (judge == pV2->y / pV1->y && judge == pV2->z / pV1->z)
			return true;
		else
			return false;
	}
	else
	{
		if (XNOR(pV1->x == 0, pV2->x == 0)
			&& XNOR(pV1->y == 0, pV2->y == 0)
			&& XNOR(pV1->z == 0, pV2->z == 0))
			return true;
		else
			return false;
	}
}
inline bool Vec3Parallel(const VECTOR3f *pV1, const VECTOR3f *pV2, double tolerance) {
	if (!pV1 || !pV2)
		return false;

	if (tolerance < 0)
		tolerance = -tolerance;
	double angle = PVEC3_ANGLE_LF(pV1, pV2);
	return (angle >= 0.0 - tolerance
		&& angle <= 0.0 + tolerance
		|| angle >= PI - tolerance
		&& angle <= PI + tolerance);
}

inline VECTOR3f *Vec3Normalize(VECTOR3f *pOut, const VECTOR3f *pV1)
{
	if (!pOut || !pV1)
		return NULL;

	FLOAT len = PVEC3_LEN(pV1);
	if (len != 0.0f)
	{
		pOut->x = pV1->x / len;
		pOut->y = pV1->y / len;
		pOut->z = pV1->z / len;
	}
	else
		;

	return pOut;
}
inline VECTOR3f *Vec3Scale(VECTOR3f *pOut, const VECTOR3f *pV1, FLOAT f)
{
	if (!pOut || !pV1)
		return NULL;

	pOut->x = pV1->x * f;
	pOut->y = pV1->y * f;
	pOut->z = pV1->z * f;

	return pOut;
}

inline VECTOR3f *Vec3Interpolation(VECTOR3f *pOut, const VECTOR3f *pV1, const VECTOR3f *pV2, FLOAT f)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	pOut->x = pV1->x*(1 - f) + pV2->x*f;
	pOut->y = pV1->y*(1 - f) + pV2->y*f;
	pOut->z = pV1->z*(1 - f) + pV2->z*f;

	return pOut;
}

// 向量到向量的投影向量
inline VECTOR3f *Vec3Project(VECTOR3f *pOut, const VECTOR3f *pV1, const VECTOR3f *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	if (PVEC3_LENSQ(pV2) == 0.0f)
	{
		// TODO
		pOut->x = 0.0f;
		pOut->y = 0.0f;
		pOut->z = 0.0f;
	}
	else
	{
		FLOAT f = PVEC3_PROJLEN(pV1, pV2) / PVEC3_LEN(pV2);

		pOut->x = pV2->x * f;
		pOut->y = pV2->y * f;
		pOut->z = pV2->z * f;
	}

	return pOut;
}

// 向量到平面的投影向量(对向量投影的三角形第三边)
inline VECTOR3f *Vec3Proj2Plane(VECTOR3f *pOut, const VECTOR3f *pV1, const VECTOR3f *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	if (PVEC3_LENSQ(pV2) == 0.0f)
	{
		// TODO
		pOut->x = pV1->x;
		pOut->y = pV1->y;
		pOut->z = pV1->z;
	}
	else
	{
		FLOAT f = PVEC3_PROJLEN(pV1, pV2) / PVEC3_LEN(pV2);

		pOut->x = pV1->x - pV2->x * f;
		pOut->y = pV1->y - pV2->y * f;
		pOut->z = pV1->z - pV2->z * f;
	}

	return pOut;
}
inline VECTOR3f *Vec3Reflect(VECTOR3f *pOut, const VECTOR3f *pVIN, const VECTOR3f *pVNOR)
{
	if (!pOut || !pVIN || !pVNOR)
		return NULL;

	if (PVEC3_LEN(pVNOR) != 0)
	{
		float normaladjust = 2 * PVEC3_PROJLEN(pVIN, pVNOR) / PVEC3_LEN(pVNOR);
		pOut->x = pVNOR->x*normaladjust - pVIN->x;
		pOut->y = pVNOR->y*normaladjust - pVIN->y;
		pOut->z = pVNOR->z*normaladjust - pVIN->z;
	}

	return pOut;
}
inline VECTOR3f *Vec3RotateAxis(VECTOR3f *pOut, const VECTOR3f *pV1, const VECTOR3f *pAxis, FLOAT angle)
{
	if (!pOut || !pV1 || !pAxis)
		return NULL;

	VECTOR3f cross;
	Vec3Cross(&cross, pAxis, pV1);
	/*VECTOR3f result =
	*pV1*cos(angle)
	+ cross*sin(angle)
	+ *pAxis*Vec3Dot(pAxis, pV1)*(1 - cos(angle));*/
	float para3 = Vec3Dot(pAxis, pV1)*(1 - cos(angle));
	pOut->x = pV1->x*cosf(angle) + cross.x*sinf(angle) + pAxis->x*para3;
	pOut->y = pV1->y*cosf(angle) + cross.y*sinf(angle) + pAxis->y*para3;
	pOut->z = pV1->z*cosf(angle) + cross.z*sinf(angle) + pAxis->z*para3;

	return pOut;

	//矩阵法
	/*D3DXMATRIX rot;
	D3DXMatrixRotationAxis(&rot, &axis, angle);*/
}
inline VECTOR3f *Vec3RotateX(VECTOR3f *pOut, const VECTOR3f *pV1, FLOAT angle) {}
inline VECTOR3f *Vec3RotateY(VECTOR3f *pOut, const VECTOR3f *pV1, FLOAT angle) {}
inline VECTOR3f *Vec3RotateZ(VECTOR3f *pOut, const VECTOR3f *pV1, FLOAT angle) {}


typedef struct VECTOR3
{
	int x;
	int y;
	int z;
public:
	VECTOR3() {};
	VECTOR3(const VECTOR3&);
	VECTOR3(int x, int y, int z);

	// casting
	//operator int* ();
	//operator const int* () const;

	// assignment operators
	VECTOR3& operator = (const VECTOR3&);
	VECTOR3& operator += (const VECTOR3&);
	VECTOR3& operator -= (const VECTOR3&);
	VECTOR3& operator *= (int);
	VECTOR3& operator /= (int);

	// unary operators
	VECTOR3 operator + () const;
	VECTOR3 operator - () const;

	// binary operators
	VECTOR3 operator + (const VECTOR3&) const;
	VECTOR3 operator - (const VECTOR3&) const;
	VECTOR3 operator * (int) const;
	VECTOR3 operator / (int) const;

	friend VECTOR3 operator * (int, const struct VECTOR3&);

	BOOL operator == (const VECTOR3&) const;
	BOOL operator != (const VECTOR3&) const;

	FLOAT Length() const;
	FLOAT LengthSq() const;

} VECTOR3, *LPVECTOR3;

inline VECTOR3 *Vec3Add(VECTOR3 *pOut, const VECTOR3 *pV1, const VECTOR3 *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;

	return pOut;
}
inline VECTOR3 *Vec3Subtract(VECTOR3 *pOut, const VECTOR3 *pV1, const VECTOR3 *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;

	return pOut;
}
inline VECTOR3 *Vec3Multiply(VECTOR3 *pOut, const VECTOR3 *pV1, FLOAT f)
{
	if (!pOut || !pV1)
		return NULL;

	pOut->x = (int)(pV1->x * f);
	pOut->y = (int)(pV1->y * f);
	pOut->z = (int)(pV1->z * f);

	return pOut;
}
inline VECTOR3 *Vec3Divide(VECTOR3 *pOut, const VECTOR3 *pV1, FLOAT f)
{
	if (!pOut || !pV1 || !f)
		return NULL;

	pOut->x = (int)(pV1->x / f);
	pOut->y = (int)(pV1->y / f);
	pOut->z = (int)(pV1->z / f);

	return pOut;
}
inline FLOAT Vec3Dot(const VECTOR3 *pV1, const VECTOR3 *pV2)
{
	if (!pV1 || !pV2)
		return 0.0f;

	return (FLOAT)(pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z);
}
inline VECTOR3 *Vec3Cross(VECTOR3 *pOut, const VECTOR3 *pV1, const VECTOR3 *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	if (pOut == pV1 || pOut == pV2)
	{
		int a, b;
		a = pV1->y*pV2->z - pV1->z*pV2->y;
		b = pV1->z*pV2->x - pV1->x*pV2->z;
		pOut->z = pV1->x*pV2->y - pV1->y*pV2->x;
		pOut->x = a;
		pOut->y = b;
	}
	else
	{
		pOut->x = pV1->y*pV2->z - pV1->z*pV2->y;
		pOut->y = pV1->z*pV2->x - pV1->x*pV2->z;
		pOut->z = pV1->x*pV2->y - pV1->y*pV2->x;
	}

	return pOut;
}

inline bool Vec3Orthogonal(const VECTOR3 *pV1, const VECTOR3 *pV2) {
	if (!pV1 || !pV2)
		return false;

	return (PVEC3_DOT(pV1, pV2) == 0);
}
inline bool Vec3Orthogonal(const VECTOR3 *pV1, const VECTOR3 *pV2, double tolerance) {
	if (!pV1 || !pV2)
		return false;

	if (tolerance < 0)
		tolerance = -tolerance;
	double angle = PVEC3_ANGLE_LF(pV1, pV2);
	return (angle >= PI / 2 - tolerance
		&& angle <= PI / 2 + tolerance);
}
inline bool Vec3Parallel(const VECTOR3 *pV1, const VECTOR3 *pV2) {
	if (!pV1 || !pV2)
		return false;

	if (pV1->x != 0 && pV1->y != 0 && pV1->z)
	{
		float judge = pV2->x / (float)pV1->x;
		if (judge == pV2->y / pV1->y && judge == pV2->z / pV1->z)
			return true;
		else
			return false;
	}
	else
	{
		if (XNOR(pV1->x == 0, pV2->x == 0)
			&& XNOR(pV1->y == 0, pV2->y == 0)
			&& XNOR(pV1->z == 0, pV2->z == 0))
			return true;
		else
			return false;
	}
}
inline bool Vec3Parallel(const VECTOR3 *pV1, const VECTOR3 *pV2, double tolerance) {
	if (!pV1 || !pV2)
		return false;

	if (tolerance < 0)
		tolerance = -tolerance;
	double angle = PVEC3_ANGLE_LF(pV1, pV2);
	return (angle >= 0.0 - tolerance
		&& angle <= 0.0 + tolerance
		|| angle >= PI - tolerance
		&& angle <= PI + tolerance);
}

inline VECTOR3 *Vec3Normalize(VECTOR3 *pOut, const VECTOR3 *pV1)
{
	if (!pOut || !pV1)
		return NULL;

	FLOAT len = PVEC3_LEN(pV1);
	if (len != 0.0f)
	{
		pOut->x = (int)(pV1->x / len);
		pOut->y = (int)(pV1->y / len);
		pOut->z = (int)(pV1->z / len);
	}
	else
		;

	return pOut;
}
inline VECTOR3 *Vec3Scale(VECTOR3 *pOut, const VECTOR3 *pV1, FLOAT f)
{
	if (!pOut || !pV1)
		return NULL;

	pOut->x = (int)(pV1->x * f);
	pOut->y = (int)(pV1->y * f);
	pOut->z = (int)(pV1->z * f);

	return pOut;
}
inline VECTOR3 *Vec3Interpolation(VECTOR3 *pOut, const VECTOR3 *pV1, const VECTOR3 *pV2, FLOAT f)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	pOut->x = (int)(pV1->x*(1 - f) + pV2->x*f);
	pOut->y = (int)(pV1->y*(1 - f) + pV2->y*f);
	pOut->z = (int)(pV1->z*(1 - f) + pV2->z*f);

	return pOut;
}
inline VECTOR3 *Vec3Project(VECTOR3 *pOut, const VECTOR3 *pV1, const VECTOR3 *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	if (PVEC3_LENSQ(pV2) == 0)
	{
		//TODO
		pOut->x = 0;
		pOut->y = 0;
		pOut->z = 0;
	}
	else
	{
		FLOAT f = PVEC3_PROJLEN(pV1, pV2) / PVEC3_LEN(pV2);

		pOut->x = (int)(pV2->x * f);
		pOut->y = (int)(pV2->y * f);
		pOut->z = (int)(pV2->z * f);
	}

	return pOut;
}
inline VECTOR3 *Vec3Proj2Plane(VECTOR3 *pOut, const VECTOR3 *pV1, const VECTOR3 *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	if (PVEC3_LENSQ(pV2) == 0.0f)
	{
		//TODO
		pOut->x = pV1->x;
		pOut->y = pV1->y;
		pOut->z = pV1->z;
	}
	else
	{
		FLOAT f = PVEC3_PROJLEN(pV1, pV2) / PVEC3_LEN(pV2);

		pOut->x = ROUNDF(pV1->x - pV2->x * f);
		pOut->y = ROUNDF(pV1->y - pV2->y * f);
		pOut->z = ROUNDF(pV1->z - pV2->z * f);
	}

	return pOut;
}
inline VECTOR3 *Vec3Reflect(VECTOR3 *pOut, const VECTOR3 *pVIN, const VECTOR3 *pVNOR)
{
	if (!pOut || !pVIN || !pVNOR)
		return NULL;

	if (PVEC3_LEN(pVNOR) != 0)
	{
		float normaladjust = 2 * PVEC3_PROJLEN(pVIN, pVNOR) / PVEC3_LEN(pVNOR);
		pOut->x = ROUNDF(pVNOR->x*normaladjust - pVIN->x);
		pOut->y = ROUNDF(pVNOR->y*normaladjust - pVIN->y);
		pOut->z = ROUNDF(pVNOR->z*normaladjust - pVIN->z);
	}

	return pOut;
}
inline VECTOR3 *Vec3RotateAxis(VECTOR3 *pOut, const VECTOR3 *pV1, const VECTOR3 *pAxis, FLOAT angle)
{
	if (!pOut || !pV1 || !pAxis)
		return NULL;

	VECTOR3 cross;
	Vec3Cross(&cross, pAxis, pV1);
	/*VECTOR3 result =
	*pV1*cos(angle)
	+ cross*sin(angle)
	+ *pAxis*Vec3Dot(pAxis, pV1)*(1 - cos(angle));*/
	float para3 = Vec3Dot(pAxis, pV1)*(1 - cos(angle));
	pOut->x = ROUNDF(pV1->x*cosf(angle) + cross.x*sinf(angle) + pAxis->x*para3);
	pOut->y = ROUNDF(pV1->y*cosf(angle) + cross.y*sinf(angle) + pAxis->y*para3);
	pOut->z = ROUNDF(pV1->z*cosf(angle) + cross.z*sinf(angle) + pAxis->z*para3);

	return pOut;

	// 矩阵法
	/*D3DXMATRIX rot;
	D3DXMatrixRotationAxis(&rot, &axis, angle);*/
}
inline VECTOR3 *Vec3RotateX(VECTOR3 *pOut, const VECTOR3 *pV1, FLOAT angle) {}
inline VECTOR3 *Vec3RotateY(VECTOR3 *pOut, const VECTOR3 *pV1, FLOAT angle) {}
inline VECTOR3 *Vec3RotateZ(VECTOR3 *pOut, const VECTOR3 *pV1, FLOAT angle) {}


typedef struct VECTOR3s
{
	short x;
	short y;
	short z;
public:
	VECTOR3s() {};
	VECTOR3s(const VECTOR3s&);
	VECTOR3s(int x, int y, int z);

	// casting
	//operator int* ();
	//operator const int* () const;

	// assignment operators
	VECTOR3s& operator = (const VECTOR3s&);
	VECTOR3s& operator += (const VECTOR3s&);
	VECTOR3s& operator -= (const VECTOR3s&);
	VECTOR3s& operator *= (int);
	VECTOR3s& operator /= (int);

	// unary operators
	VECTOR3s operator + () const;
	VECTOR3s operator - () const;

	// binary operators
	VECTOR3s operator + (const VECTOR3s&) const;
	VECTOR3s operator - (const VECTOR3s&) const;
	VECTOR3s operator * (int) const;
	VECTOR3s operator / (int) const;

	friend VECTOR3s operator * (int, const struct VECTOR3s&);

	BOOL operator == (const VECTOR3s&) const;
	BOOL operator != (const VECTOR3s&) const;

	FLOAT Length() const;
	FLOAT LengthSq() const;

} VECTORs3, *LPVECTORs3;
