#include <serenity/ecs/systems/SystemResolveTransformDirty.h>
#include <serenity/ecs/systems/SystemBodyParentChild.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/ecs/components/ComponentCollisionShape.h>
#include <serenity/math/Engine_Math.h>

SystemResolveTransformDirty::SystemResolveTransformDirty(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system   = (SystemResolveTransformDirty&)inSystem;
        auto& systemPC = inSystem.getECS().getSystem<SystemBodyParentChild>();

        /*
        * if this is a root transform and has a bullet rigid body
        * this system happens BEFORE SystemRigidTransformSync
        * TODO: this probably needs some bug fixing
        */
        system.forEach<SystemBodyParentChild*>([](SystemBodyParentChild* pcsArg, Entity entity, ComponentTransform* transform, ComponentRigidBody* rigidBody) {
            auto collisionShape         = entity.getComponent<ComponentCollisionShape>();
            const auto& thisWorldMatrix = pcsArg->m_WorldTransforms[entity.id() - 1];
            if (collisionShape && collisionShape->isChildShape()) {
                auto parentWorldMatrix  = pcsArg->m_WorldTransforms[collisionShape->getParent().id() - 1];
                auto localMatrix        = glm::inverse(parentWorldMatrix) * thisWorldMatrix;
                const auto localScale   = Engine::Math::removeMatrixScale<glm_mat4, glm_vec3>(localMatrix);
                collisionShape->updateChildShapeTransform(localMatrix);
            }else{
                rigidBody->internal_set_matrix(thisWorldMatrix);
            }
            //if (!transform->hasParent()) {
            //    rigidBody->internal_setPosition(transform->m_Position.x, transform->m_Position.y, transform->m_Position.z);
            //    rigidBody->internal_setRotation(transform->m_Rotation.x, transform->m_Rotation.y, transform->m_Rotation.z, transform->m_Rotation.w); //TODO: is this needed below instead?
            //}
           // rigidBody->internal_setScale(transform->m_Scale.x, transform->m_Scale.y, transform->m_Scale.z);
        }, & systemPC, SystemExecutionPolicy::ParallelWait);
    });
}
