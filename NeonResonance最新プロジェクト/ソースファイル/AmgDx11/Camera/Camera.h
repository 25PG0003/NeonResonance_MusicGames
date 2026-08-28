#pragma once

#include <DirectXMath.h>
#include "../Math/Vector3.h"
#include "../Math/Matrix.h"


class Camera
{
public:
    Camera();
    ~Camera() = default;

    void Initialize(float width, float height);

    void SetPosition(const Vector3& pos);
    void SetTarget(const Vector3& target);

    Vector3 GetPosition() const { return Position; }

    const Matrix& GetView() const { return ViewMatrix; }
    const Matrix& GetProjection() const { return ProjectionMatrix; }

private:
    Vector3 Position;
    Vector3 Target;
    Vector3 Up;

    Matrix ViewMatrix;
    Matrix ProjectionMatrix;
};