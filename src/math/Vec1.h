#pragma once

struct Vec1
{
	float x;
	Vec1();
	Vec1(const float& aX);
	Vec1(const Vec1&) = default;
	~Vec1() = default;
	Vec1& operator=(const Vec1&) = default;
};

inline Vec1::Vec1()
	: x(0)
{
}

inline Vec1::Vec1(const float& aX)
	: x(aX)
{
}
