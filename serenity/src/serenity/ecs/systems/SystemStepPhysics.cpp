#include <serenity/ecs/systems/SystemStepPhysics.h>
#include <serenity/system/Engine.h>

SystemStepPhysics::SystemStepPhysics(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        const float fixed_time_step     = PHYSICS_MIN_STEP / float(Engine::Physics::getNumberOfStepsPerFrame());
        constexpr const int maxSubSteps = 100;
        Engine::getPhysicsManager().update(scene, ENGINE_FIXED_TIMESTEP_VALUE, maxSubSteps, fixed_time_step);
    });
}
