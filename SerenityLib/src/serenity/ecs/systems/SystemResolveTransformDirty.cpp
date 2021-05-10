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
        /*  if this is a root transform and has a bullet rigid body
            this system happens BEFORE SystemRigidTransformSync
            TODO: this probably needs some bug fixing   */
        system.forEach<SystemBodyParentChild*>([](SystemBodyParentChild* pcsArg, Entity entity, ComponentTransform* transform, ComponentRigidBody* rigidBody) {
            auto collisionShape = entity.getComponent<ComponentCollisionShape>();
            pcsArg->computeRigidBodyMatrices(rigidBody, collisionShape, entity);
        }, & systemPC, SystemExecutionPolicy::ParallelWait);
    });
}
