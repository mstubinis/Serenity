#include <serenity/ecs/systems/SystemCompoundChildTransforms.h>
#include <serenity/ecs/components/ComponentBody.h>
#include <serenity/ecs/components/ComponentCollisionShape.h>
#include <serenity/ecs/systems/SystemBodyParentChild.h>

#include <serenity/math/Engine_Math.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>

#include <serenity/ecs/ECS.h>
#include <serenity/scene/Scene.h>

SystemCompoundChildTransforms::SystemCompoundChildTransforms(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system   = (SystemCompoundChildTransforms&)inSystem;
        auto& systemPC = Engine::priv::PublicScene::GetECS(scene).getSystem<SystemBodyParentChild>();
        system.forEach<SystemBodyParentChild*>([](SystemBodyParentChild* pcs, Entity entity, ComponentBody* transform, ComponentCollisionShape* collisionShape) {
            if (collisionShape->isChildShape()) {
                auto  parentCompound          = collisionShape->getParentCompoundShape();

                const auto& parentWorldMatrix = pcs->m_WorldTransforms[collisionShape->getParent().id() - 1];
                const auto& thisWorldMatrix   = pcs->m_WorldTransforms[entity.id() - 1];

                auto localPos                 = Engine::Math::getMatrixPosition(thisWorldMatrix) - Engine::Math::getMatrixPosition(parentWorldMatrix);
                localPos                      = localPos * glm::quat_cast(parentWorldMatrix);

                const auto localMatrix        = glm::inverse(parentWorldMatrix) * thisWorldMatrix;
                const auto localRot           = glm::quat_cast(localMatrix);

                btTransform tr;
                tr.setOrigin(Engine::Math::toBT(localPos)); //tr.setOrigin(Engine::Math::toBT(transform->m_Position));
                tr.setRotation(Engine::Math::toBT(localRot)); //tr.setRotation(Engine::Math::toBT(transform->m_Rotation));
                parentCompound->updateChildTransform(collisionShape->getChildShapeIndex(), tr);
                parentCompound->recalculateLocalAabb();
            }
        }, &systemPC, SystemExecutionPolicy::ParallelWait);
    });
}