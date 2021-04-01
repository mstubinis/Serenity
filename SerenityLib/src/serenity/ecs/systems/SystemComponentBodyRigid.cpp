#include <serenity/ecs/systems/SystemComponentBodyRigid.h>
#include <serenity/ecs/components/ComponentBodyRigid.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/ECSComponentPool.h>
#include <serenity/resources/Engine_Resources.h>

#include <serenity/physics/PhysicsModule.h>

#include <serenity/scene/Scene.h>

SystemComponentBodyRigid::SystemComponentBodyRigid(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto& system = (SystemComponentBodyRigid&)inSystem;
        auto model   = entity.getComponent<ComponentModel>();
        if (model) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
            auto rigidBodyComponent = static_cast<ComponentBodyRigid*>(component);
            rigidBodyComponent->internal_calculate_mass();
        }
    });
    setEntityAddedToSceneFunction([](SystemBaseClass& inSystem, Scene& scene, Entity entity) {
        auto& pool               = inSystem.getComponentPool<ComponentBodyRigid>(0);
        auto  rigidBodyComponent = pool.getComponent(entity);
        if (rigidBodyComponent) {
            auto currentScene = Engine::Resources::getCurrentScene();
            if (currentScene && currentScene == &scene) {
                rigidBodyComponent->addPhysicsToWorld();
                //Engine::Physics::addRigidBodyThreadSafe(rigidBodyComponent->getBtBody(), rigidBodyComponent->getCollisionGroup(), rigidBodyComponent->getCollisionMask());
                rigidBodyComponent->internal_calculate_mass();
            }
        }
    });
    setSceneLeftFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& system = (SystemComponentBodyRigid&)inSystem;
        system.forEach([](Entity entity, ComponentBodyRigid* rigidBodyComponent) {
            //Engine::Physics::removeRigidBody(rigidBodyComponent->getBtBody());
            rigidBodyComponent->removePhysicsFromWorld();
        }, SystemExecutionPolicy::Normal);
    });
    setSceneEnteredFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& system = (SystemComponentBodyRigid&)inSystem;
        system.forEach([](Entity entity, ComponentBodyRigid* rigidBodyComponent) {
            //Engine::Physics::addRigidBody(rigidBodyComponent->getBtBody(), rigidBodyComponent->getCollisionGroup(), rigidBodyComponent->getCollisionMask());
            rigidBodyComponent->addPhysicsToWorld();
        }, SystemExecutionPolicy::Normal);
    });
}
