#pragma once

#include <windows.h>
#include <math.h>
#include <iostream>
#include <iomanip>

using std::ostream;
using std::istream;
using std::setw;
using std::setprecision;

//GENERAL
#define PI_F						3.14159265f
#define PI							3.1415926535897932
#define XNOR(B1, B2)				!((B1) ^ (B2))

#define GDIRECT(L, T, W, H)			{(LONG)(L - 1), (LONG)(T - 1), (LONG)(L + W), (LONG)(T + H)}

#define ROUND(f)					(int)(f + 0.5f)
#define ROUND_BYTE(f)				(byte)(f + 0.5f)

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

// 三阶向量运算
#define IS_VEC3_ZERO(V)				((V).x == 0 && (V).y == 0 && (V).z == 0)	//判断向量全0
#define IS_PVEC3_ZERO(V)			((V)->x == 0 && (V)->y == 0 && (V)->z == 0)
#define VEC3_LEN(V)					sqrtf((FLOAT)(V).x*(V).x + (FLOAT)(V).y*(V).y + (FLOAT)(V).z*(V).z)		//向量模
#define PVEC3_LEN(PV)				sqrtf((FLOAT)(PV)->x*(PV)->x + (FLOAT)(PV)->y*(PV)->y + (FLOAT)(PV)->z*(PV)->z)
#define VEC3_LEN_LF(V)				sqrt((double)(V).x*(V).x + (double)(V).y*(V).y + (double)(V).z*(V).z)
#define PVEC3_LEN_LF(PV)			sqrt((double)(PV)->x*(PV)->x + (double)(PV)->y*(PV)->y + (double)(PV)->z*(PV)->z)
#define VEC3_LENSQ(V)				((V).x*(V).x + (V).y*(V).y + (V).z*(V).z)			//向量模平方
#define PVEC3_LENSQ(PV)				((PV)->x*(PV)->x + (PV)->y*(PV)->y + (PV)->z*(PV)->z)
#define VEC3_DOT(V1, V2)			((V1).x*(V2).x + (V1).y*(V2).y + (V1).z*(V2).z)		//向量点积
#define PVEC3_DOT(PV1, PV2)			((PV1)->x*(PV2)->x + (PV1)->y*(PV2)->y + (PV1)->z*(PV2)->z)
#define VEC3_DOT_LF(V1, V2)			((double)(V1).x*(V2).x + (double)(V1).y*(V2).y + (double)(V1).z*(V2).z)	
#define PVEC3_DOT_LF(PV1, PV2)		((double)(PV1)->x*(PV2)->x + (double)(PV1)->y*(PV2)->y + (double)(PV1)->z*(PV2)->z)	
#define VEC3_PROJLEN(VS, VD)		(VEC3_DOT(VS, VD) / VEC3_LEN(VD))					//向量到向量的投影长度
#define PVEC3_PROJLEN(PVS, PVD)		(PVEC3_DOT(PVS, PVD) / PVEC3_LEN(PVD))
#define VEC3_PROJ2PLANE(VS, VD)		()
#define VEC3_ANGLE(V1, V2)			acosf(VEC3_DOT(V1, V2) / VEC3_LEN(V1) / VEC3_LEN(V2))//向量夹角
#define PVEC3_ANGLE(PV1, PV2)		acosf(PVEC3_DOT(PV1, PV2) / PVEC3_LEN(PV1) / PVEC3_LEN(PV2))
#define VEC3_ANGLE_LF(V1, V2)		acos(VEC3_DOT_LF(V1, V2) / VEC3_LEN_LF(V1) / VEC3_LEN_LF(V2))
#define PVEC3_ANGLE_LF(PV1, PV2)	acos(PVEC3_DOT_LF(PV1, PV2) / PVEC3_LEN_LF(PV1) / PVEC3_LEN_LF(PV2))

