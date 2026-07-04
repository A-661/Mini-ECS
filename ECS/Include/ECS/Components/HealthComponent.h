#pragma once
#include "ECS/Component.h"
struct HealthComponent : ComponentTag
{
    float currentHP = 0.0f;
    float maxHP = 0.0f;

    HealthComponent() = default;

    HealthComponent(float currentHP, float maxHP) : currentHP(currentHP), maxHP(maxHP) {}
};