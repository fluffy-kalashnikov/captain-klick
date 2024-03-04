#ifndef __CBPASS_H__
#define __CBPASS_H__

#ifdef __cplusplus
	#include "Math/Vec1.h"
	#include "Math/Vec2.h"
	#include "Math/Vec3.h"
	#include "Math/Vec4.h"
	#include "Math/Mat4.h"
	using float1 = Vec1;
	using float2 = Vec2;
	using float3 = Vec3;
	using float4 = Vec4;
	using float4x4 = Mat4;
#endif //__cplusplus

struct cbInstanceStruct
{
	float4x4 transform;
	float1 PADDING0[48];
};

struct cbPassStruct
{
	float1 deltaSeconds;
	float1 timeSeconds;
	float2 PADDING0;
	float4x4 VP;
	float1 PADDING1[44];
};

#ifdef __cplusplus
	static_assert(sizeof(cbPassStruct) % 256 == 0, "must have 256-byte alignment");
	static_assert(sizeof(cbInstanceStruct) % 256 == 0, "must have 256-byte alignment");
#else
	ConstantBuffer<cbInstanceStruct> cbInstance : register(b0);
	ConstantBuffer<cbPassStruct> cbPass : register(b1);
#endif //__cplusplus

#endif //__CBPASS_H__