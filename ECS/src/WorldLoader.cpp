#include "ECS/WorldLoader.h"
#include "ECS/World.h"

bool WorldLoader::LoadFromFile(World& world, const std::filesystem::path& path)
{
    /* todo if file not found return 0;
    rapid yaml parsing
    create entities and components
    */
    world.SetName(path.string()); 

    
    /* example creating entities */
    
    WorldECS& ecs = world.GetECS();

    Entity e1 = ecs.CreateEntity();
    ecs.AddComponent<TransformComponent>(e1, TransformComponent{ 0.0f, 1.0f, 2.0f });
    ecs.AddComponent<VelocityComponent>(e1, VelocityComponent{ 1.0f, 0.0f, 0.0f });
    
    Entity e2 = ecs.CreateEntity();
    ecs.AddComponent<TransformComponent>(e2, TransformComponent{ 10.0f, 20.0f, 30.0f });
    
    return true;
}

bool WorldLoader::SaveToFile(World& world, const std::filesystem::path& path)
{
    // todo construct yaml and save
    return true;
}
