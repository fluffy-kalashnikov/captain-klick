#pragma once

struct Mat4
{
	float m11, m12, m13, m14,
	      m21, m22, m23, m24,
	      m31, m32, m33, m34,
	      m41, m42, m43, m44;
	Mat4()
		: m11(0), m12(0), m13(0), m14(0)
		, m21(0), m22(0), m23(0), m24(0)
		, m31(0), m32(0), m33(0), m34(0)
		, m41(0), m42(0), m43(0), m44(0)
	{}
	Mat4(const float& a11, const float& a12, const float& a13, const float& a14,
		 const float& a21, const float& a22, const float& a23, const float& a24,
		 const float& a31, const float& a32, const float& a33, const float& a34,
		 const float& a41, const float& a42, const float& a43, const float& a44)
		: m11(a11), m12(a12), m13(a13), m14(a14)
		, m21(a21), m22(a22), m23(a23), m24(a24)
		, m31(a31), m32(a32), m33(a33), m34(a34)
		, m41(a41), m42(a42), m43(a43), m44(a44)
	{}
	Mat4(const Mat4&) = default;
	~Mat4() = default;
	Mat4& operator=(const Mat4&) = default;
	static const Mat4 Identity;
};

inline const Mat4 Mat4::Identity(1, 0, 0, 0, 
	                             0, 1, 0, 0, 
	                             0, 0, 1, 0, 
	                             0, 0, 0, 1);