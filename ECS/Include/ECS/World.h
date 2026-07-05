#pragma once
#include <string>
#include <filesystem>
#include <cstdint>
#include <utility>
#include <vector>

#include "WorldECS.h"

struct WorldDesc {
    std::filesystem::path WorldFilePath;
    std::string Name;
};

class World {
public:
    World(const WorldDesc& desc)
        : _desc(desc)
    {
    }

    bool Load();
    
    void Tick(float dt);

    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsEntityAlive(Entity entity) const;
    size_t GetEntityCount() const;
    std::vector<Entity> GetAliveEntities() const;

    template<typename T, typename... Args>
    T& AddComponent(Entity entity, Args&&... args)
    {
        return _ecs.AddComponent<T>(entity, std::forward<Args>(args)...);
    }

    template<typename T>
    void RemoveComponent(Entity entity)
    {
        _ecs.RemoveComponent<T>(entity);
    }

    template<typename T>
    bool HasComponent(Entity entity) const
    {
        return _ecs.HasComponent<T>(entity);
    }

    template<typename T>
    T& GetComponent(Entity entity)
    {
        return _ecs.GetComponent<T>(entity);
    }

    template<typename T>
    const T& GetComponent(Entity entity) const
    {
        return _ecs.GetComponent<T>(entity);
    }

    template<typename... Components, typename Func>
    void ForEach(Func&& func)
    {
        _ecs.ForEach<Components...>(std::forward<Func>(func));
    }

    void SetTimeScale(float scale);
    float GetTimeScale() const;

    void SetPaused(bool paused);
    bool IsPaused() const;

    WorldECS& GetECS();
    const WorldECS& GetECS() const;

    void SetName(std::string name);
    const std::string& GetName() const;

    const std::filesystem::path& GetPath() const;

private:
    
    WorldDesc _desc;

    WorldECS _ecs;

    float _timeScale = 1.0f;
    bool _bPaused = false;
};
