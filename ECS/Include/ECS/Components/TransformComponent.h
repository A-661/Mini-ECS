#pragma once
#include "ECS/Component.h"
#include "ECS/Math/Vector3.h"

struct TransformComponent : ComponentTag
{
    Vector3 position = Vector3{ 0.0f, 0.0f, 0.0f };
    Vector3 rotation = Vector3{ 0.0f, 0.0f, 0.0f };
    Vector3 scale = Vector3{ 1.0f, 1.0f, 1.0f };

    TransformComponent() = default;

    TransformComponent(float inX, float inY, float inZ)
        : position(inX, inY, inZ)
    {
    }

    TransformComponent(const Vector3& Position, const Vector3& Rotation, const Vector3& Scale)
        : position(Position), rotation(Rotation), scale(Scale)
    {
    }
};
