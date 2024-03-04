#pragma once
#include "Math/Mat4.h"
#include "Math/Vec3.h"

struct Transform
{
public:
	Vec3 translation;
	Vec3 orientation;
	Vec3 scale;
	Transform();
	Transform(const Transform&) = default;
	~Transform() = default;
	Transform& operator=(const Transform&) = default;
	Mat4 ToMatrix() const;
	[[nodiscard]] Vec3 TransformPoint(const Vec3& aVector) const;
	[[nodiscard]] Vec3 TransformVector(const Vec3& aVector) const;
};

inline Transform::Transform()
	: translation(Vec3::Zero)
	, orientation(Vec3::Zero)
	, scale(Vec3::One)
{
}

inline Mat4 Transform::ToMatrix() const
{
	return Mat4::TransformMatrix(translation, orientation, scale);
}

inline Vec3 Transform::TransformPoint(const Vec3& aPoint) const
{
	return Mat4::TransformPoint(aPoint, ToMatrix());
}

inline Vec3 Transform::TransformVector(const Vec3& aVector) const
{
	return Mat4::TransformVector(aVector, ToMatrix());
}