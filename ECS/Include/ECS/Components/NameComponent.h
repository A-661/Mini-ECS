#pragma once
#include <string>
#include <utility>

#include "ECS/Component.h"

struct NameComponent : ComponentTag
{
    std::string value;

    NameComponent() = default;

    explicit NameComponent(std::string inValue)
        : value(std::move(inValue))
    {
    }
};
