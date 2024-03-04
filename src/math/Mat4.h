#pragma once
#include "Math/Vec3.h"
#include "Math/Vec4.h"
#include <cmath>

struct Mat4
{
	float m11, m12, m13, m14,
	      m21, m22, m23, m24,
	      m31, m32, m33, m34,
	      m41, m42, m43, m44;
	static const Mat4 Zero;
	static const Mat4 Identity;
	constexpr Mat4();
	constexpr Mat4(const float& a11, const float& a12, const float& a13, const float& a14,
		           const float& a21, const float& a22, const float& a23, const float& a24,
		           const float& a31, const float& a32, const float& a33, const float& a34,
		           const float& a41, const float& a42, const float& a43, const float& a44);
	Mat4(const Mat4&) = default;
	~Mat4() = default;
	Mat4& operator=(const Mat4&) = default;
	bool operator==(const Mat4&) const = default;
	bool operator!=(const Mat4&) const = default;
	Mat4 Transpose() const;
	Mat4 Inverse() const;
	static Vec3 TransformPoint(const Vec3& aV, const Mat4& aM);
	static Vec3 TransformVector(const Vec3& aV, const Mat4& aM);

	static Mat4 RollMatrix(float aRollInRadians);
	static Mat4 PitchMatrix(float aPitchInRadians);
	static Mat4 YawMatrix(float aYawInRadians);
	static Mat4 RollPitchYawMatrix(float aRollInRadians, float aPitchInRadians, float aYawInRadians);
	static Mat4 ScaleMatrix(float aX, float aY, float aZ);
	static Mat4 TranslationMatrix(float aX, float aY, float aZ);
	static Mat4 TransformMatrix(const Vec3& aLocation, const Vec3& aRotation, const Vec3& aScale);
	static Mat4 PerspectiveMatrix(float aFieldOfView, float aAspectRatio, float aNearPlane, float aFarPlane);
};

inline const Mat4 Mat4::Zero;
inline const Mat4 Mat4::Identity(1, 0, 0, 0, 
	                             0, 1, 0, 0, 
	                             0, 0, 1, 0, 
	                             0, 0, 0, 1);

inline constexpr Mat4::Mat4()
	: m11(0), m12(0), m13(0), m14(0)
	, m21(0), m22(0), m23(0), m24(0)
	, m31(0), m32(0), m33(0), m34(0)
	, m41(0), m42(0), m43(0), m44(0)
{
}

inline constexpr Mat4::Mat4(const float& a11, const float& a12, const float& a13, const float& a14,
	                        const float& a21, const float& a22, const float& a23, const float& a24,
	                        const float& a31, const float& a32, const float& a33, const float& a34,
	                        const float& a41, const float& a42, const float& a43, const float& a44)
	: m11(a11), m12(a12), m13(a13), m14(a14)
	, m21(a21), m22(a22), m23(a23), m24(a24)
	, m31(a31), m32(a32), m33(a33), m34(a34)
	, m41(a41), m42(a42), m43(a43), m44(a44)
{
}

inline Mat4 operator*(const Mat4& lhs, const Mat4& rhs)
{
	Mat4 mul;
	mul.m11 = lhs.m11 * rhs.m11 + lhs.m12 * rhs.m21 + lhs.m13 * rhs.m31 + lhs.m14 * rhs.m41;
	mul.m12 = lhs.m11 * rhs.m12 + lhs.m12 * rhs.m22 + lhs.m13 * rhs.m32 + lhs.m14 * rhs.m42;
	mul.m13 = lhs.m11 * rhs.m13 + lhs.m12 * rhs.m23 + lhs.m13 * rhs.m33 + lhs.m14 * rhs.m43;
	mul.m14 = lhs.m11 * rhs.m14 + lhs.m12 * rhs.m24 + lhs.m13 * rhs.m34 + lhs.m14 * rhs.m44;
	mul.m21 = lhs.m21 * rhs.m11 + lhs.m22 * rhs.m21 + lhs.m23 * rhs.m31 + lhs.m24 * rhs.m41;
	mul.m22 = lhs.m21 * rhs.m12 + lhs.m22 * rhs.m22 + lhs.m23 * rhs.m32 + lhs.m24 * rhs.m42;
	mul.m23 = lhs.m21 * rhs.m13 + lhs.m22 * rhs.m23 + lhs.m23 * rhs.m33 + lhs.m24 * rhs.m43;
	mul.m24 = lhs.m21 * rhs.m14 + lhs.m22 * rhs.m24 + lhs.m23 * rhs.m34 + lhs.m24 * rhs.m44;
	mul.m31 = lhs.m31 * rhs.m11 + lhs.m32 * rhs.m21 + lhs.m33 * rhs.m31 + lhs.m34 * rhs.m41;
	mul.m32 = lhs.m31 * rhs.m12 + lhs.m32 * rhs.m22 + lhs.m33 * rhs.m32 + lhs.m34 * rhs.m42;
	mul.m33 = lhs.m31 * rhs.m13 + lhs.m32 * rhs.m23 + lhs.m33 * rhs.m33 + lhs.m34 * rhs.m43;
	mul.m34 = lhs.m31 * rhs.m14 + lhs.m32 * rhs.m24 + lhs.m33 * rhs.m34 + lhs.m34 * rhs.m44;
	mul.m41 = lhs.m41 * rhs.m11 + lhs.m42 * rhs.m21 + lhs.m43 * rhs.m31 + lhs.m44 * rhs.m41;
	mul.m42 = lhs.m41 * rhs.m12 + lhs.m42 * rhs.m22 + lhs.m43 * rhs.m32 + lhs.m44 * rhs.m42;
	mul.m43 = lhs.m41 * rhs.m13 + lhs.m42 * rhs.m23 + lhs.m43 * rhs.m33 + lhs.m44 * rhs.m43;
	mul.m44 = lhs.m41 * rhs.m14 + lhs.m42 * rhs.m24 + lhs.m43 * rhs.m34 + lhs.m44 * rhs.m44;
	return mul;
}

