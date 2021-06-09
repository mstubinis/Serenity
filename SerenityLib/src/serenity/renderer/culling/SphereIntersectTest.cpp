#include <serenity/renderer/culling/SphereIntersectTest.h>
#include <serenity/ecs/components/ComponentCamera.h>
#include <serenity/scene/Camera.h>
#include <serenity/math/Engine_Math.h>

int Engine::priv::Culling::SphereIntersectTest::sphereIntersectTest(const glm_vec3& position, const float radius, const std::array<glm::vec4, 6>& frustumPlanes) {
    int res         = 1; // inside the viewing frustum
    const auto zero = (decimal)0.0;
    const auto two  = (decimal)2.0;
    if (radius <= zero) {
        return 0;
    }
    for (size_t i = 0; i < frustumPlanes.size(); ++i) {
        const auto d = frustumPlanes[i].x * position.x + frustumPlanes[i].y * position.y + frustumPlanes[i].z * position.z + frustumPlanes[i].w;
        if (d > radius * two) {
            return 0; // outside the viewing frustrum
        } else if (d > zero) {
            res = 2;  // intersecting the viewing plane
        }
    }
    return res;
}
int Engine::priv::Culling::SphereIntersectTest::sphereIntersectTest(const glm_vec3& position, const float radius, const Camera& camera) {
    return Engine::priv::Culling::SphereIntersectTest::sphereIntersectTest(position, radius, camera.getComponent<ComponentCamera>()->m_FrustumPlanes);
}
int Engine::priv::Culling::SphereIntersectTest::sphereIntersectTest(const glm_vec3& position, const float radius, const ComponentCamera& componentCamera) {
    return Engine::priv::Culling::SphereIntersectTest::sphereIntersectTest(position, radius, componentCamera.m_FrustumPlanes);
}
int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const std::array<glm::vec4, 6>& frustumPlanes) {
    return Engine::priv::Culling::SphereIntersectTest::sphereIntersectTest(position, radius, frustumPlanes);
}
int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const Camera& camera) {
    return Engine::priv::Culling::SphereIntersectTest::sphereIntersectTest(position, radius, camera);
}
int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const Camera* const camera) {
    return camera ? Engine::priv::Culling::SphereIntersectTest::sphereIntersectTest(position, radius, *camera) : 1;
}
int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const ComponentCamera& componentCamera) {
    return Engine::priv::Culling::SphereIntersectTest::sphereIntersectTest(position, radius, componentCamera);
}


int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;
    return Engine::priv::Culling::sphereIntersectTest(position, radius, viewProjMatrix);
}
int Engine::priv::Culling::sphereIntersectTest(const glm_vec3& position, const float radius, const glm::mat4& viewProjectionMatrix) {
    std::array<glm::vec4, 6> planes;
    Engine::Math::extractViewFrustumPlanesHartmannGribbs(viewProjectionMatrix, planes);
    return Engine::priv::Culling::sphereIntersectTest(position, radius, planes);
}