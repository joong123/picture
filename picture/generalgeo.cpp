#include "stdafx.h"

#include "generalgeo.h"

VECTOR3f::VECTOR3f(const VECTOR3f &v2)
{
	x = v2.x;
	y = v2.y;
	z = v2.z;
}

VECTOR3f::VECTOR3f(FLOAT x, FLOAT y, FLOAT z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void VECTOR3f::Output(ostream &out)
{
	out << '(';
	out << setprecision(3) << x << ", " << y << ", " << z << ')';
}

VECTOR3f & VECTOR3f::operator=(const VECTOR3f &v2)
{
	x = v2.x;
	y = v2.y;
	z = v2.z;

	return *this;
}

VECTOR3f & VECTOR3f::operator+=(const VECTOR3f &v2)
{
	x += v2.x;
	y += v2.y;
	z += v2.z;

	return *this;
}

VECTOR3f & VECTOR3f::operator-=(const VECTOR3f &v2)
{
	x -= v2.x;
	y -= v2.y;
	z -= v2.z;

	return *this;
}

VECTOR3f & VECTOR3f::operator*=(FLOAT f)
{
	x *= f;
	x *= f;
	x *= f;

	return *this;
}

VECTOR3f & VECTOR3f::operator/=(FLOAT f)
{
	if (f != 0.0f)
	{
		x /= f;
		x /= f;
		x /= f;
	}

	return *this;
}

VECTOR3f VECTOR3f::operator+() const
{
	return *this;
}

VECTOR3f VECTOR3f::operator-() const
{
	VECTOR3f rs;
	rs.x = -x;
	rs.y = -y;
	rs.z = -z;

	return rs;
}

VECTOR3f VECTOR3f::operator+(const VECTOR3f &v2) const
{
	VECTOR3f rs;
	rs.x = x + v2.x;
	rs.y = y + v2.y;
	rs.z = z + v2.z;

	return rs;
}

VECTOR3f VECTOR3f::operator-(const VECTOR3f &v2) const
{
	VECTOR3f rs;
	rs.x = x - v2.x;
	rs.y = y - v2.y;
	rs.z = z - v2.z;

	return rs;
}

VECTOR3f VECTOR3f::operator*(FLOAT f) const
{
	VECTOR3f rs;
	rs.x = x * f;
	rs.y = y * f;
	rs.z = z * f;

	return rs;
}

VECTOR3f VECTOR3f::operator/(FLOAT f) const
{
	VECTOR3f rs;
	if (f != 0.0f)
	{
		rs.x = x / f;
		rs.y = y / f;
		rs.z = z / f;
	}
	else
	{
		rs.x = x;
		rs.y = y;
		rs.z = z;
	}

	return rs;
}

SPHERICAL_ANGLE GetSphericalAngle(short x, short y, short z)
{
	float azi = 0, zen = 0;

	if (x != 0 && y != 0)
	{
		azi = R2D_F(atanf((float)y / x));

		//这两步不能合并
		if (azi < 0)
			azi += 180.0f;
		if (y < 0)
			azi += 180.0f;

		zen = R2D_F(atanf((float)z / sqrtf((float)x*x + y*y)));
	}
	else if (x == 0 && y == 0)
	{
		azi = 0;
		zen = z == 0 ? 90.0f : (z > 0 ? 180.0f : 0.0f);
	}
	else if (x == 0)
	{
		azi = y > 0 ? 90.0f : 270.0f;
		zen = R2D_F(atanf((float)z / abs(y)));
	}
	else //(y == 0)
	{
		azi = x > 0 ? 0.0f : 180.0f;
		zen = R2D_F(atanf((float)z / abs(x)));
	}
	zen += 90.0f;

	return{ azi,zen };
}

const WCHAR * GetDegreeOfRadian(double angle)
{
	WCHAR strAngle[32] = L"";
	double degree = R2D(angle);
	int deg = (int)degree;// 度整数值
	degree = degree - deg;
	int min = (int)(degree * 60);// 分整数值
	degree = degree - min / 60.0;
	double sec = degree * 3600;
	StringCchPrintf(strAngle, 32, L"%d°%d'%.2lf\"", deg, min, sec);

	return strAngle;
}

VECTOR3f operator*(FLOAT f, const VECTOR3f &v2)
{
	VECTOR3f rs;
	rs.x = f*v2.x;
	rs.y = f*v2.y;
	rs.z = f*v2.z;

	return rs;
}

ostream & operator<<(ostream & out, const VECTOR3f &v)
{
	out << setw(4) << v.x << ", " << v.y << ", " << v.z;

	return out;
}

BOOL VECTOR3f::operator==(const VECTOR3f &v2) const
{
	return (x == v2.x && y == v2.y && z == v2.z);
}

BOOL VECTOR3f::operator!=(const VECTOR3f &v2) const
{
	return (x != v2.x || y != v2.y || z != v2.z);
}

FLOAT VECTOR3f::Length() const
{
	return sqrtf(x*x + y*y + z*z);
}

FLOAT VECTOR3f::LengthSq() const
{
	return x*x + y*y + z*z;
}



VECTOR3::VECTOR3(const VECTOR3 &v2)
{
	x = v2.x;
	y = v2.y;
	z = v2.z;
}

VECTOR3::VECTOR3(int x2, int y2, int z2)
{
	x = x2;
	y = y2;
	z = z2;
}

VECTOR3 & VECTOR3::operator=(const VECTOR3 &v2)
{
	x = v2.x;
	y = v2.y;
	z = v2.z;

	return *this;
}

VECTOR3 & VECTOR3::operator+=(const VECTOR3 &v2)
{
	x += v2.x;
	y += v2.y;
	z += v2.z;

	return *this;
}

VECTOR3 & VECTOR3::operator-=(const VECTOR3 &v2)
{
	x -= v2.x;
	y -= v2.y;
	z -= v2.z;

	return *this;
}

VECTOR3 & VECTOR3::operator*=(int i2)
{
	x *= i2;
	x *= i2;
	x *= i2;

	return *this;
}

VECTOR3 & VECTOR3::operator/=(int i2)
{
	if (i2 != 0)
	{
		x /= i2;
		x /= i2;
		x /= i2;
	}

	return *this;
}

VECTOR3 VECTOR3::operator+() const
{
	return *this;
}

VECTOR3 VECTOR3::operator-() const
{
	return VECTOR3(-x, -y, -z);
}

VECTOR3 VECTOR3::operator+(const VECTOR3 &v2) const
{
	VECTOR3 rs;
	rs.x = x + v2.x;
	rs.y = y + v2.y;
	rs.z = z + v2.z;

	return rs;
}

VECTOR3 VECTOR3::operator-(const VECTOR3 &v2) const
{
	VECTOR3 rs;
	rs.x = x - v2.x;
	rs.y = y - v2.y;
	rs.z = z - v2.z;

	return rs;
}

VECTOR3 VECTOR3::operator*(int i2) const
{
	VECTOR3 rs;
	rs.x = x * i2;
	rs.y = y * i2;
	rs.z = z * i2;

	return rs;
}

VECTOR3 VECTOR3::operator/(int i2) const
{
	VECTOR3 rs;
	if (i2 != 0)
	{
		rs.x = x / i2;
		rs.y = y / i2;
		rs.z = z / i2;
	}
	else
	{
		rs.x = x;
		rs.y = y;
		rs.z = z;
	}

	return rs;
}

VECTOR3 operator*(int i2, const VECTOR3 &v2)
{
	VECTOR3 rs;
	rs.x = i2*v2.x;
	rs.y = i2*v2.y;
	rs.z = i2*v2.z;

	return rs;
}

VECTOR3s operator*(int i2, const VECTOR3s &v2)
{
	VECTOR3s rs;
	rs.x = i2*v2.x;
	rs.y = i2*v2.y;
	rs.z = i2*v2.z;

	return rs;
}

BOOL VECTOR3::operator==(const VECTOR3 &v2) const
{
	return (x == v2.x && y == v2.y && z == v2.z);
}

BOOL VECTOR3::operator!=(const VECTOR3 &v2) const
{
	return (x != v2.x || y != v2.y || z != v2.z);
}

FLOAT VECTOR3::Length() const
{
	return sqrtf((float)x*x + (float)y*y + (float)z*z);
}

FLOAT VECTOR3::LengthSq() const
{
	return (float)x*x + (float)y*y + (float)z*z;
}

VECTOR3s::VECTOR3s(const VECTOR3s &v2)
{
	x = v2.x;
	y = v2.y;
	z = v2.z;
}

VECTOR3s::VECTOR3s(int x2, int y2, int z2)
{
	x = x2;
	y = y2;
	z = z2;
}

VECTOR3s & VECTOR3s::operator=(const VECTOR3s &v2)
{
	x = v2.x;
	y = v2.y;
	z = v2.z;

	return *this;
}

VECTOR3s & VECTOR3s::operator+=(const VECTOR3s &v2)
{
	x += v2.x;
	y += v2.y;
	z += v2.z;

	return *this;
}

VECTOR3s & VECTOR3s::operator-=(const VECTOR3s &v2)
{
	x -= v2.x;
	y -= v2.y;
	z -= v2.z;

	return *this;
}

VECTOR3s & VECTOR3s::operator*=(int i2)
{
	x *= i2;
	x *= i2;
	x *= i2;

	return *this;
}

VECTOR3s & VECTOR3s::operator/=(int i2)
{
	if (i2 != 0)
	{
		x /= i2;
		x /= i2;
		x /= i2;
	}

	return *this;
}

VECTOR3s VECTOR3s::operator+() const
{
	return *this;
}

VECTOR3s VECTOR3s::operator-() const
{
	return VECTOR3s(-x, -y, -z);
}

VECTOR3s VECTOR3s::operator+(const VECTOR3s &v2) const
{
	VECTOR3s rs;
	rs.x = x + v2.x;
	rs.y = y + v2.y;
	rs.z = z + v2.z;

	return rs;
}

VECTOR3s VECTOR3s::operator-(const VECTOR3s &v2) const
{
	VECTOR3s rs;
	rs.x = x - v2.x;
	rs.y = y - v2.y;
	rs.z = z - v2.z;

	return rs;
}

VECTOR3s VECTOR3s::operator*(int i2) const
{
	VECTOR3s rs;
	rs.x = x * i2;
	rs.y = y * i2;
	rs.z = z * i2;

	return rs;
}

VECTOR3s VECTOR3s::operator/(int i2) const
{
	VECTOR3s rs;
	if (i2 != 0)
	{
		rs.x = x / i2;
		rs.y = y / i2;
		rs.z = z / i2;
	}
	else
	{
		rs.x = x;
		rs.y = y;
		rs.z = z;
	}

	return rs;
}

BOOL VECTOR3s::operator==(const VECTOR3s &v2) const
{
	return (x == v2.x && y == v2.y && z == v2.z);
}

BOOL VECTOR3s::operator!=(const VECTOR3s &v2) const
{
	return (x != v2.x || y != v2.y || z != v2.z);
}

FLOAT VECTOR3s::Length() const
{
	return sqrtf((float)x*x + (float)y*y + (float)z*z);
}

FLOAT VECTOR3s::LengthSq() const
{
	return (float)x*x + (float)y*y + (float)z*z;
}
