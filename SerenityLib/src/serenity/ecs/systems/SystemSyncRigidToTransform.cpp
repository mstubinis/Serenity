#include <serenity/ecs/systems/SystemSyncRigidToTransform.h>
#include <serenity/ecs/systems/SystemTransformParentChild.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/ecs/components/ComponentCollisionShape.h>
#include <serenity/math/Engine_Math.h>

SystemSyncRigidToTransform::SystemSyncRigidToTransform(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system   = (SystemSyncRigidToTransform&)inSystem;
        auto& systemPC = inSystem.getECS().getSystem<SystemTransformParentChild>();
        system.forEach<SystemTransformParentChild*>([](SystemTransformParentChild* pcsArg, Entity entity, ComponentTransform* transform, ComponentRigidBody* rigidBody) {
            auto collisionShape = entity.getComponent<ComponentCollisionShape>();
            pcsArg->syncRigidToTransform(rigidBody, collisionShape, entity);
        }, & systemPC, SystemExecutionPolicy::ParallelWait);
    });
}
