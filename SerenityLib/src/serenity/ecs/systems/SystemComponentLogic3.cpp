#include <serenity/ecs/systems/SystemComponentLogic3.h>
#include <serenity/ecs/components/ComponentLogic3.h>
#include <serenity/ecs/ECSComponentPool.h>

SystemComponentLogic3::SystemComponentLogic3(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& system, const float dt, Scene& scene) {
        const auto& components = system.getComponentPool<ComponentLogic3>(0).data();
        for (const auto& component : components) {
            component.comp.call(dt);
        }
    });
}