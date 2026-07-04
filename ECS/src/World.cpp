#include "ECS/World.h"
#include "ECS/WorldLoader.h"

bool World::Load()
{
    return WorldLoader::LoadFromFile(*this, _desc.WorldFilePath);
}

bool World::Save(std::filesystem::path path)
{
    return WorldLoader::SaveToFile(*this, path);
}

void World::Tick(float dt) // not implemented
{
    if (_bPaused) { return; }

    float scaledDt = dt * _timeScale;

    // scripting here
}

Entity World::CreateEntity()
{
    return _ecs.CreateEntity();
}

void World::DestroyEntity(Entity entity)
{
    _ecs.DestroyEntity(entity);
}

bool World::IsEntityAlive(Entity entity) const
{
    return _ecs.IsAlive(entity);
}

size_t World::GetEntityCount() const
{
    return _ecs.GetEntityCount();
}

void World::SetTimeScale(float scale) { _timeScale = scale; }

float World::GetTimeScale() const { return _timeScale; }

void World::SetPaused(bool paused) { _bPaused = paused; }

bool World::IsPaused() const { return _bPaused; }

void World::SetName(std::string name) {
    _desc.Name = name;
}

const std::string& World::GetName() const
{
    return _desc.Name;
}

const std::filesystem::path& World::GetPath() const { return _desc.WorldFilePath; }

WorldECS& World::GetECS()
{
    return _ecs;
}

const WorldECS& World::GetECS() const
{
    return _ecs;
}
