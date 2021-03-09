#include <serenity/ecs/systems/SystemComponentCamera.h>
#include <serenity/ecs/components/ComponentCamera.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/math/Engine_Math.h>

SystemComponentCamera::SystemComponentCamera(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& system, const float dt, Scene& scene) {
        auto& pool       = system.getComponentPool<ComponentCamera>(0);
        auto& components = pool.data();
        auto lamda_update_component = [&](ComponentCamera& b, size_t i, size_t k) {
            Engine::Math::extractViewFrustumPlanesHartmannGribbs(b.getViewProjection(), b.getFrustrumPlanes());//update frustrum planes 
        };
        if (components.size() < 50) {
            for (size_t i = 0; i < components.size(); ++i) {
                lamda_update_component(components[i], i, 0);
            }
        }else{
            Engine::priv::threading::addJobSplitVectored(lamda_update_component, components, true, 0);
        }
    });
}
