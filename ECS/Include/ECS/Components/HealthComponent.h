#pragma once
#include "ECS/Component.h"
struct HealthComponent : Component
{
    float currentHP = 0.0f;
    float maxHP = 0.0f;

    HealthComponent() = default;

    HealthComponent(float inCurrentHP, float inMaxHP) : currentHP(inCurrentHP), maxHP(inMaxHP) {}
};