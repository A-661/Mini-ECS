#pragma once
#include <vector>
#include <memory>
#include <filesystem>
#include <cstdint>
#include <algorithm>
#include <string>
#include <utility>

#include "ECS/System.h"

class World;

// main entry-point for multi-world ECS
// SceneManager owns systems, but each system is bound to a specific world
class SceneManager
{
public:
    void SetSceneName(std::string name);
    const std::string& GetSceneName() const;

    World* CreateWorld(std::string name, bool paused = false, float timeScale = 1.0f);

    bool LoadWorld(const std::filesystem::path& path);

    bool UnloadWorld(size_t index);
    
    // saves and loads the full scene files
    bool SaveScene(const std::filesystem::path& path) const;
    bool LoadScene(const std::filesystem::path& path);

    void ClearScene();

    World* GetWorld(size_t index = 0);
    size_t GetWorldCount() const;

    struct SystemEntry
    {
        uint8_t priority = 0;
        World* world = nullptr;
        std::string type;
        std::unique_ptr<System> system;
    };

    template<typename T, typename... Args>
    void AddSystem(World* world, uint8_t priority, Args&&... args) {
        SystemEntry entry;
        entry.priority = priority;
        entry.world = world;
        entry.system = std::make_unique<T>(std::forward<Args>(args)...);
        entry.type = entry.system->GetTypeName();

        _systemVector.push_back(std::move(entry));
        
        // keep insertion order for same priorities
        SortSystems();
    }

    bool AddSystemByName(World* world, const std::string& type, uint8_t priority);

    // systems are ticked by global priority, not sequentially per world
    void Tick(float dt);

private:
    static std::unique_ptr<System> CreateSystemByName(const std::string& type);
    static void SortSystems(std::vector<SystemEntry>& systems);
    void SortSystems();

private:
    std::string _sceneName = "UntitledScene";
    std::vector<std::unique_ptr<World>> _worldVector;
    std::vector<SystemEntry> _systemVector;
};