#pragma once

struct Vec3
{
	float x;
	float y;
	float z;
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
};