#pragma once
#include "ECS/Component.h"
struct TransformComponent : ComponentTag
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    TransformComponent() = default;

    TransformComponent(float inX, float inY, float inZ)
        : x(inX), y(inY), z(inZ)
    {
    }
};