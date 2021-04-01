#include <serenity/ecs/systems/SystemComponentLogic1.h>
#include <serenity/ecs/components/ComponentLogic1.h>
#include <serenity/ecs/ECSComponentPool.h>

SystemComponentLogic1::SystemComponentLogic1(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& system, const float dt, Scene& scene) {
        const auto& components = system.getComponentPool<ComponentLogic1>(0).data();
        for (const auto& component : components) {
            component.comp.call(dt);
        }
    });
}