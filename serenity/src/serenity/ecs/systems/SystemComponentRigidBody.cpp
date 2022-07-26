#include <serenity/ecs/systems/SystemComponentRigidBody.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/ECSComponentPool.h>
#include <serenity/resources/Engine_Resources.h>

#include <serenity/physics/PhysicsModule.h>

#include <serenity/scene/Scene.h>

SystemComponentRigidBody::SystemComponentRigidBody(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        //auto& system = static_cast<SystemComponentRigidBody&>(inSystem);
        auto model   = entity.getComponent<ComponentModel>();
        if (model) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
            auto rigidBodyComponent = static_cast<ComponentRigidBody*>(component);
            rigidBodyComponent->internal_calculate_mass();
        }
    });
    setSceneLeftFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& system = static_cast<SystemComponentRigidBody&>(inSystem);
        system.forEach([](Entity entity, ComponentRigidBody* rigidBodyComponent) {
            /*bool result = */rigidBodyComponent->removePhysicsFromWorld();
        }, SystemExecutionPolicy::Normal);
    });
    setSceneEnteredFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& system = static_cast<SystemComponentRigidBody&>(inSystem);
        system.forEach([](Entity entity, ComponentRigidBody* rigidBodyComponent) {
            /*bool result = */rigidBodyComponent->addPhysicsToWorld();
        }, SystemExecutionPolicy::Normal);
    });
}