inline Vec4 operator*(const Vec4& lhs, const Mat4& rhs)
{
	Vec4 mul;
	mul.x = lhs.x * rhs.m11 + lhs.y * rhs.m21 + lhs.z * rhs.m31 + lhs.w * rhs.m41;
	mul.y = lhs.x * rhs.m12 + lhs.y * rhs.m22 + lhs.z * rhs.m32 + lhs.w * rhs.m42;
	mul.z = lhs.x * rhs.m13 + lhs.y * rhs.m23 + lhs.z * rhs.m33 + lhs.w * rhs.m43;
	mul.w = lhs.x * rhs.m14 + lhs.y * rhs.m24 + lhs.z * rhs.m34 + lhs.w * rhs.m44;
	return mul;
}

inline Mat4 Mat4::Transpose() const
{
	return {
		m11, m21, m31, m41,
		m12, m22, m32, m42,
		m13, m23, m33, m43,
		m14, m24, m34, m44
	};
}

inline Mat4 Mat4::Inverse() const
{
	const Mat4 inverseOrientation = {
		m11, m21, m31, 0,
		m12, m22, m32, 0,
		m13, m23, m33, 0,
		0, 0, 0, 1
	};
	const Mat4 inverseTranslation = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-m41, -m42, -m43, 1
	};
	return inverseTranslation * inverseOrientation;
}

inline Vec3 Mat4::TransformPoint(const Vec3& aPoint, const Mat4& aTransform)
{
	Vec4 point(aPoint.x, aPoint.y, aPoint.z, 1.f);
	point = point * aTransform;
	return Vec3(point.x, point.y, point.z);
}

inline Vec3 Mat4::TransformVector(const Vec3& aVector, const Mat4& aTransform)
{
	Vec4 vector(aVector.x, aVector.y, aVector.z, 0.f);
	vector = vector * aTransform;
	return Vec3(vector.x, vector.y, vector.z);
}

inline Mat4 Mat4::RollMatrix(float aRollInRadians)
{
	const float sin = std::sin(aRollInRadians);
	const float cos = std::cos(aRollInRadians);
	return {
		cos, sin, 0.0, 0.0,
	   -sin, cos, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
}

inline Mat4 Mat4::PitchMatrix(float aPitchInRadians)
{
	const float sin = std::sin(aPitchInRadians);
	const float cos = std::cos(aPitchInRadians);
	return {
		1.0, 0.0, 0.0, 0.0,
		0.0, cos, sin, 0.0,
		0.0,-sin, cos, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
}

inline Mat4 Mat4::YawMatrix(float aYawInRadians)
{
	const float sin = std::sin(aYawInRadians);
	const float cos = std::cos(aYawInRadians);
	return {
		cos, 0.0,-sin, 0.0,
		0.0, 1.0, 0.0, 0.0,
		sin, 0.0, cos, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
}

inline Mat4 Mat4::RollPitchYawMatrix(float aRollInRadians, float aPitchInRadians, float aYawInRadians)
{
	Mat4 m = RollMatrix(aRollInRadians);
	m = m * PitchMatrix(aPitchInRadians);
	m = m * YawMatrix(aYawInRadians);
	return m;
}

inline Mat4 Mat4::ScaleMatrix(float aX, float aY, float aZ)
{
	const float& m11 = aX;
	const float& m22 = aY;
	const float& m33 = aZ;
	return {
		m11, 0.0, 0.0, 0.0,
		0.0, m22, 0.0, 0.0,
		0.0, 0.0, m33, 0.0,
		0.0, 0.0, 0.0, 1.0,
	};
}

inline Mat4 Mat4::TranslationMatrix(float aX, float aY, float aZ)
{
	const float& m41 = aX;
	const float& m42 = aY;
	const float& m43 = aZ;
	return {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		m41, m42, m43, 1.0,
	};
}

inline Mat4 Mat4::TransformMatrix(const Vec3& aLocation, const Vec3& aRotation, const Vec3& aScale)
{
	Mat4 m = ScaleMatrix(aScale.x, aScale.y, aScale.z);
	m = m * RollPitchYawMatrix(aRotation.z, aRotation.x, aRotation.y);
	m = m * TranslationMatrix(aLocation.x, aLocation.y, aLocation.z);
	return m;
}

inline Mat4 Mat4::PerspectiveMatrix(const float aFieldOfView, 
	const float aAspectRatio, const float aNearPlane, const float aFarPlane)
{
	const float zoomX = 1.f / std::tanf(aFieldOfView / 2.f);
	const float zoomY = zoomX * aAspectRatio;
	const float nearPlane = aNearPlane;
	const float farPlane = aFarPlane;
	const float Q = farPlane / (farPlane - nearPlane);

	const float m11 = zoomX;
	const float m22 = zoomY;
	const float m33 = Q;
	const float m43 = (-nearPlane * Q);

	return {
		m11, 0.0, 0.0, 0.0,
		0.0, m22, 0.0, 0.0,
		0.0, 0.0, m33, 1.0,
		0.0, 0.0, m43, 0.0
	};
}
