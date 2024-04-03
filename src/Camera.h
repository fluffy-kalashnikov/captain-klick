#pragma once
#include "Math/Transform.h"
#include "Math/Vec2.h"
#include "Math/Vec3.h"

class Camera
{
public:
	Camera();
	Camera(const Camera&) = delete;
	~Camera() = default;
	Camera& operator=(const Camera&) = delete;
	void OnInitialize();
	void OnUpdate(float aDeltaSeconds);
	Mat4 ToViewMatrix() const;
	Mat4 ToProjectionMatrix(float aAspectRatio) const;
private:
	Transform myTransform;
	Vec3 myMoveVelocity;
	float myGravity;
	bool myGrounded;
};

