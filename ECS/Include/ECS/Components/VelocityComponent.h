#pragma once
#include "ECS/Component.h"

struct VelocityComponent : ComponentTag
{
    float vx = 0.0f;
    float vy = 0.0f;
    float vz = 0.0f;

    VelocityComponent() = default;

    VelocityComponent(float inVx, float inVy, float inVz)
        : vx(inVx), vy(inVy), vz(inVz)
    {
    }
};
