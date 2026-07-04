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
    
    Entity e1 = world.CreateEntity();
    world.AddComponent<TransformComponent>(e1, TransformComponent{
        Vector3{ 0.0f, 1.0f, 2.0f },
        Vector3{ 0.0f, 0.0f, 0.0f },
        Vector3{ 1.0f, 1.0f, 1.0f }
    });
    world.AddComponent<VelocityComponent>(e1, VelocityComponent{
        Vector3{ 1.0f, 0.0f, 0.0f },
        Vector3{ 0.0f, 0.0f, 90.0f }
    });
    
    Entity e2 = world.CreateEntity();
    world.AddComponent<TransformComponent>(e2, TransformComponent{
        Vector3{ 10.0f, 20.0f, 30.0f },
        Vector3{ 0.0f, 45.0f, 0.0f },
        Vector3{ 1.0f, 1.0f, 1.0f }
    });
    
    return true;
}

bool WorldLoader::SaveToFile(World& world, const std::filesystem::path& path)
{
    // todo construct yaml and save
    return true;
}
