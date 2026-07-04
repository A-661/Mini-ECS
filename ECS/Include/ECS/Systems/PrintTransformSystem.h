#pragma once
#include "ECS/System.h"
#include "ECS/World.h"
#include <iostream>

class PrintTransformSystem : public System
{
public:
    explicit PrintTransformSystem()
    {
    }

    void Tick(World& world, float dt) override
    {
        WorldECS& ecs = world.GetECS();

        ecs.ForEach<TransformComponent>([](Entity entity, TransformComponent& tr)
            {
                std::cout << "  Entity " << entity
                    << " location = (" << tr.x << ", " << tr.y << ", " << tr.z << ")\n";
            });
    }
};
