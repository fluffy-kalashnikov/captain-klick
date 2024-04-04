#include "Camera.h"
#include "Math/Vec3.h"
#include <iostream>
#include "InputHandler.h"
#undef min
#undef max

static constexpr float DEG_90 = 90.f * 3.14f / 180.f;

#include <sstream>

Camera::Camera()
	: myGravity(0)
	, myGrounded(false)
{
}

void Camera::Update(float aDeltaSeconds, const Vec2 aResolution)
{
	myResolution = aResolution;

	//moving
	//Vec3 direction = myMoveVelocity;
	//direction.Normalize();

	//Vec3 acceleration{
	//	static_cast<float>(globalInputHandler.keyboard.d - globalInputHandler.keyboard.a),
	//	0.f,
	//	static_cast<float>(globalInputHandler.keyboard.w - globalInputHandler.keyboard.s)
	//};
	//if (acceleration != Vec3::Zero)
	//{
	//	acceleration.Normalize();
	//	if ((acceleration | direction) <= 0.f)
	//	{
	//		acceleration *= 5.f;
	//	}
	//	acceleration *= (1600 * aDeltaSeconds);
	//	acceleration = Mat4::TransformVector(acceleration, Mat4::YawMatrix(myTransform.orientation.y));
	//	if (!myGrounded)
	//	{
	//		acceleration *= 0.4f;
	//	}

	//	myMoveVelocity += acceleration;
	//	float speedClamped = std::min(400.f, myMoveVelocity.Size());
	//	myMoveVelocity.Normalize();
	//	myMoveVelocity *= speedClamped;
	//}
	//else if (myGrounded)
	//{
	//	const float speed = myMoveVelocity.Size();
	//	myMoveVelocity.Normalize();
	//	myMoveVelocity *= std::max(0.f, speed - 800 * aDeltaSeconds);
	//}

	//myTransform.translation += myMoveVelocity * aDeltaSeconds;



	aDeltaSeconds;

	//looking
	myTransform.orientation.x += globalInputHandler.mouse.delta.y * 0.001f;
	myTransform.orientation.y += globalInputHandler.mouse.delta.x * 0.001f;
	if (myTransform.orientation.x > DEG_90)
	{
		myTransform.orientation.x = DEG_90;
	}
	if (myTransform.orientation.x < -DEG_90)
	{
		myTransform.orientation.x = -DEG_90;
	}





	////falling
	//constexpr float eyeHeight = 150.f;
	//if (globalInputHandler.keyboard.space && myGrounded)
	//{
	//	//jump
	//	myGravity = 500.f;
	//	myGrounded = false;
	//}
	//else if (myTransform.translation.y == eyeHeight && myGravity <= 0)
	//{
	//	//ground
	//	myGravity = 0;
	//	myGrounded = true;
	//}
	//else
	//{
	//	//fall
	//	myGravity -= 1400.f * aDeltaSeconds;
	//}
	//myTransform.translation.y = std::max(eyeHeight, myTransform.translation.y + myGravity * aDeltaSeconds);
}

Mat4 Camera::GetViewMatrix() const
{
	return myTransform.ToMatrix().Inverse();
}

Mat4 Camera::GetProjectionMatrix() const
{
	float aspectRatio = myResolution.x / myResolution.y;
	return Mat4::PerspectiveMatrix(DEG_90, aspectRatio, 10, 10000);
}