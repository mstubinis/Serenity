#include <serenity/ecs/systems/SystemComponentBody.h>
#include <serenity/ecs/components/ComponentBody.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/ECS.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/ecs/ECSComponentPool.h>
#include <serenity/physics/Collision.h>
#include <serenity/resources/Engine_Resources.h>

SystemComponentBody::SystemComponentBody(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system = (SystemComponentBody&)inSystem;
        system.forEach<float>([](float dt, Entity entity, ComponentBody* b) {
            b->m_Position += (b->m_LinearVelocity * (decimal)dt);
        }, dt, SystemExecutionPolicy::ParallelWait);
    });
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto& system  = (SystemComponentBody&)inSystem;
        auto model    = entity.getComponent<ComponentModel>();
        if (model) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
        }
    });
}
