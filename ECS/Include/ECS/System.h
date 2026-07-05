#pragma once

class World;

class System
{
public:
    virtual ~System() = default;

    virtual const char* GetTypeName() const { return "UnknownSystem"; }

    virtual void Tick(World& world, float dt) = 0;
};