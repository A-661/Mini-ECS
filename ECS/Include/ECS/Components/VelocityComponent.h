#pragma once
#include "ECS/Component.h"
#include "ECS/Math/Vector3.h"

struct VelocityComponent : ComponentTag
{
    Vector3 linearVelocity;
    Vector3 angularVelocity;

    VelocityComponent() = default;

    VelocityComponent(float inVx, float inVy, float inVz)
        : linearVelocity(inVx, inVy, inVz)
    {
    }

    VelocityComponent(const Vector3& inLinearVelocity, const Vector3& inAngularVelocity = Vector3{})
        : linearVelocity(inLinearVelocity), angularVelocity(inAngularVelocity)
    {
    }
};
