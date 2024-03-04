#pragma once
#include "Math/Transform.h"
#include "Math/Vec2.h"

class Camera
{
public:
	Camera() = default;
	Camera(const Camera&) = delete;
	~Camera() = default;
	Camera& operator=(const Camera&) = delete;
	void OnInitialize();
	void OnUpdate(float aDeltaSeconds);
	Mat4 ToViewMatrix() const;
	Mat4 ToProjectionMatrix() const;
private:
	Transform myTransform;
};

