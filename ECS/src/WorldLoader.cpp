#include "ECS/WorldLoader.h"

#include "ECS/RymlUtils.h"
#include "ECS/World.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
    void LoadNameComponent(World& world, Entity entity, ryml::ConstNodeRef node, const std::string& context)
    {
        RymlUtils::RequireMap(node, context);
        world.AddComponent<NameComponent>(entity, NameComponent{ RymlUtils::ReadRequiredStringChild(node, "Value", context) });
    }

    void LoadTransformComponent(World& world, Entity entity, ryml::ConstNodeRef node, const std::string& context)
    {
        RymlUtils::RequireMap(node, context);

        TransformComponent transform;
        transform.position = RymlUtils::ReadVector3Child(node, "Position", context);
        transform.rotation = RymlUtils::ReadVector3Child(node, "Rotation", context);
        transform.scale = RymlUtils::ReadVector3Child(node, "Scale", context);

        world.AddComponent<TransformComponent>(entity, transform);
    }

    void LoadVelocityComponent(World& world, Entity entity, ryml::ConstNodeRef node, const std::string& context)
    {
        RymlUtils::RequireMap(node, context);
        world.AddComponent<VelocityComponent>(
            entity,
            VelocityComponent{ RymlUtils::ReadVector3Child(node, "Value", context) });
    }

    void LoadHealthComponent(World& world, Entity entity, ryml::ConstNodeRef node, const std::string& context)
    {
        RymlUtils::RequireMap(node, context);
        world.AddComponent<HealthComponent>(
            entity,
            HealthComponent{
                RymlUtils::ReadRequiredFloatChild(node, "Current", context),
                RymlUtils::ReadRequiredFloatChild(node, "Max", context)
            });
    }
}

// loads one world from already parsed YAML node
bool WorldLoader::LoadWorldFromYamlNode(World& world, ryml::ConstNodeRef worldNode, const std::string& context)
{
    try
    {
        RymlUtils::RequireMap(worldNode, context);

        const std::string worldName = RymlUtils::ReadRequiredStringChild(worldNode, "Name", context);
        world.SetName(worldName);
        world.SetPaused(RymlUtils::ReadOptionalBoolChild(worldNode, "Paused", false, context));
        world.SetTimeScale(RymlUtils::ReadOptionalFloatChild(worldNode, "TimeScale", 1.0f, context));

        if (!RymlUtils::HasChild(worldNode, "Entities"))
        {
            return true;
        }

        ryml::ConstNodeRef entitiesNode = worldNode["Entities"];
        RymlUtils::RequireSequence(entitiesNode, context + ".Entities");

        std::unordered_map<std::string, Entity> yamlIdToEntity;
        std::vector<std::pair<ryml::ConstNodeRef, Entity>> entityLoadList;
        entityLoadList.reserve(static_cast<size_t>(entitiesNode.num_children()));

        // first pass - create all runtime entities and build YAML id => Entity map
        size_t entityIndex = 0;
        for (ryml::ConstNodeRef entityNode : entitiesNode.children())
        {
            const std::string entityContext = context + ".Entities[" + std::to_string(entityIndex) + "]";
            RymlUtils::RequireMap(entityNode, entityContext);

            const std::string yamlId = RymlUtils::ReadRequiredStringChild(entityNode, "Id", entityContext);
            if (yamlId.empty())
            {
                throw RymlUtils::Error(entityContext + ".Id must not be empty");
            }

            if (yamlIdToEntity.find(yamlId) != yamlIdToEntity.end())
            {
                throw RymlUtils::Error(context + " has duplicate Entity Id '" + yamlId + "'");
            }

            const Entity entity = world.CreateEntity();
            yamlIdToEntity.emplace(yamlId, entity);
            entityLoadList.emplace_back(entityNode, entity);
            ++entityIndex;
        }

        // second pass - add components after all entities exist
        for (const auto& [entityNode, entity] : entityLoadList)
        {
            if (!RymlUtils::HasChild(entityNode, "Components"))
            {
                continue;
            }

            ryml::ConstNodeRef componentsNode = entityNode["Components"];
            if (!componentsNode.readable() || componentsNode.is_val())
            {
                continue;
            }

            const std::string yamlId = RymlUtils::ReadRequiredStringChild(entityNode, "Id", context + ".Entity");
            const std::string componentContext = context + ".Entity('" + yamlId + "').Components";
            RymlUtils::RequireMap(componentsNode, componentContext);

            for (ryml::ConstNodeRef componentNode : componentsNode.children())
            {
                const std::string componentType = RymlUtils::ToString(componentNode.key());
                const std::string componentFieldContext = componentContext + "." + componentType;

                if (componentType == "Name")
                {
                    LoadNameComponent(world, entity, componentNode, componentFieldContext);
                }
                else if (componentType == "Transform")
                {
                    LoadTransformComponent(world, entity, componentNode, componentFieldContext);
                }
                else if (componentType == "Velocity")
                {
                    LoadVelocityComponent(world, entity, componentNode, componentFieldContext);
                }
                else if (componentType == "Health")
                {
                    LoadHealthComponent(world, entity, componentNode, componentFieldContext);
                }
                else
                {
                    // yes, its a forward-compatible loader with file versions
                    std::cerr << "[WorldLoader] Warning: unknown component '" << componentType
                        << "' on Entity '" << yamlId << "' in World '" << worldName << "'\n";
                }
            }
        }

        return true;
    }
    catch (const RymlUtils::Error& error)
    {
        std::cerr << "[WorldLoader] Failed to load world: " << error.what() << "\n";
        return false;
    }
}

bool WorldLoader::LoadFromFile(World& world, const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[WorldLoader] Failed to open world file for reading: " << path << "\n";
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    try
    {
        RymlUtils::ScopedErrorHandler errorHandler;
        const std::string yaml = buffer.str();
        ryml::Tree tree = ryml::parse_in_arena(ryml::csubstr(yaml.data(), yaml.size()));
        return LoadWorldFromYamlNode(world, tree.crootref(), "World");
    }
    catch (const std::exception& error)
    {
        std::cerr << "[WorldLoader] Failed to parse world YAML '" << path << "': " << error.what() << "\n";
        return false;
    }
}
