#ifndef __CBFRAME_H__
#define __CBFRAME_H__


#ifdef __cplusplus
	#include "math/Vec2.h"
	#include "math/Vec3.h"
	#include "math/Vec4.h"
	#include "math/Mat4.h"
	using float1 = float;
	using float2 = Vec2;
	using float3 = Vec3;
	using float4 = Vec4;
	using float4x4 = Mat4;
#endif //__cplusplus

struct cbFrame
{
	float1 deltaSeconds{};
	float1 timeSeconds{};
	float4x4 transform;
};

#endif //__CBFRAME_H__