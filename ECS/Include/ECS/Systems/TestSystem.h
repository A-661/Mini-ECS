#pragma once
#include "ECS/System.h"
#include "ECS/World.h"
#include <iostream>

class TestSystem : public System
{
public:
    TestSystem(const char* name)
        : m_name(name)
    {
    }

    void Tick(World& world, float dt) override
    {
        std::cout << "[TestSystem] " << m_name
            << " | World: " << world.GetPath()
            << " | dt: " << dt << std::endl;
    }

private:
    const char* m_name;
};