typedef struct VECTOR3
{
	FLOAT x;
	FLOAT y;
	FLOAT z;
public:
	VECTOR3() {};
	VECTOR3(const VECTOR3&);
	VECTOR3(FLOAT x, FLOAT y, FLOAT z);

	void Output(ostream &);

	// casting
	//operator FLOAT* ();
	//operator const FLOAT* () const;

	// assignment operators
	VECTOR3& operator = (const VECTOR3&);
	VECTOR3& operator += (const VECTOR3&);
	VECTOR3& operator -= (const VECTOR3&);
	VECTOR3& operator *= (FLOAT);
	VECTOR3& operator /= (FLOAT);

	// unary operators
	VECTOR3 operator + () const;
	VECTOR3 operator - () const;

	// binary operators
	VECTOR3 operator + (const VECTOR3&) const;
	VECTOR3 operator - (const VECTOR3&) const;
	VECTOR3 operator * (FLOAT) const;
	VECTOR3 operator / (FLOAT) const;

	friend VECTOR3 operator * (FLOAT, const struct VECTOR3&);

	// relation operators
	BOOL operator == (const VECTOR3&) const;
	BOOL operator != (const VECTOR3&) const;

	// stream operators
	friend ostream& operator << (ostream&, const VECTOR3&);
	friend istream& operator >> (istream&, VECTOR3&);

	// 其他运算，和宏定义功能相同
	FLOAT Length() const;
	FLOAT LengthSq() const;

} VECTOR3, *LPVECTOR3;

// 对binary operators和其他成员函数的加速
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

	pOut->x = pV1->x * f;
	pOut->y = pV1->y * f;
	pOut->z = pV1->z * f;

	return pOut;
}
inline VECTOR3 *Vec3Divide(VECTOR3 *pOut, const VECTOR3 *pV1, FLOAT f)
{
	if (!pOut || !pV1 || !f)
		return NULL;

	pOut->x = pV1->x / f;
	pOut->y = pV1->y / f;
	pOut->z = pV1->z / f;

	return pOut;
}
inline FLOAT Vec3Dot(const VECTOR3 *pV1, const VECTOR3 *pV2)//和宏定义功能相同
{
	if (!pV1 || !pV2)
		return 0.0f;

	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}
inline VECTOR3 *Vec3Cross(VECTOR3 *pOut, const VECTOR3 *pV1, const VECTOR3 *pV2)
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
		pOut->x = pV1->x / len;
		pOut->y = pV1->y / len;
		pOut->z = pV1->z / len;
	}
	else
		;

	return pOut;
}
inline VECTOR3 *Vec3Scale(VECTOR3 *pOut, const VECTOR3 *pV1, FLOAT f)
{
	if (!pOut || !pV1)
		return NULL;

	pOut->x = pV1->x * f;
	pOut->y = pV1->y * f;
	pOut->z = pV1->z * f;

	return pOut;
}
inline VECTOR3 *Vec3Interpolation(VECTOR3 *pOut, const VECTOR3 *pV1, const VECTOR3 *pV2, FLOAT f)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	pOut->x = pV1->x*(1 - f) + pV2->x*f;
	pOut->y = pV1->y*(1 - f) + pV2->y*f;
	pOut->z = pV1->z*(1 - f) + pV2->z*f;

	return pOut;
}
inline VECTOR3 *Vec3Project(VECTOR3 *pOut, const VECTOR3 *pV1, const VECTOR3 *pV2)//向量到向量的投影向量
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	if (PVEC3_LENSQ(pV2) == 0.0f)
	{
		//TODO
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
inline VECTOR3 *Vec3Proj2Plane(VECTOR3 *pOut, const VECTOR3 *pV1, const VECTOR3 *pV2)//向量到平面的投影向量(对向量投影的三角形第三边)
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

		pOut->x = pV1->x - pV2->x * f;
		pOut->y = pV1->y - pV2->y * f;
		pOut->z = pV1->z - pV2->z * f;
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
		pOut->x = pVNOR->x*normaladjust - pVIN->x;
		pOut->y = pVNOR->y*normaladjust - pVIN->y;
		pOut->z = pVNOR->z*normaladjust - pVIN->z;
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
	pOut->x = pV1->x*cosf(angle) + cross.x*sinf(angle) + pAxis->x*para3;
	pOut->y = pV1->y*cosf(angle) + cross.y*sinf(angle) + pAxis->y*para3;
	pOut->z = pV1->z*cosf(angle) + cross.z*sinf(angle) + pAxis->z*para3;

	return pOut;

	//矩阵法
	/*D3DXMATRIX rot;
	D3DXMatrixRotationAxis(&rot, &axis, angle);*/
}
inline VECTOR3 *Vec3RotateX(VECTOR3 *pOut, const VECTOR3 *pV1, FLOAT angle) {}
inline VECTOR3 *Vec3RotateY(VECTOR3 *pOut, const VECTOR3 *pV1, FLOAT angle) {}
inline VECTOR3 *Vec3RotateZ(VECTOR3 *pOut, const VECTOR3 *pV1, FLOAT angle) {}


