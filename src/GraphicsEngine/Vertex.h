#pragma once
#include "Math/Vec2.h"
#include "Math/Vec4.h"

struct Index
{
	unsigned short n{};
};

struct Vertex
{
	Vec4 position;
	Vec2 uv;
	Vertex(const float& aX, const float& aY, const float& aZ);
	Vertex(const float& aX, const float& aY, const float& aZ, const float& aU, const float& aV);
};

inline Vertex::Vertex(const float& aX, const float& aY, const float& aZ)
	: Vertex(aX, aY, aZ, 0, 0)
{
};

inline Vertex::Vertex(const float& aX, const float& aY, const float& aZ, const float& aU, const float& aV)
	: position(aX, aY, aZ, 1)
	, uv(aU, aV)
{
}