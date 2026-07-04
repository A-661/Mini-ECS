#include "ECS/SceneManager.h"
#include "ECS/World.h"
#include "ECS/System.h"

bool SceneManager::LoadWorld(const std::filesystem::path& path)
{
    WorldDesc desc;
    desc.WorldFilePath = path;

    auto world = std::make_unique<World>(desc);

    if (!world->Load()) { return false; }

    _worldVector.push_back(std::move(world));
    return true;
}

bool SceneManager::UnloadWorld(size_t index)
{
    if (index >= _worldVector.size())
    {
        return false;
    }

    World* worldToRemove = _worldVector[index].get();

    _systemVector.erase(
        std::remove_if(
            _systemVector.begin(),
            _systemVector.end(),
            [worldToRemove](const SystemEntry& entry)
            {
                return entry.world == worldToRemove;
            }),
        _systemVector.end()
    );

    _worldVector.erase(_worldVector.begin() + index);
    return true;
}

bool SceneManager::SaveWorld(World* world, const std::filesystem::path& path)
{
    if (!world->Save(path)) { return false; }
    return true;
}

World* SceneManager::GetWorld(size_t index)
{
    if (index >= _worldVector.size())
        return nullptr;

    return _worldVector[index].get();
}

void SceneManager::Tick(float dt)
{
    for (auto& entry : _systemVector)
    {
        if (!entry.world || entry.world->IsPaused()) { continue; }

        entry.system->Tick(*entry.world, dt);
    }
}
