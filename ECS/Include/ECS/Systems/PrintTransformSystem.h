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

    const char* GetTypeName() const override { return "PrintTransformSystem"; }

    void Tick(World& world, float dt) override
    {
        world.ForEach<TransformComponent>([](Entity entity, TransformComponent& tr)
            {
                std::cout << "  Entity " << entity
                    << " position = (" << tr.position.x << ", " << tr.position.y << ", " << tr.position.z << ")"
                    << " rotation = (" << tr.rotation.x << ", " << tr.rotation.y << ", " << tr.rotation.z << ")"
                    << " scale = (" << tr.scale.x << ", " << tr.scale.y << ", " << tr.scale.z << ")\n";
            });
    }
};
