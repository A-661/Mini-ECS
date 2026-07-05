#include "ECS/SceneManager.h"

#include "ECS/RymlUtils.h"
#include "ECS/World.h"
#include "ECS/WorldLoader.h"
#include "ECS/Systems/MovementSystem.h"
#include "ECS/Systems/PrintTransformSystem.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

namespace
{
    std::string FormatVector3(const Vector3& value)
    {
        return "[" + RymlUtils::FormatFloat(value.x) + ", " + RymlUtils::FormatFloat(value.y) + ", " + RymlUtils::FormatFloat(value.z) + "]";
    }

    std::string MakeEntityYamlId(const World& world, Entity entity, std::unordered_set<std::string>& usedIds, size_t fallbackIndex)
    {
        std::string base;
        if (world.HasComponent<NameComponent>(entity))
        {
            base = RymlUtils::Trim(world.GetComponent<NameComponent>(entity).value);
        }

        if (base.empty())
        {
            base = "Entity" + std::to_string(fallbackIndex);
        }

        std::string candidate = base;
        int suffix = 2;
        while (usedIds.find(candidate) != usedIds.end())
        {
            candidate = base + "_" + std::to_string(suffix++);
        }

        usedIds.insert(candidate);
        return candidate;
    }
}

void SceneManager::SetSceneName(std::string name)
{
    _sceneName = std::move(name);
}

const std::string& SceneManager::GetSceneName() const
{
    return _sceneName;
}

World* SceneManager::CreateWorld(std::string name, bool paused, float timeScale)
{
    WorldDesc desc;
    desc.Name = std::move(name);

    auto world = std::make_unique<World>(desc);
    world->SetPaused(paused);
    world->SetTimeScale(timeScale);

    World* result = world.get();
    _worldVector.push_back(std::move(world));
    return result;
}

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

bool SceneManager::SaveScene(const std::filesystem::path& path) const
{
    std::ofstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[SceneManager] Failed to open scene file for writing: " << path << "\n";
        return false;
    }

    file << "Scene:\n";
    file << "  Name: " << RymlUtils::EscapeYamlScalar(_sceneName) << "\n\n";
    file << "  Worlds:\n";

    for (const std::unique_ptr<World>& worldPtr : _worldVector)
    {
        const World& world = *worldPtr;

        file << "    - Name: " << RymlUtils::EscapeYamlScalar(world.GetName()) << "\n";
        file << "      Paused: " << (world.IsPaused() ? "true" : "false") << "\n";
        file << "      TimeScale: " << RymlUtils::FormatFloat(world.GetTimeScale()) << "\n\n";

        bool hasSystems = false;
        for (const SystemEntry& entry : _systemVector)
        {
            if (entry.world == &world)
            {
                hasSystems = true;
                break;
            }
        }

        if (hasSystems)
        {
            file << "      Systems:\n";
            for (const SystemEntry& entry : _systemVector)
            {
                if (entry.world != &world)
                {
                    continue;
                }

                const std::string type = !entry.type.empty()
                    ? entry.type
                    : (entry.system ? entry.system->GetTypeName() : "UnknownSystem");

                file << "        - Type: " << RymlUtils::EscapeYamlScalar(type) << "\n";
                file << "          Priority: " << static_cast<int>(entry.priority) << "\n";
            }
        }
        else
        {
            file << "      Systems: []\n";
        }

        file << "\n";

        const std::vector<Entity> entities = world.GetAliveEntities();
        if (entities.empty())
        {
            file << "      Entities: []\n";
            continue;
        }

        file << "      Entities:\n";

        std::unordered_set<std::string> usedIds;
        size_t fallbackIndex = 1;

        for (Entity entity : entities)
        {
            const std::string yamlId = MakeEntityYamlId(world, entity, usedIds, fallbackIndex++);
            const bool hasAnyComponent =
                world.HasComponent<NameComponent>(entity) ||
                world.HasComponent<TransformComponent>(entity) ||
                world.HasComponent<VelocityComponent>(entity) ||
                world.HasComponent<HealthComponent>(entity);

            file << "        - Id: " << RymlUtils::EscapeYamlScalar(yamlId) << "\n";

            if (!hasAnyComponent)
            {
                file << "          Components: {}\n";
                continue;
            }

            file << "          Components:\n";

            if (world.HasComponent<NameComponent>(entity))
            {
                const NameComponent& name = world.GetComponent<NameComponent>(entity);
                file << "            Name:\n";
                file << "              Value: " << RymlUtils::EscapeYamlScalar(name.value) << "\n\n";
            }

            if (world.HasComponent<TransformComponent>(entity))
            {
                const TransformComponent& transform = world.GetComponent<TransformComponent>(entity);
                file << "            Transform:\n";
                file << "              Position: " << FormatVector3(transform.position) << "\n";
                file << "              Rotation: " << FormatVector3(transform.rotation) << "\n";
                file << "              Scale: " << FormatVector3(transform.scale) << "\n\n";
            }

            if (world.HasComponent<VelocityComponent>(entity))
            {
                const VelocityComponent& velocity = world.GetComponent<VelocityComponent>(entity);
                file << "            Velocity:\n";
                file << "              Value: " << FormatVector3(velocity.linearVelocity) << "\n\n";
            }

            if (world.HasComponent<HealthComponent>(entity))
            {
                const HealthComponent& health = world.GetComponent<HealthComponent>(entity);
                file << "            Health:\n";
                file << "              Current: " << RymlUtils::FormatFloat(health.currentHP) << "\n";
                file << "              Max: " << RymlUtils::FormatFloat(health.maxHP) << "\n\n";
            }
        }
    }

    return true;
}

