#include <serenity/ecs/systems/SystemComponentTransform.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentModel.h>

SystemComponentTransform::SystemComponentTransform(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    /*
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system = (SystemComponentTransform&)inSystem;
        system.forEach<float>([](float dt, Entity entity, ComponentBody* b) {
            b->m_Position += (b->m_LinearVelocity * (decimal)dt);
        }, dt, SystemExecutionPolicy::ParallelWait);
    });
    */
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto& system  = (SystemComponentTransform&)inSystem;
        auto model    = entity.getComponent<ComponentModel>();
        if (model) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
        }
    });
}
