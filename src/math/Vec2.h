#pragma once

struct Vec2
{
	float x;
	float y;
	static Vec2 Zero;
	static Vec2 One;
	Vec2();
	Vec2(const float& aX, const float& aY);
	Vec2(const Vec2&) = default;
	~Vec2() = default;
	Vec2& operator=(const Vec2&) = default;
	bool operator==(const Vec2&) const = default;
	bool operator!=(const Vec2&) const = default;
};

inline Vec2 Vec2::Zero;
inline Vec2 Vec2::One(1, 1);

inline Vec2::Vec2()
	: x(0)
	, y(0)
{
}

inline Vec2::Vec2(const float& aX, const float& aY)
	: x(aX)
	, y(aY)
{
}
