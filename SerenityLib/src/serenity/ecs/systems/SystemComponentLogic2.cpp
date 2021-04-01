#include <serenity/ecs/systems/SystemComponentLogic2.h>
#include <serenity/ecs/components/ComponentLogic2.h>
#include <serenity/ecs/ECSComponentPool.h>

SystemComponentLogic2::SystemComponentLogic2(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& system, const float dt, Scene& scene) {
        const auto& components = system.getComponentPool<ComponentLogic2>(0).data();
        for (const auto& component : components) {
            component.comp.call(dt);
        }
    });
}