#pragma once
#include "Math/Transform.h"
#include "Math/Vec2.h"
#include "Math/Vec3.h"

class Camera
{
public:
	Camera();
	Camera(const Camera&) = default;
	~Camera() = default;
	Camera& operator=(const Camera&) = default;
	void Update(float aDeltaSeconds);
	Mat4 ToViewMatrix() const;
	Mat4 ToProjectionMatrix(float aAspectRatio) const;
private:
	Transform myTransform;
	Vec3 myMoveVelocity;
	float myGravity;
	bool myGrounded;
};

