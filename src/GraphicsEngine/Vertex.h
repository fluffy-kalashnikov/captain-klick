#pragma once
#include "Math/Vec4.h"

struct Vertex
{
	Vec4 position;
	Vec4 color;
	Vertex(const float& aX, const float& aY, const float& aZ);
};

struct Index
{
	unsigned short n{};
};

inline Vertex::Vertex(const float& aX, const float& aY, const float& aZ)
	: position(aX, aY, aZ, 1)
	, color(1, 1, 1, 1)
{
};