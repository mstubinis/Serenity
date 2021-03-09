#include <serenity/ecs/systems/SystemComponentLogic1.h>
#include <serenity/ecs/components/ComponentLogic1.h>
#include <serenity/ecs/ECS.h>

SystemComponentLogic1::SystemComponentLogic1(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& system, const float dt, Scene& scene) {
        auto& pool = system.getComponentPool<ComponentLogic1>(0);
        const auto& components = pool.data();
        for (auto& component : components) {
            component.call(dt);
        }
    });
}