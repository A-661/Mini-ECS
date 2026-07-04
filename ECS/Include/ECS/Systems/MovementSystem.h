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
        WorldECS& ecs = world.GetECS();

        ecs.ForEach<TransformComponent, VelocityComponent>([](Entity entity, TransformComponent& tr, VelocityComponent& vel)
            {
                tr.x += vel.vx;
                tr.y += vel.vy;
                tr.z += vel.vz;

                std::cout << "  Moved entity " << entity
                    << " by (" << vel.vx << ", " << vel.vy << ", " << vel.vz << ")\n";
            });
    }
};
