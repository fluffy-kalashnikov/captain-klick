#pragma once

struct Vec2
{
	float x;
	float y;
	Vec2()
		: x(0)
		, y(0)
	{
	}
	Vec2(const float& aX, const float& aY)
		: x(aX)
		, y(aY)
	{
	}
	Vec2(const Vec2&) = default;
	~Vec2() = default;
	Vec2& operator=(const Vec2&) = default;
};