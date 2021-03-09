#include <serenity/ecs/systems/SystemComponentLogic2.h>
#include <serenity/ecs/components/ComponentLogic2.h>
#include <serenity/ecs/ECS.h>

SystemComponentLogic2::SystemComponentLogic2(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& system, const float dt, Scene& scene) {
        auto& pool = system.getComponentPool<ComponentLogic2>(0);
        const auto& components = pool.data();
        for (auto& component : components) {
            component.call(dt);
        }
    });
}