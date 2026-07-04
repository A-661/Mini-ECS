#include "ECS/SceneManager.h"
#include "ECS/System.h"
#include "ECS/World.h"
#include "ECS/Systems/MovementSystem.h"
#include "ECS/Systems/TestSystem.h"
#include "ECS/Systems/PrintTransformSystem.h"
#include <iostream>



int main()
{
    std::cout << "Engine test start...\n";

    SceneManager sceneManager;

    sceneManager.LoadWorld("world1.yaml");
    sceneManager.LoadWorld("world2.yaml");

    World* world1 = sceneManager.GetWorld(0);
    World* world2 = sceneManager.GetWorld(1);

    if (!world1 || !world2)
    {
        std::cout << "Failed to create worlds\n";
        return -1;
    }

    Entity destroyTestEntity = world1->CreateEntity();
    world1->AddComponent<TransformComponent>(destroyTestEntity, TransformComponent{
        Vector3{ 100.0f, 100.0f, 100.0f },
        Vector3{ 0.0f, 0.0f, 0.0f },
        Vector3{ 1.0f, 1.0f, 1.0f }
    });
    world1->AddComponent<HealthComponent>(destroyTestEntity, HealthComponent{ 10.0f, 10.0f });

    std::cout << "destroy test before | alive: " << world1->IsEntityAlive(destroyTestEntity)
        << " | transform: " << world1->HasComponent<TransformComponent>(destroyTestEntity)
        << " | health: " << world1->HasComponent<HealthComponent>(destroyTestEntity) << "\n";

    world1->DestroyEntity(destroyTestEntity);

    std::cout << "destroy test after | alive: " << world1->IsEntityAlive(destroyTestEntity)
        << " | transform: " << world1->HasComponent<TransformComponent>(destroyTestEntity)
        << " | health: " << world1->HasComponent<HealthComponent>(destroyTestEntity) << "\n";

    // priority 1
    sceneManager.AddSystem<TestSystem>(world2, 1, "Animation (world2)");
    sceneManager.AddSystem<TestSystem>(world1, 1, "Scripting (world1)");

    // priority 0
    sceneManager.AddSystem<TestSystem>(world1, 0, "Physics (world1)");
    sceneManager.AddSystem<TestSystem>(world2, 0, "Scripting (world2)");

    // priority 3
    sceneManager.AddSystem<TestSystem>(world1, 3, "Audio (world1)");

    sceneManager.AddSystem<MovementSystem>(world1, 1);
    sceneManager.AddSystem<MovementSystem>(world2, 0);
    
    sceneManager.AddSystem<PrintTransformSystem>(world1, 1);
    sceneManager.AddSystem<PrintTransformSystem>(world2, 0);


    for (int i = 0; i < 5; i++) {
        std::cout << "tick start\n";

        sceneManager.Tick(0.016f);

        std::cout << "tick end\n";
    }
    

    return 0;
}
