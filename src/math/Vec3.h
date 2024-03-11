#pragma once
#include <cmath>

struct Vec3
{
	float x;
	float y;
	float z;
	static const Vec3 Zero;
	static const Vec3 One;
	Vec3()
		: x(0)
		, y(0)
		, z(0)
	{
	}
	Vec3(const float& aX, const float& aY, const float& aZ)
		: x(aX)
		, y(aY)
		, z(aZ)
	{
	}
	Vec3(const Vec3&) = default;
	~Vec3() = default;
	Vec3& operator=(const Vec3&) = default;
	bool operator==(const Vec3&) const = default;
	bool operator!=(const Vec3&) const = default;
	void Normalize();
	float Size() const;
	float SizeSquared() const;
};

inline const Vec3 Vec3::Zero; 
inline const Vec3 Vec3::One(1, 1, 1);


inline void operator+=(Vec3& lhs, const Vec3& rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
}

inline Vec3 operator+(Vec3 lhs, const Vec3& rhs)
{
	lhs += rhs;
	return lhs;
}

inline void operator-=(Vec3& lhs, const Vec3& rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
}

inline Vec3 operator-(Vec3 lhs, const Vec3& rhs)
{
	lhs -= rhs;
	return lhs;
}

inline Vec3 operator-(Vec3 lhs)
{
	lhs.x = -lhs.x;
	lhs.y = -lhs.y;
	lhs.z = -lhs.z;
	return lhs;
}

inline void operator*=(Vec3& lhs, const Vec3& rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	lhs.z *= rhs.z;
}

inline Vec3 operator*(Vec3 lhs, const Vec3& rhs)
{
	lhs *= rhs;
	return lhs;
}

inline void operator*=(Vec3& lhs, const float& rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	lhs.z *= rhs;
}

inline Vec3 operator*(Vec3 lhs, const float& rhs)
{
	lhs *= rhs;
	return lhs;
}

inline void operator/=(Vec3& lhs, const Vec3& rhs)
{
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;
	lhs.z /= rhs.z;
}

inline Vec3 operator/(Vec3 lhs, const Vec3& rhs)
{
	lhs /= rhs;
	return lhs;
}

inline void operator/=(Vec3& lhs, const float& rhs)
{
	lhs.x /= rhs;
	lhs.y /= rhs;
	lhs.z /= rhs;
}

inline Vec3 operator/(Vec3 lhs, const float& rhs)
{
	lhs /= rhs;
	return lhs;
}

inline float operator|(const Vec3& lhs, const Vec3& rhs)
{
	return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

inline void Vec3::Normalize()
{
	if (const float size = Size())
	{
		x /= size;
		y /= size;
		z /= size;
	}
}

inline float Vec3::Size() const
{
	return std::sqrt(SizeSquared());
}

inline float Vec3::SizeSquared() const
{
	return x*x + y*y + z*z;
}
