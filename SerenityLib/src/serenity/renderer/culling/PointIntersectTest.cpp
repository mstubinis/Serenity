#include <serenity/renderer/culling/PointIntersectTest.h>
#include <serenity/ecs/components/ComponentCamera.h>
#include <serenity/scene/Camera.h>
#include <serenity/math/Engine_Math.h>

namespace {
    int internal_pointIntersectTest(const glm_vec3& position, const std::array<glm::vec4, 6>& frustumPlanes) {
        for (const auto& frustPlane : frustumPlanes) {
            const auto d = frustPlane.x * position.x + frustPlane.y * position.y + frustPlane.z * position.z + frustPlane.w;
            if (d > decimal(0.0)) {
                return 0; //outside
            }
        }
        return 1; //inside
    }
    int internal_pointIntersectTest(const glm_vec3& position, const Camera& camera) {
        return internal_pointIntersectTest(position, camera.getComponent<ComponentCamera>()->getFrustrumPlanes());
    }
    int internal_pointIntersectTest(const glm_vec3& position, const ComponentCamera& componentCamera) {
        return internal_pointIntersectTest(position, componentCamera.getFrustrumPlanes());
    }
}

int Engine::priv::Culling::pointIntersectTest(const glm_vec3& position, const std::array<glm::vec4, 6>& frustumPlanes) {
    return internal_pointIntersectTest(position, frustumPlanes);
}
int Engine::priv::Culling::pointIntersectTest(const glm_vec3& position, const Camera& camera) {
    return internal_pointIntersectTest(position, camera);
}
int Engine::priv::Culling::pointIntersectTest(const glm_vec3& position, const Camera* const camera) {
    return camera ? internal_pointIntersectTest(position, *camera) : 1;
}
int Engine::priv::Culling::pointIntersectTest(const glm_vec3& position, const ComponentCamera& componentCamera) {
    return internal_pointIntersectTest(position, componentCamera);
}



int Engine::priv::Culling::pointIntersectTest(const glm_vec3& position, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    const glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;
    return pointIntersectTest(position, viewProjMatrix);
}
int Engine::priv::Culling::pointIntersectTest(const glm_vec3& position, const glm::mat4& viewProjectionMatrix) {
    std::array<glm::vec4, 6> planes;
    Engine::Math::extractViewFrustumPlanesHartmannGribbs(viewProjectionMatrix, planes);
    return pointIntersectTest(position, planes);
}