#include <serenity/ecs/systems/SystemComponentCamera.h>
#include <serenity/ecs/components/ComponentCamera.h>
#include <serenity/math/Engine_Math.h>

SystemComponentCamera::SystemComponentCamera(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    /*
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system     = static_cast<SystemComponentCamera&>(inSystem);
        system.forEach([](Entity entity, ComponentCamera* b) {
            Engine::Math::extractViewFrustumPlanesHartmannGribbs(b->getViewProjection(), b->m_FrustumPlanes);
        }, SystemExecutionPolicy::ParallelWait);
    });
    */
}
