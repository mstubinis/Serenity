#include <serenity/renderer/culling/SphereIntersectTest.h>
#include <serenity/renderer/culling/Culling.h>
#include <serenity/ecs/components/ComponentCamera.h>
#include <serenity/scene/Camera.h>
#include <serenity/math/Engine_Math.h>

namespace {
    //returns 0 if outside the frustum, 1 if inside the frustum, and 2 if intersecting the frustum
    int internal_sphere_intersect_test(const glm_vec3& position, const float radius, const std::array<glm::vec4, 6>& frustumPlanes) {
        if (radius <= 0.0f) {
            return 0;
        }
        for (const auto& frustPlane : frustumPlanes) {
            const auto d = frustPlane.x * position.x + frustPlane.y * position.y + frustPlane.z * position.z + frustPlane.w;
            if (d > radius * 2.0f) {
                return 0; // outside the viewing frustrum
            } else if (d > 0.0f) {
                return 2;  // intersecting the viewing plane
            }
        }
        return 1; // inside the viewing frustum
    }
    int internal_sphere_intersect_test(const glm_vec3& position, const float radius, const Camera& camera) {
        return internal_sphere_intersect_test(position, radius, camera.getComponent<ComponentCamera>()->getFrustrumPlanes());
    }
    int internal_sphere_intersect_test(const glm_vec3& position, const float radius, const ComponentCamera& componentCamera) {
        return internal_sphere_intersect_test(position, radius, componentCamera.getFrustrumPlanes());
    }
}
int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const std::array<glm::vec4, 6>& frustumPlanes) {
    return internal_sphere_intersect_test(position, radius, frustumPlanes);
}
int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const Camera& camera) {
    return internal_sphere_intersect_test(position, radius, camera);
}
int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const Camera* const camera) {
    return camera ? internal_sphere_intersect_test(position, radius, *camera) : 1;
}
int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const ComponentCamera& componentCamera) {
    return internal_sphere_intersect_test(position, radius, componentCamera);
}


int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    return internal_sphere_intersect_test(position, radius, Engine::priv::Culling::getFrustumPlanes(projectionMatrix * viewMatrix));
}
int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const glm::mat4& viewProjectionMatrix) {
    return internal_sphere_intersect_test(position, radius, Engine::priv::Culling::getFrustumPlanes(viewProjectionMatrix));
}
