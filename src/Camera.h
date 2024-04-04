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
	void Update(float aDeltaSeconds, Vec2 aResolution);
	Mat4 GetViewMatrix() const;
	Mat4 GetProjectionMatrix() const;
private:
	bool myGrounded;
	float myGravity;
	Vec2 myResolution;
	Vec3 myMoveVelocity;
	Transform myTransform;
};

