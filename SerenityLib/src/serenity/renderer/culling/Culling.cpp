#include <serenity/renderer/culling/Culling.h>
#include <serenity/renderer/culling/SphereIntersectTest.h>
#include <serenity/renderer/culling/DistanceTest.h>

#include <serenity/scene/Camera.h>
#include <serenity/scene/Viewport.h>
#include <serenity/model/ModelInstance.h>

void Engine::priv::Culling::cull(Camera* camera, Viewport* viewport, const std::vector<ModelInstance*>& inModelInstances) {
    for (const auto& modelInstancePtr : inModelInstances) {
        const auto parent              = modelInstancePtr->getParent();
        const auto transform           = parent.getComponent<ComponentTransform>();
        const auto model               = parent.getComponent<ComponentModel>();
        const bool is_valid_viewport   = viewport ? PublicModelInstance::IsViewportValid(*modelInstancePtr, *viewport) : true;
        if (is_valid_viewport && transform) {
            const bool passedVisibleTest = modelInstancePtr->isVisible();
            if (modelInstancePtr->isForceRendered()) {
                modelInstancePtr->setPassedRenderCheck(passedVisibleTest);
            } else {
                const float modelRadius            = model->getRadius();
                const glm_vec3 modelWorldPosition  = transform->getWorldPosition() + glm_vec3{ modelInstancePtr->getPosition() };
                const bool passedSphereTest        = Engine::priv::Culling::sphereIntersectTest(modelWorldPosition, modelRadius, camera) != 0;
                const bool passedDistanceTest      = Engine::priv::Culling::distanceTest(modelWorldPosition, modelRadius, camera);
                const bool result                  = passedVisibleTest && passedSphereTest && passedDistanceTest;
                modelInstancePtr->setPassedRenderCheck(result);
            }
        } else {
            modelInstancePtr->setPassedRenderCheck(false);
        }
    }
}