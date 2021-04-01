#include <serenity/ecs/systems/SystemComponentLogic.h>
#include <serenity/ecs/components/ComponentLogic.h>
#include <serenity/ecs/ECSComponentPool.h>

SystemComponentLogic::SystemComponentLogic(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& system, const float dt, Scene& scene) {
        const auto& components = system.getComponentPool<ComponentLogic>(0).data();
        for (const auto& component : components) {
            component.comp.call(dt);
        }
    });
}