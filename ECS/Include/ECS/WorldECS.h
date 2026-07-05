#pragma once
#include "ECS/ECSStorage.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/VelocityComponent.h"
#include "ECS/Components/HealthComponent.h"
#include "ECS/Components/NameComponent.h"

using WorldECS = ECSStorage<
    TransformComponent,
    VelocityComponent,
    HealthComponent,
    NameComponent
>;