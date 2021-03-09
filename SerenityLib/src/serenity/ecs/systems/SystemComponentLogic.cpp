#include <serenity/ecs/systems/SystemComponentLogic.h>
#include <serenity/ecs/components/ComponentLogic.h>
#include <serenity/ecs/ECS.h>

SystemComponentLogic::SystemComponentLogic(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& system, const float dt, Scene& scene) {
        auto& pool = system.getComponentPool<ComponentLogic>(0);
        const auto& components = pool.data();
        for (auto& component : components) {
            component.call(dt);
        }
    });
}