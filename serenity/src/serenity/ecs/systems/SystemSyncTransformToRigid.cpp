#include <serenity/ecs/systems/SystemSyncTransformToRigid.h>
#include <serenity/ecs/systems/SystemTransformParentChild.h>
#include <serenity/ecs/systems/SystemComponentTransform.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/ecs/components/ComponentCollisionShape.h>
#include <serenity/math/Engine_Math.h>

SystemSyncTransformToRigid::SystemSyncTransformToRigid(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    /* if this is a root transform, set this transform to use the bullet physics rigid body's coordinate system
       this system happens AFTER SystemSyncRigidToTransform  */
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system   = static_cast<SystemSyncTransformToRigid&>(inSystem);
        auto& systemPC = inSystem.getECS().getSystem<SystemTransformParentChild>();
        system.forEach<SystemTransformParentChild*>([](SystemTransformParentChild* pcsArg, Entity entity, ComponentTransform* transform, ComponentRigidBody* rigidBody) {
            if (!transform->hasParent()) {
                transform->m_Position = rigidBody->getPosition();
                transform->m_Rotation = rigidBody->getRotation();
#ifdef COMPONENT_TRANSFORM_STORE_RIGHT
                Engine::Math::recalculateForwardRightUp(*rigidBody->getBtBody(), transform->m_Forward, transform->m_Right, transform->m_Up);
#else
                Engine::Math::recalculateForwardUp(*rigidBody->getBtBody(), transform->m_Forward, transform->m_Up);
#endif
                SystemComponentTransform::syncLocalVariablesToTransforms(pcsArg, entity, transform);
            }
        }, &systemPC, SystemExecutionPolicy::ParallelWait);


        systemPC.computeAllParentChildWorldTransforms();


        system.forEach<SystemTransformParentChild*>([](SystemTransformParentChild* pcsArg, Entity entity, ComponentTransform* transform, ComponentRigidBody* rigidBody) {
            auto collisionShape = entity.getComponent<ComponentCollisionShape>();
            pcsArg->syncRigidToTransform(rigidBody, collisionShape, entity);
        }, &systemPC, SystemExecutionPolicy::ParallelWait);
    });
}