#include <serenity/ecs/systems/SystemComponentModel.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/ECS.h>
#include <serenity/threading/ThreadingModule.h>

SystemComponentModel::SystemComponentModel(Engine::priv::ECS& ecs) 
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto system      = static_cast<SystemComponentModel&>(inSystem);
        /*
        auto& components = inSystem.getComponentPool<ComponentModel>(0).data();
        auto lamda_update_component = [dt](ComponentModel& componentModel, size_t i, size_t k) {
            for (auto& modelInstance : componentModel) {
                modelInstance->m_Animations.update(dt); //process the animations here
            }
        };
        if (components.size() < 100) {
            for (size_t i = 0; i < components.size(); ++i) {
                lamda_update_component(components[i], i, 0);
            }
        } else {
            Engine::priv::threading::addJobSplitVectored(lamda_update_component, components, true, 0);
        }
        */

        system.forEach<float>([](float dt, ComponentModel* componentModel) {
            for (auto& modelInstance : *componentModel) {
                modelInstance->m_Animations.update(dt); //process the animations here
            }
        }, dt, SystemExecutionPolicy::ParallelWait);
    });
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto* componentModel = static_cast<ComponentModel*>(component);
        if (componentModel) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*componentModel);
        }
    });
    setEntityAddedToSceneFunction([](SystemBaseClass& inSystem, Scene& scene, Entity entity) {
        auto& pool           = inSystem.getComponentPool<ComponentModel>(0);
        auto* componentModel = pool.getComponent(entity);
        if (componentModel) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*componentModel);
        }
    });
}