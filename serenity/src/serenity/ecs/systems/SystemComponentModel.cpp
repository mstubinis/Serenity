#include <serenity/ecs/systems/SystemComponentModel.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/ECSComponentPool.h>

#include <serenity/utils/BlockProfiler.h>

namespace {
    struct Args {
        SystemComponentModel* system;
        float dt;
    };
}

SystemComponentModel::SystemComponentModel(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto system = static_cast<SystemComponentModel&>(inSystem);

        //{
        //    Engine::block_profiler blk(std::cout, "anim time: ");

            system.m_ModelInstancesWithAnimations.clear();
            //get the instances that are acually running animations
            system.forEach<Args>([](Args args, Entity entity, ComponentModel* componentModel) {
                for (auto& modelInstance : *componentModel) {
                    if (!modelInstance->getRunningAnimations().empty()) {
                        args.system->m_ModelInstancesWithAnimations.push_back(modelInstance.get());
                    }
                }
            }, Args{ std::addressof(system), dt }, SystemExecutionPolicy::Normal);
            //and then process them in parallel
            Engine::priv::threading::addJobSplitVectored([dt](size_t i, int32_t jobIndex, std::vector<ModelInstance*>& modelInstances) {
                modelInstances[i]->m_Animations.update(dt);
            }, true, true, system.m_ModelInstancesWithAnimations.size(), 0, system.m_ModelInstancesWithAnimations);
        //}
    });
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto* componentModel = static_cast<ComponentModel*>(component);
        Engine::priv::ComponentModel_Functions::CalculateRadius(*componentModel);
    });
}