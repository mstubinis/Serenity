#include <serenity/ecs/systems/SystemStepPhysics.h>
#include <serenity/system/Engine.h>

SystemStepPhysics::SystemStepPhysics(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& engine          = *Engine::priv::Core::m_Engine;
        float fixed_time_step = PHYSICS_MIN_STEP / float(Engine::Physics::getNumberOfStepsPerFrame());
        engine.m_DebugManager.stop_clock();
        engine.m_PhysicsModule.update(scene, ENGINE_FIXED_TIMESTEP_VALUE, 100, fixed_time_step);
        engine.m_DebugManager.calculate_physics();
    });
}
