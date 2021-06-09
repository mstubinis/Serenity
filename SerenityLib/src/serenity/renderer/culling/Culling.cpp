#include <serenity/renderer/culling/Culling.h>
#include <serenity/renderer/culling/SphereIntersectTest.h>
#include <serenity/renderer/culling/DistanceTest.h>

#include <serenity/scene/Camera.h>
#include <serenity/scene/Viewport.h>
#include <serenity/model/ModelInstance.h>

void Engine::priv::Culling::cull(const Camera* const camera, const Viewport* const viewport, const std::vector<ModelInstance*>& inModelInstances) {
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
                const glm_vec3 modelWorldPosition  = transform->getWorldPosition() + glm_vec3{ modelInstancePtr->getPosition() };
                const bool passedSphereTest        = Engine::priv::Culling::sphereIntersectTest(modelWorldPosition, model->getRadius(), camera) != 0;
                const bool passedDistanceTest      = Engine::priv::Culling::distanceTest(modelWorldPosition, model->getRadius(), camera);
                const bool result                  = passedVisibleTest && passedSphereTest && passedDistanceTest;
                modelInstancePtr->setPassedRenderCheck(result);
            }
        } else {
            modelInstancePtr->setPassedRenderCheck(false);
        }
    }
}
void Engine::priv::Culling::cull(const glm::mat4& viewProjMatrix, const Viewport* const viewport, const std::vector<ModelInstance*>& inModelInstances) {
    for (const auto& modelInstancePtr : inModelInstances) {
        const auto parent            = modelInstancePtr->getParent();
        const auto transform         = parent.getComponent<ComponentTransform>();
        const auto model             = parent.getComponent<ComponentModel>();
        const bool is_valid_viewport = viewport ? PublicModelInstance::IsViewportValid(*modelInstancePtr, *viewport) : true;
        if (is_valid_viewport && transform) {
            const bool passedVisibleTest = modelInstancePtr->isVisible();
            if (modelInstancePtr->isForceRendered()) {
                modelInstancePtr->setPassedRenderCheck(passedVisibleTest);
            } else {
                const glm_vec3 modelWorldPosition = transform->getWorldPosition() + glm_vec3{ modelInstancePtr->getPosition() };
                const bool passedSphereTest       = Engine::priv::Culling::sphereIntersectTest(modelWorldPosition, model->getRadius(), viewProjMatrix) != 0;
                const bool result                 = passedVisibleTest && passedSphereTest;
                modelInstancePtr->setPassedRenderCheck(result);
            }
        } else {
            modelInstancePtr->setPassedRenderCheck(false);
        }
    }
}
void Engine::priv::Culling::cull(const glm::mat4& viewProjMatrix, const std::vector<ModelInstance*>& inModelInstances) {
    for (const auto& modelInstancePtr : inModelInstances) {
        const auto parent    = modelInstancePtr->getParent();
        const auto transform = parent.getComponent<ComponentTransform>();
        const auto model     = parent.getComponent<ComponentModel>();
        if (transform) {
            const bool passedVisibleTest = modelInstancePtr->isVisible();
            if (modelInstancePtr->isForceRendered()) {
                modelInstancePtr->setPassedRenderCheck(passedVisibleTest);
            } else {
                const glm_vec3 modelWorldPosition = transform->getWorldPosition() + glm_vec3{ modelInstancePtr->getPosition() };
                const bool passedSphereTest       = Engine::priv::Culling::sphereIntersectTest(modelWorldPosition, model->getRadius(), viewProjMatrix) != 0;
                const bool result                 = passedVisibleTest && passedSphereTest;
                modelInstancePtr->setPassedRenderCheck(result);
            }
        } else {
            modelInstancePtr->setPassedRenderCheck(false);
        }
    }
}
void Engine::priv::Culling::cull(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const std::vector<ModelInstance*>& inModelInstances) {
    glm::mat4 viewProj = projMatrix * viewMatrix;
    Engine::priv::Culling::cull(viewProj, inModelInstances);
}
void Engine::priv::Culling::cull(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const Viewport* const viewport, const std::vector<ModelInstance*>& inModelInstances) {
    glm::mat4 viewProj = projMatrix * viewMatrix;
    Engine::priv::Culling::cull(viewProj, viewport, inModelInstances);
}
void Engine::priv::Culling::cull(const Camera* const camera, const std::vector<ModelInstance*>& inModelInstances) {
    Engine::priv::Culling::cull(camera, nullptr, inModelInstances);
}
void Engine::priv::Culling::cull(const Viewport* const viewport, const std::vector<ModelInstance*>& inModelInstances) {
    Engine::priv::Culling::cull(nullptr, viewport, inModelInstances);
}
void Engine::priv::Culling::cull(const std::vector<ModelInstance*>& inModelInstances) {
    Engine::priv::Culling::cull(nullptr, nullptr, inModelInstances);
}