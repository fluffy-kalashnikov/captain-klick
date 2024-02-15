#pragma once

struct Vec4
{
	float x;
	float y;
	float z;
	float w;
	Vec4()
		: x(0)
		, y(0)
		, z(0)
		, w(0)
	{
	}
	Vec4(const float& aX, const float& aY, const float& aZ, const float& aW)
		: x(aX)
		, y(aY)
		, z(aZ)
		, w(aW)
	{
	}
	Vec4(const Vec4&) = default;
	~Vec4() = default;
	Vec4& operator=(const Vec4&) = default;
};