typedef struct VECTORi3
{
	int x;
	int y;
	int z;
public:
	VECTORi3() {};
	VECTORi3(const VECTORi3&);
	VECTORi3(int x, int y, int z);

	// casting
	//operator int* ();
	//operator const int* () const;

	// assignment operators
	VECTORi3& operator = (const VECTORi3&);
	VECTORi3& operator += (const VECTORi3&);
	VECTORi3& operator -= (const VECTORi3&);
	VECTORi3& operator *= (int);
	VECTORi3& operator /= (int);

	// unary operators
	VECTORi3 operator + () const;
	VECTORi3 operator - () const;

	// binary operators
	VECTORi3 operator + (const VECTORi3&) const;
	VECTORi3 operator - (const VECTORi3&) const;
	VECTORi3 operator * (int) const;
	VECTORi3 operator / (int) const;

	friend VECTORi3 operator * (int, const struct VECTORi3&);

	BOOL operator == (const VECTORi3&) const;
	BOOL operator != (const VECTORi3&) const;

	FLOAT Length() const;
	FLOAT LengthSq() const;

} VECTORi3, *LPVECTORi3;

inline VECTORi3 *Vec3Add(VECTORi3 *pOut, const VECTORi3 *pV1, const VECTORi3 *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;

	return pOut;
}
inline VECTORi3 *Vec3Subtract(VECTORi3 *pOut, const VECTORi3 *pV1, const VECTORi3 *pV2)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;

	return pOut;
}
inline VECTORi3 *Vec3Multiply(VECTORi3 *pOut, const VECTORi3 *pV1, FLOAT f)
{
	if (!pOut || !pV1)
		return NULL;

	pOut->x = (int)(pV1->x * f);
	pOut->y = (int)(pV1->y * f);
	pOut->z = (int)(pV1->z * f);

	return pOut;
}
inline VECTORi3 *Vec3Divide(VECTORi3 *pOut, const VECTORi3 *pV1, FLOAT f)
{
	if (!pOut || !pV1 || !f)
		return NULL;

	pOut->x = (int)(pV1->x / f);
	pOut->y = (int)(pV1->y / f);
	pOut->z = (int)(pV1->z / f);

	return pOut;
}
inline FLOAT Vec3Dot(const VECTORi3 *pV1, const VECTORi3 *pV2)//和宏定义功能相同
{
	if (!pV1 || !pV2)
		return 0.0f;

	return (FLOAT)(pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z);
}
inline VECTORi3 *Vec3Cross(VECTORi3 *pOut, const VECTORi3 *pV1, const VECTORi3 *pV2)
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

