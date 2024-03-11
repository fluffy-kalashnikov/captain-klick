#include "Camera.h"
#include "Math/Vec3.h"
#include <iostream>
#include "Window.h"

static constexpr float DEG_90 = 90.f * 3.14f / 180.f;


void Camera::OnInitialize()
{
	myTransform.translation.x = 200;
	myTransform.translation.y = 200;
	myTransform.translation.z = -200;

	myTransform.orientation.x = 3.14f/4.f;
	myTransform.orientation.y = 3.14f/4.f;
}

void Camera::OnUpdate(float aDeltaSeconds)
{
	Vec3 moveDelta{
		static_cast<float>(globalWindow.keyboard.d - globalWindow.keyboard.a),
		static_cast<float>(globalWindow.keyboard.e - globalWindow.keyboard.q),
		static_cast<float>(globalWindow.keyboard.w - globalWindow.keyboard.s)
	};
	moveDelta.Normalize();
	moveDelta *= (200.f * aDeltaSeconds);
	moveDelta = myTransform.TransformVector(moveDelta);

	myTransform.translation += moveDelta;
	myTransform.orientation.x += globalWindow.mouse.delta.y * 0.001f;
	myTransform.orientation.y += globalWindow.mouse.delta.x * 0.001f;

	if (myTransform.orientation.x > DEG_90)
	{
		myTransform.orientation.x = DEG_90;
	}
	if (myTransform.orientation.x < -DEG_90)
	{
		myTransform.orientation.x = -DEG_90;
	}
}

Mat4 Camera::ToViewMatrix() const
{
	return myTransform.ToMatrix().Inverse();
}

Mat4 Camera::ToProjectionMatrix() const
{
	return Mat4::PerspectiveMatrix(DEG_90, globalWindow.aspectRatio, 10, 10000);
}