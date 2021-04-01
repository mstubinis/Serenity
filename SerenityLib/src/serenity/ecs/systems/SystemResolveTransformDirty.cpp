#include <serenity/ecs/systems/SystemResolveTransformDirty.h>
#include <serenity/ecs/components/ComponentBody.h>
#include <serenity/ecs/components/ComponentBodyRigid.h>

SystemResolveTransformDirty::SystemResolveTransformDirty(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system = (SystemResolveTransformDirty&)inSystem;

        //TODO: rethink this, should these be worldPositions, worldRotations, worldScales instead of local? (except for compound children?)
        system.forEach([](Entity entity, ComponentBody* transform, ComponentBodyRigid* rigidBody) {
            if (!transform->hasParent()) {
                rigidBody->internal_setPosition(transform->m_Position.x, transform->m_Position.y, transform->m_Position.z);
                rigidBody->internal_setRotation(transform->m_Rotation.x, transform->m_Rotation.y, transform->m_Rotation.z, transform->m_Rotation.w); //TODO: is this needed below instead?
            }
            rigidBody->internal_setScale(transform->m_Scale.x, transform->m_Scale.y, transform->m_Scale.z);
        }, SystemExecutionPolicy::ParallelWait);
    });
}