bool SceneManager::LoadScene(const std::filesystem::path& path)
{
    std::error_code existsError;
    if (!std::filesystem::exists(path, existsError))
    {
        std::cerr << "[SceneManager] Scene file does not exist: " << path << "\n";
        return false;
    }

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[SceneManager] Failed to open scene file for reading: " << path << "\n";
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    try
    {
        RymlUtils::ScopedErrorHandler errorHandler;

        const std::string yaml = buffer.str();
        ryml::Tree tree = ryml::parse_in_arena(ryml::csubstr(yaml.data(), yaml.size()));
        ryml::ConstNodeRef root = tree.crootref();

        ryml::ConstNodeRef sceneNode = RymlUtils::RequireChild(root, "Scene", "Root");
        RymlUtils::RequireMap(sceneNode, "Scene");

        std::string loadedSceneName = path.stem().string();
        if (RymlUtils::HasChild(sceneNode, "Name"))
        {
            loadedSceneName = RymlUtils::ReadScalar(sceneNode["Name"], "Scene.Name");
        }

        ryml::ConstNodeRef worldsNode = RymlUtils::RequireChild(sceneNode, "Worlds", "Scene");
        RymlUtils::RequireSequence(worldsNode, "Scene.Worlds");

        ClearScene();
        _sceneName = std::move(loadedSceneName);

        size_t worldIndex = 0;
        for (ryml::ConstNodeRef worldNode : worldsNode.children())
        {
            const std::string worldContext = "Scene.Worlds[" + std::to_string(worldIndex) + "]";
            RymlUtils::RequireMap(worldNode, worldContext);

            WorldDesc desc;
            desc.WorldFilePath = path;

            auto world = std::make_unique<World>(desc);
            if (!WorldLoader::LoadWorldFromYamlNode(*world, worldNode, worldContext))
            {
                return false;
            }

            World* worldPtr = world.get();
            _worldVector.push_back(std::move(world));

            if (RymlUtils::HasChild(worldNode, "Systems"))
            {
                ryml::ConstNodeRef systemsNode = worldNode["Systems"];
                RymlUtils::RequireSequence(systemsNode, worldContext + ".Systems");

                size_t systemIndex = 0;
                for (ryml::ConstNodeRef systemNode : systemsNode.children())
                {
                    const std::string systemContext = worldContext + ".Systems[" + std::to_string(systemIndex) + "]";
                    RymlUtils::RequireMap(systemNode, systemContext);

                    const std::string type = RymlUtils::ReadRequiredStringChild(systemNode, "Type", systemContext);
                    const uint8_t priority = RymlUtils::ReadOptionalPriorityChild(systemNode, "Priority", 0, systemContext);
                    AddSystemByName(worldPtr, type, priority);
                    ++systemIndex;
                }
            }

            ++worldIndex;
        }

        return true;
    }
    catch (const std::exception& error)
    {
        std::cerr << "[SceneManager] Failed to load scene '" << path << "': " << error.what() << "\n";
        return false;
    }
}

void SceneManager::ClearScene()
{
    _systemVector.clear();
    _worldVector.clear();
}

World* SceneManager::GetWorld(size_t index)
{
    if (index >= _worldVector.size())
        return nullptr;

    return _worldVector[index].get();
}

size_t SceneManager::GetWorldCount() const
{
    return _worldVector.size();
}

std::unique_ptr<System> SceneManager::CreateSystemByName(const std::string& type)
{
    if (type == "MovementSystem")
    {
        return std::make_unique<MovementSystem>();
    }

    if (type == "PrintTransformSystem")
    {
        return std::make_unique<PrintTransformSystem>();
    }

    return nullptr;
}

bool SceneManager::AddSystemByName(World* world, const std::string& type, uint8_t priority)
{
    std::unique_ptr<System> system = CreateSystemByName(type);
    if (!system)
    {
        std::cerr << "[SceneManager] Warning: unknown system '" << type << "'\n";
        return false;
    }

    SystemEntry entry;
    entry.priority = priority;
    entry.world = world;
    entry.type = type;
    entry.system = std::move(system);

    _systemVector.push_back(std::move(entry));
    SortSystems();
    return true;
}

void SceneManager::SortSystems(std::vector<SystemEntry>& systems)
{
    std::stable_sort(systems.begin(), systems.end(),
        [](const SystemEntry& a, const SystemEntry& b)
        {
            return a.priority < b.priority;
        });
}

void SceneManager::SortSystems()
{
    SortSystems(_systemVector);
}

void SceneManager::Tick(float dt)
{
    for (auto& entry : _systemVector)
    {
        if (!entry.world || !entry.system || entry.world->IsPaused()) { continue; }

        entry.system->Tick(*entry.world, dt * entry.world->GetTimeScale());
    }
}