inline bool Vec3Orthogonal(const VECTORi3 *pV1, const VECTORi3 *pV2) {
	if (!pV1 || !pV2)
		return false;

	return (PVEC3_DOT(pV1, pV2) == 0);
}
inline bool Vec3Orthogonal(const VECTORi3 *pV1, const VECTORi3 *pV2, double tolerance) {
	if (!pV1 || !pV2)
		return false;

	if (tolerance < 0)
		tolerance = -tolerance;
	double angle = PVEC3_ANGLE_LF(pV1, pV2);
	return (angle >= PI / 2 - tolerance
		&& angle <= PI / 2 + tolerance);
}
inline bool Vec3Parallel(const VECTORi3 *pV1, const VECTORi3 *pV2) {
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
inline bool Vec3Parallel(const VECTORi3 *pV1, const VECTORi3 *pV2, double tolerance) {
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

inline VECTORi3 *Vec3Normalize(VECTORi3 *pOut, const VECTORi3 *pV1)
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
inline VECTORi3 *Vec3Scale(VECTORi3 *pOut, const VECTORi3 *pV1, FLOAT f)
{
	if (!pOut || !pV1)
		return NULL;

	pOut->x = (int)(pV1->x * f);
	pOut->y = (int)(pV1->y * f);
	pOut->z = (int)(pV1->z * f);

	return pOut;
}
inline VECTORi3 *Vec3Interpolation(VECTORi3 *pOut, const VECTORi3 *pV1, const VECTORi3 *pV2, FLOAT f)
{
	if (!pOut || !pV1 || !pV2)
		return NULL;

	pOut->x = (int)(pV1->x*(1 - f) + pV2->x*f);
	pOut->y = (int)(pV1->y*(1 - f) + pV2->y*f);
	pOut->z = (int)(pV1->z*(1 - f) + pV2->z*f);

	return pOut;
}
inline VECTORi3 *Vec3Project(VECTORi3 *pOut, const VECTORi3 *pV1, const VECTORi3 *pV2)//向量到向量的投影向量
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
inline VECTORi3 *Vec3Proj2Plane(VECTORi3 *pOut, const VECTORi3 *pV1, const VECTORi3 *pV2)//向量到平面的投影向量
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

		pOut->x = ROUND(pV1->x - pV2->x * f);
		pOut->y = ROUND(pV1->y - pV2->y * f);
		pOut->z = ROUND(pV1->z - pV2->z * f);
	}

	return pOut;
}
inline VECTORi3 *Vec3Reflect(VECTORi3 *pOut, const VECTORi3 *pVIN, const VECTORi3 *pVNOR)
{
	if (!pOut || !pVIN || !pVNOR)
		return NULL;

	if (PVEC3_LEN(pVNOR) != 0)
	{
		float normaladjust = 2 * PVEC3_PROJLEN(pVIN, pVNOR) / PVEC3_LEN(pVNOR);
		pOut->x = ROUND(pVNOR->x*normaladjust - pVIN->x);
		pOut->y = ROUND(pVNOR->y*normaladjust - pVIN->y);
		pOut->z = ROUND(pVNOR->z*normaladjust - pVIN->z);
	}

	return pOut;
}
inline VECTORi3 *Vec3RotateAxis(VECTORi3 *pOut, const VECTORi3 *pV1, const VECTORi3 *pAxis, FLOAT angle)
{
	if (!pOut || !pV1 || !pAxis)
		return NULL;

	VECTORi3 cross;
	Vec3Cross(&cross, pAxis, pV1);
	/*VECTORi3 result =
	*pV1*cos(angle)
	+ cross*sin(angle)
	+ *pAxis*Vec3Dot(pAxis, pV1)*(1 - cos(angle));*/
	float para3 = Vec3Dot(pAxis, pV1)*(1 - cos(angle));
	pOut->x = ROUND(pV1->x*cosf(angle) + cross.x*sinf(angle) + pAxis->x*para3);
	pOut->y = ROUND(pV1->y*cosf(angle) + cross.y*sinf(angle) + pAxis->y*para3);
	pOut->z = ROUND(pV1->z*cosf(angle) + cross.z*sinf(angle) + pAxis->z*para3);

	return pOut;

	//矩阵法
	/*D3DXMATRIX rot;
	D3DXMatrixRotationAxis(&rot, &axis, angle);*/
}
inline VECTORi3 *Vec3RotateX(VECTORi3 *pOut, const VECTORi3 *pV1, FLOAT angle) {}
inline VECTORi3 *Vec3RotateY(VECTORi3 *pOut, const VECTORi3 *pV1, FLOAT angle) {}
inline VECTORi3 *Vec3RotateZ(VECTORi3 *pOut, const VECTORi3 *pV1, FLOAT angle) {}


typedef struct VECTORs3
{
	short x;
	short y;
	short z;
public:
	VECTORs3() {};
	VECTORs3(const VECTORs3&);
	VECTORs3(int x, int y, int z);

	// casting
	//operator int* ();
	//operator const int* () const;

	// assignment operators
	VECTORs3& operator = (const VECTORs3&);
	VECTORs3& operator += (const VECTORs3&);
	VECTORs3& operator -= (const VECTORs3&);
	VECTORs3& operator *= (int);
	VECTORs3& operator /= (int);

	// unary operators
	VECTORs3 operator + () const;
	VECTORs3 operator - () const;

	// binary operators
	VECTORs3 operator + (const VECTORs3&) const;
	VECTORs3 operator - (const VECTORs3&) const;
	VECTORs3 operator * (int) const;
	VECTORs3 operator / (int) const;

	friend VECTORs3 operator * (int, const struct VECTORs3&);

	BOOL operator == (const VECTORs3&) const;
	BOOL operator != (const VECTORs3&) const;

	FLOAT Length() const;
	FLOAT LengthSq() const;

} VECTORs3, *LPVECTORs3;
