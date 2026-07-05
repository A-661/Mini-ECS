#pragma once
#include <filesystem>
#include <string>

#include <ryml.hpp>

class World;

class WorldLoader
{
public:
    static bool LoadWorldFromYamlNode(
        World& world,
        ryml::ConstNodeRef worldNode,
        const std::string& context = "World");

    static bool LoadFromFile(World& world, const std::filesystem::path& path);

    static bool SaveToFile(World& world, const std::filesystem::path& path);
};