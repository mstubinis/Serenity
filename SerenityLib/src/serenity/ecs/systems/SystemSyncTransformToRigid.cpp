#include <serenity/ecs/systems/SystemSyncTransformToRigid.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/math/Engine_Math.h>

SystemSyncTransformToRigid::SystemSyncTransformToRigid(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    /* if this is a root transform, set this transform to use the bullet physics rigid body's coordinate system
       this system happens AFTER SystemSyncRigidToTransform  */
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system = (SystemSyncTransformToRigid&)inSystem;
        system.forEach([](Entity entity, ComponentTransform* transform, ComponentRigidBody* rigidBody) {
            if (!transform->hasParent()) {
                transform->m_Position = rigidBody->getPosition();
                transform->m_Rotation = rigidBody->getRotation();
                Engine::Math::recalculateForwardRightUp(*rigidBody->getBtBody(), transform->m_Forward, transform->m_Right, transform->m_Up);
            }
        }, SystemExecutionPolicy::ParallelWait);
    });
}