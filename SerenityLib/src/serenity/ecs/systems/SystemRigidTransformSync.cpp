#include <serenity/ecs/systems/SystemRigidTransformSync.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/math/Engine_Math.h>

SystemRigidTransformSync::SystemRigidTransformSync(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system = (SystemRigidTransformSync&)inSystem;
        system.forEach([](Entity entity, ComponentTransform* transform, ComponentRigidBody* rigidBody) {
            if (!transform->hasParent()) {
                transform->m_Position = rigidBody->internal_getPosition();
                transform->m_Rotation = rigidBody->internal_getRotation();
                Engine::Math::recalculateForwardRightUp(*rigidBody->getBtBody(), transform->m_Forward, transform->m_Right, transform->m_Up);
            }
        }, SystemExecutionPolicy::ParallelWait);
    });
}