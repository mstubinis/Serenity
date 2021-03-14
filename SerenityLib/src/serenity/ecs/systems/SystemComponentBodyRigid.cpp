#include <serenity/ecs/systems/SystemComponentBodyRigid.h>
#include <serenity/ecs/components/ComponentBodyRigid.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/ECS.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/ecs/ECSComponentPool.h>
#include <serenity/physics/Collision.h>
#include <serenity/resources/Engine_Resources.h>

SystemComponentBodyRigid::SystemComponentBodyRigid(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system = (SystemComponentBodyRigid&)inSystem;
        system.forEach([](Entity entity, ComponentBodyRigid* b) {
            auto& BtRigidBody = *b->m_BulletRigidBody;
            Engine::Math::recalculateForwardRightUp(BtRigidBody, b->m_Forward, b->m_Right, b->m_Up);
        }, SystemExecutionPolicy::ParallelWait);
    });
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto& system = (SystemComponentBodyRigid&)inSystem;
        auto model = entity.getComponent<ComponentModel>();
        if (model) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
        }
    });
    setEntityAddedToSceneFunction([](SystemBaseClass& inSystem, Scene& scene, Entity entity) {
        auto& pool = inSystem.getComponentPool<ComponentBodyRigid>(0);
        auto* component_ptr = pool.getComponent(entity);
        if (component_ptr) {
            component_ptr->setCollision(component_ptr->m_Collision->getType(), component_ptr->m_Mass);
            auto currentScene = Engine::Resources::getCurrentScene();
            if (currentScene && currentScene == &scene) {
                component_ptr->addPhysicsToWorld(true);
            }
        }
    });
    setSceneEnteredFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& system = (SystemComponentBodyRigid&)inSystem;
        system.forEach([](Entity entity, ComponentBodyRigid* component) {
            component->addPhysicsToWorld(true, false);
        }, SystemExecutionPolicy::Normal);
    });
    setSceneLeftFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& system = (SystemComponentBodyRigid&)inSystem;
        system.forEach([](Entity entity, ComponentBodyRigid* component) {
            component->removePhysicsFromWorld(true, false);
        }, SystemExecutionPolicy::Normal);
    });
}
