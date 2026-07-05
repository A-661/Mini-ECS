#pragma once
#include "ECS/System.h"
#include "ECS/World.h"
#include <iostream>

class MovementSystem : public System
{
public:
    explicit MovementSystem()
    {
    }

    const char* GetTypeName() const override { return "MovementSystem"; }

    void Tick(World& world, float dt) override
    {
        world.ForEach<TransformComponent, VelocityComponent>([dt](Entity entity, TransformComponent& tr, VelocityComponent& vel)
            {
                const Vector3 deltaPosition = vel.linearVelocity * dt;
                const Vector3 deltaRotation = vel.angularVelocity * dt;

                tr.position += deltaPosition;
                tr.rotation += deltaRotation;

                std::cout << "  Moved entity " << entity
                    << " by (" << deltaPosition.x << ", " << deltaPosition.y << ", " << deltaPosition.z << ")"
                    << " and rotated by (" << deltaRotation.x << ", " << deltaRotation.y << ", " << deltaRotation.z << ")\n";
            });
    }
};
