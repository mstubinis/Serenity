#include <serenity/ecs/systems/SystemComponentLogic3.h>
#include <serenity/ecs/components/ComponentLogic3.h>
#include <serenity/ecs/ECS.h>

SystemComponentLogic3::SystemComponentLogic3(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& system, const float dt, Scene& scene) {
        auto& pool = system.getComponentPool<ComponentLogic3>(0);
        const auto& components = pool.data();
        for (auto& component : components) {
            component.call(dt);
        }
    });
}