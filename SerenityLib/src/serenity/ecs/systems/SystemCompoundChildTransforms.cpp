#include <serenity/ecs/systems/SystemCompoundChildTransforms.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentCollisionShape.h>
#include <serenity/ecs/systems/SystemTransformParentChild.h>

#include <serenity/math/Engine_Math.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>

#include <serenity/ecs/ECS.h>
#include <serenity/scene/Scene.h>

SystemCompoundChildTransforms::SystemCompoundChildTransforms(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    /*
    * update's the compound child shape coordinate systems
    */
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system   = (SystemCompoundChildTransforms&)inSystem;
        auto& systemPC = inSystem.getECS().getSystem<SystemTransformParentChild>();
        system.forEach<SystemTransformParentChild*>([](SystemTransformParentChild* pcsArg, Entity entity, ComponentCollisionShape* collisionShape) {
            if (collisionShape->isChildShape()) {
                const auto& parentWorldMatrix = pcsArg->m_WorldTransforms[collisionShape->getParent().id() - 1];
                const auto& thisWorldMatrix   = pcsArg->m_WorldTransforms[entity.id() - 1];
                auto localMatrix              = glm::inverse(parentWorldMatrix) * thisWorldMatrix;

                Engine::Math::removeMatrixScale<glm_mat4, glm_vec3>(localMatrix);
                collisionShape->updateChildShapeTransform(localMatrix);
            }
        }, &systemPC, SystemExecutionPolicy::ParallelWait);
    });
}