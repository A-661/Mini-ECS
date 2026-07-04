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

    void Tick(World& world, float dt) override
    {
        world.ForEach<TransformComponent, VelocityComponent>([dt](Entity entity, TransformComponent& tr, VelocityComponent& vel)
            {
                const float dx = vel.vx * dt;
                const float dy = vel.vy * dt;
                const float dz = vel.vz * dt;

                tr.x += dx;
                tr.y += dy;
                tr.z += dz;

                std::cout << "  Moved entity " << entity
                    << " by (" << dx << ", " << dy << ", " << dz << ")\n";
            });
    }
};
