#include "ECS/SceneManager.h"
#include "ECS/World.h"

#include <iostream>

namespace
{
    void PrintVector(const Vector3& value)
    {
        std::cout << "(" << value.x << ", " << value.y << ", " << value.z << ")";
    }

    void PrintRestoredWorld(World* world)
    {
        if (!world)
        {
            return;
        }

        std::cout << "World restored: " << world->GetName()
            << " | paused: " << (world->IsPaused() ? "true" : "false")
            << " | timeScale: " << world->GetTimeScale()
            << " | entities: " << world->GetEntityCount() << "\n";

        for (Entity entity : world->GetAliveEntities())
        {
            std::cout << "  [" << world->GetName() << "] Entity " << entity;

            if (world->HasComponent<NameComponent>(entity))
            {
                std::cout << " | name: " << world->GetComponent<NameComponent>(entity).value;
            }

            if (world->HasComponent<TransformComponent>(entity))
            {
                const TransformComponent& transform = world->GetComponent<TransformComponent>(entity);
                std::cout << " | position: ";
                PrintVector(transform.position);
            }

            if (world->HasComponent<VelocityComponent>(entity))
            {
                const VelocityComponent& velocity = world->GetComponent<VelocityComponent>(entity);
                std::cout << " | velocity: ";
                PrintVector(velocity.linearVelocity);
            }

            if (world->HasComponent<HealthComponent>(entity))
            {
                const HealthComponent& health = world->GetComponent<HealthComponent>(entity);
                std::cout << " | health: " << health.currentHP << "/" << health.maxHP;
            }

            std::cout << "\n";
        }
    }
}

// Demo:
// 1. load scene from YAML
// 2. print worlds/entities/components
// 3. tick systems 3 times
// 4. save changed scene back to YAML

int main()
{
    std::cout << "Engine test start...\n";

    const char* scenePath = "Assets/Scenes/DemoScene.yaml";

    SceneManager sceneManager;
    if (!sceneManager.LoadScene(scenePath))
    {
        std::cout << "Failed to load scene: " << scenePath << "\n";
        return -1;
    }

    std::cout << "Scene loaded: " << sceneManager.GetSceneName()
        << " | worlds: " << sceneManager.GetWorldCount() << "\n";

    for (size_t worldIndex = 0; worldIndex < sceneManager.GetWorldCount(); ++worldIndex)
    {
        PrintRestoredWorld(sceneManager.GetWorld(worldIndex));
    }

    std::cout << "\nRunning scene ticks...\n";
    for (int i = 0; i < 3; ++i)
    {
        std::cout << "tick " << (i + 1) << ":\n";
        sceneManager.Tick(1.0f);
    }

    const char* savedScenePath = "Assets\\Scenes\\SavedDemoScene.yaml";
    if (!sceneManager.SaveScene(savedScenePath))
    {
        std::cout << "Failed to save scene: " << savedScenePath << "\n";
        return -1;
    }

    std::cout << "Scene saved: " << savedScenePath << "\n";

    return 0;
}
