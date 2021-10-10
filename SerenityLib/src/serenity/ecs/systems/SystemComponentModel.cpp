#include <serenity/ecs/systems/SystemComponentModel.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/ECSComponentPool.h>

SystemComponentModel::SystemComponentModel(Engine::priv::ECS& ecs) 
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto system = static_cast<SystemComponentModel&>(inSystem);
        system.forEach<float>([](float dt, Entity entity, ComponentModel* componentModel) {
            for (auto& modelInstance : *componentModel) {
                modelInstance->m_Animations.update(dt); //process the animations here
            }
        }, dt, SystemExecutionPolicy::ParallelWait);
    });
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto* componentModel = static_cast<ComponentModel*>(component);
        Engine::priv::ComponentModel_Functions::CalculateRadius(*componentModel);
    });
    setEntityAddedToSceneFunction([](SystemBaseClass& inSystem, Scene& scene, Entity entity) {
        auto& pool           = inSystem.getComponentPool<ComponentModel>(0);
        auto* componentModel = pool.getComponent(entity);
        if (componentModel) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*componentModel);
        }
    });
}