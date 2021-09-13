#include <serenity/renderer/culling/Culling.h>
#include <serenity/renderer/culling/SphereIntersectTest.h>
#include <serenity/renderer/culling/DistanceTest.h>
#include <serenity/math/Engine_Math.h>

#include <serenity/scene/Camera.h>
#include <serenity/scene/Viewport.h>
#include <serenity/model/ModelInstance.h>

namespace {
    template <class ... ARGS>
    bool all_passed(const ARGS& ... args) noexcept {
        const auto a0 = ((0, ..., args));
        return ((args == a0) && ...);
    }
    template<class FUNC>
    void cull_impl(const std::vector<ModelInstance*>& inModelInstances, const Viewport* const viewport, const auto& cameraOrMatrixOrPlanes, const Camera* const camera, FUNC&& resultFunc) {
        for (const auto& modelInstancePtr : inModelInstances) {
            const auto parent            = modelInstancePtr->getParent();
            const auto transform         = parent.getComponent<ComponentTransform>();
            const auto model             = parent.getComponent<ComponentModel>();
            const bool is_valid_viewport = viewport ? Engine::priv::PublicModelInstance::IsViewportValid(*modelInstancePtr, *viewport) : true;
            if (is_valid_viewport && transform) {
                const bool passedVisibleTest = modelInstancePtr->isVisible();
                if (modelInstancePtr->isForceRendered()) {
                    modelInstancePtr->setPassedRenderCheck(passedVisibleTest);
                } else {
                    const glm_vec3 modelWorldPosition = transform->getWorldPosition() + glm_vec3{ modelInstancePtr->getPosition() };
                    const auto result = passedVisibleTest && resultFunc(modelWorldPosition, model, cameraOrMatrixOrPlanes, camera);
                    modelInstancePtr->setPassedRenderCheck(result);
                }
            } else {
                modelInstancePtr->setPassedRenderCheck(false);
            }
        }
    }
};

void Engine::priv::Culling::cull(const Camera* const camera, const Viewport* const viewport, const std::vector<ModelInstance*>& inModelInstances) {
    const auto planes = Engine::priv::Culling::getFrustumPlanes(*camera);
    cull_impl(inModelInstances, viewport, planes, camera, [](const glm_vec3& modelWorldPosition, ComponentModel* model, const auto& cameraOrMatrixOrPlanes, const Camera* const camera) {
        return all_passed(
            Engine::priv::Culling::sphereIntersectTest(modelWorldPosition, model->getRadius(), cameraOrMatrixOrPlanes) != 0,
            Engine::priv::Culling::distanceTest(modelWorldPosition, model->getRadius(), camera)
        );
    });
}
void Engine::priv::Culling::cull(const glm::mat4& viewProjMatrix, const Viewport* const viewport, const std::vector<ModelInstance*>& inModelInstances) {
    const auto planes = Engine::priv::Culling::getFrustumPlanes(viewProjMatrix);
    cull_impl(inModelInstances, viewport, planes, nullptr, [](const glm_vec3& modelWorldPosition, ComponentModel* model, const auto& cameraOrMatrixOrPlanes, const Camera* const camera) {
        return all_passed(
            Engine::priv::Culling::sphereIntersectTest(modelWorldPosition, model->getRadius(), cameraOrMatrixOrPlanes) != 0
        );
    });
}
void Engine::priv::Culling::cull(const glm::mat4& viewProjMatrix, const std::vector<ModelInstance*>& inModelInstances) {
    const auto planes = Engine::priv::Culling::getFrustumPlanes(viewProjMatrix);
    cull_impl(inModelInstances, nullptr, planes, nullptr, [](const glm_vec3& modelWorldPosition, ComponentModel* model, const auto& cameraOrMatrixOrPlanes, const Camera* const camera) {
        return all_passed(
            Engine::priv::Culling::sphereIntersectTest(modelWorldPosition, model->getRadius(), cameraOrMatrixOrPlanes) != 0
        );
    });
}
void Engine::priv::Culling::cull(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const std::vector<ModelInstance*>& inModelInstances) {
    Engine::priv::Culling::cull(projMatrix * viewMatrix, inModelInstances);
}
void Engine::priv::Culling::cull(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const Viewport* const viewport, const std::vector<ModelInstance*>& inModelInstances) {
    Engine::priv::Culling::cull(projMatrix * viewMatrix, viewport, inModelInstances);
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
const std::array<glm::vec4, 6>& Engine::priv::Culling::getFrustumPlanes(const Camera& camera) {
    return camera.getComponent<ComponentCamera>()->getFrustrumPlanes();
}
std::array<glm::vec4, 6> Engine::priv::Culling::getFrustumPlanes(const glm::mat4& viewProjectionMatrix) {
    std::array<glm::vec4, 6> planes;
    Engine::Math::extractViewFrustumPlanesHartmannGribbs(viewProjectionMatrix, planes);
    return planes;
}