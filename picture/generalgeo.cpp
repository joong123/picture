#include "stdafx.h"

#include "generalgeo.h"

VECTOR3::VECTOR3(const VECTOR3 &v2)
{
	x = v2.x;
	y = v2.y;
	z = v2.z;
}

VECTOR3::VECTOR3(FLOAT x, FLOAT y, FLOAT z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void VECTOR3::Output(ostream &out)
{
	out << '(';
	out << setprecision(3) << x << ", " << y << ", " << z << ')';
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

VECTOR3 & VECTOR3::operator*=(FLOAT f)
{
	x *= f;
	x *= f;
	x *= f;

	return *this;
}

VECTOR3 & VECTOR3::operator/=(FLOAT f)
{
	if (f != 0.0f)
	{
		x /= f;
		x /= f;
		x /= f;
	}

	return *this;
}

VECTOR3 VECTOR3::operator+() const
{
	return *this;
}

VECTOR3 VECTOR3::operator-() const
{
	VECTOR3 rs;
	rs.x = -x;
	rs.y = -y;
	rs.z = -z;

	return rs;
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

VECTOR3 VECTOR3::operator*(FLOAT f) const
{
	VECTOR3 rs;
	rs.x = x * f;
	rs.y = y * f;
	rs.z = z * f;

	return rs;
}

VECTOR3 VECTOR3::operator/(FLOAT f) const
{
	VECTOR3 rs;
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

VECTOR3 operator*(FLOAT f, const VECTOR3 &v2)
{
	VECTOR3 rs;
	rs.x = f*v2.x;
	rs.y = f*v2.y;
	rs.z = f*v2.z;

	return rs;
}

ostream & operator<<(ostream & out, const VECTOR3 &v)
{
	out << setw(4) << v.x << ", " << v.y << ", " << v.z;

	return out;
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
	return sqrtf(x*x + y*y + z*z);
}

FLOAT VECTOR3::LengthSq() const
{
	return x*x + y*y + z*z;
}



VECTORi3::VECTORi3(const VECTORi3 &v2)
{
	x = v2.x;
	y = v2.y;
	z = v2.z;
}

VECTORi3::VECTORi3(int x2, int y2, int z2)
{
	x = x2;
	y = y2;
	z = z2;
}

VECTORi3 & VECTORi3::operator=(const VECTORi3 &v2)
{
	x = v2.x;
	y = v2.y;
	z = v2.z;

	return *this;
}

VECTORi3 & VECTORi3::operator+=(const VECTORi3 &v2)
{
	x += v2.x;
	y += v2.y;
	z += v2.z;

	return *this;
}

VECTORi3 & VECTORi3::operator-=(const VECTORi3 &v2)
{
	x -= v2.x;
	y -= v2.y;
	z -= v2.z;

	return *this;
}

VECTORi3 & VECTORi3::operator*=(int i2)
{
	x *= i2;
	x *= i2;
	x *= i2;

	return *this;
}

VECTORi3 & VECTORi3::operator/=(int i2)
{
	if (i2 != 0)
	{
		x /= i2;
		x /= i2;
		x /= i2;
	}

	return *this;
}

VECTORi3 VECTORi3::operator+() const
{
	return *this;
}

VECTORi3 VECTORi3::operator-() const
{
	return VECTORi3(-x, -y, -z);
}

VECTORi3 VECTORi3::operator+(const VECTORi3 &v2) const
{
	VECTORi3 rs;
	rs.x = x + v2.x;
	rs.y = y + v2.y;
	rs.z = z + v2.z;

	return rs;
}

VECTORi3 VECTORi3::operator-(const VECTORi3 &v2) const
{
	VECTORi3 rs;
	rs.x = x - v2.x;
	rs.y = y - v2.y;
	rs.z = z - v2.z;

	return rs;
}

VECTORi3 VECTORi3::operator*(int i2) const
{
	VECTORi3 rs;
	rs.x = x * i2;
	rs.y = y * i2;
	rs.z = z * i2;

	return rs;
}

VECTORi3 VECTORi3::operator/(int i2) const
{
	VECTORi3 rs;
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

VECTORi3 operator*(int i2, const VECTORi3 &v2)
{
	VECTORi3 rs;
	rs.x = i2*v2.x;
	rs.y = i2*v2.y;
	rs.z = i2*v2.z;

	return rs;
}

VECTORs3 operator*(int i2, const VECTORs3 &v2)
{
	VECTORs3 rs;
	rs.x = i2*v2.x;
	rs.y = i2*v2.y;
	rs.z = i2*v2.z;

	return rs;
}

BOOL VECTORi3::operator==(const VECTORi3 &v2) const
{
	return (x == v2.x && y == v2.y && z == v2.z);
}

BOOL VECTORi3::operator!=(const VECTORi3 &v2) const
{
	return (x != v2.x || y != v2.y || z != v2.z);
}

FLOAT VECTORi3::Length() const
{
	return sqrtf((float)x*x + (float)y*y + (float)z*z);
}

FLOAT VECTORi3::LengthSq() const
{
	return (float)x*x + (float)y*y + (float)z*z;
}

VECTORs3::VECTORs3(const VECTORs3 &v2)
{
	x = v2.x;
	y = v2.y;
	z = v2.z;
}

VECTORs3::VECTORs3(int x2, int y2, int z2)
{
	x = x2;
	y = y2;
	z = z2;
}

VECTORs3 & VECTORs3::operator=(const VECTORs3 &v2)
{
	x = v2.x;
	y = v2.y;
	z = v2.z;

	return *this;
}

VECTORs3 & VECTORs3::operator+=(const VECTORs3 &v2)
{
	x += v2.x;
	y += v2.y;
	z += v2.z;

	return *this;
}

VECTORs3 & VECTORs3::operator-=(const VECTORs3 &v2)
{
	x -= v2.x;
	y -= v2.y;
	z -= v2.z;

	return *this;
}

VECTORs3 & VECTORs3::operator*=(int i2)
{
	x *= i2;
	x *= i2;
	x *= i2;

	return *this;
}

VECTORs3 & VECTORs3::operator/=(int i2)
{
	if (i2 != 0)
	{
		x /= i2;
		x /= i2;
		x /= i2;
	}

	return *this;
}

VECTORs3 VECTORs3::operator+() const
{
	return *this;
}

VECTORs3 VECTORs3::operator-() const
{
	return VECTORs3(-x, -y, -z);
}

VECTORs3 VECTORs3::operator+(const VECTORs3 &v2) const
{
	VECTORs3 rs;
	rs.x = x + v2.x;
	rs.y = y + v2.y;
	rs.z = z + v2.z;

	return rs;
}

VECTORs3 VECTORs3::operator-(const VECTORs3 &v2) const
{
	VECTORs3 rs;
	rs.x = x - v2.x;
	rs.y = y - v2.y;
	rs.z = z - v2.z;

	return rs;
}

VECTORs3 VECTORs3::operator*(int i2) const
{
	VECTORs3 rs;
	rs.x = x * i2;
	rs.y = y * i2;
	rs.z = z * i2;

	return rs;
}

VECTORs3 VECTORs3::operator/(int i2) const
{
	VECTORs3 rs;
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

BOOL VECTORs3::operator==(const VECTORs3 &v2) const
{
	return (x == v2.x && y == v2.y && z == v2.z);
}

BOOL VECTORs3::operator!=(const VECTORs3 &v2) const
{
	return (x != v2.x || y != v2.y || z != v2.z);
}

FLOAT VECTORs3::Length() const
{
	return sqrtf((float)x*x + (float)y*y + (float)z*z);
}

FLOAT VECTORs3::LengthSq() const
{
	return (float)x*x + (float)y*y + (float)z*z;
}
