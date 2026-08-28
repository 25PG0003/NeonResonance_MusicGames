#include "Camera.h"
#include <DirectXMath.h>
#include "../GameSystem.h"

using namespace DirectX;

Camera::Camera()
{
	Position = Vector3(0.0f, 10.0f, -10.0f);
	Target = Vector3(0.0f, 0.0f, 0.0f);
	Up = Vector3(0.0f, 1.0f, 0.0f);
}

void Camera::Initialize(float width, float height)
{
	Position = Vector3(0.0f, 5.0f, -12.0f);
	Target = Vector3(0.0f, 0.0f, 0.0f);
	Up = Vector3(0.0f, 1.0f, 0.0f);

	ViewMatrix.LookAt(Position, Target, Up);

	ProjectionMatrix.PerspectiveFov(
		XMConvertToRadians(45.0f),
		width / height,
		0.1f, 1000.0f
	);
}

void Camera::SetPosition(const Vector3& pos)
{
	Position = pos;
	ViewMatrix.LookAt(Position, Target, Up);
}

void Camera::SetTarget(const Vector3& target)
{
	Target = target;
	ViewMatrix.LookAt(Position, Target, Up);
}