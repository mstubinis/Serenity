#include <serenity/renderer/culling/PointIntersectTest.h>
#include <serenity/ecs/components/ComponentCamera.h>
#include <serenity/scene/Camera.h>

int Engine::priv::Culling::PointIntersectTest::pointIntersectTest(const glm_vec3& position, const std::array<glm::vec4, 6>& frustumPlanes) {
    const auto zero = (decimal)0.0;
    for (uint32_t i = 0; i < frustumPlanes.size(); ++i) {
        const auto d = frustumPlanes[i].x * position.x + frustumPlanes[i].y * position.y + frustumPlanes[i].z * position.z + frustumPlanes[i].w;
        if (d > zero) {
            return 0; //outside
        }
    }
    return 1; //inside
}
int Engine::priv::Culling::PointIntersectTest::pointIntersectTest(const glm_vec3& position, const Camera& camera) {
    return Engine::priv::Culling::PointIntersectTest::pointIntersectTest(position, camera.getComponent<ComponentCamera>()->m_FrustumPlanes);
}
int Engine::priv::Culling::PointIntersectTest::pointIntersectTest(const glm_vec3& position, const ComponentCamera& componentCamera) {
    return Engine::priv::Culling::PointIntersectTest::pointIntersectTest(position, componentCamera.m_FrustumPlanes);
}
int Engine::priv::Culling::pointIntersectTest(const glm_vec3& position, const std::array<glm::vec4, 6>& frustumPlanes) {
    return Engine::priv::Culling::PointIntersectTest::pointIntersectTest(position, frustumPlanes);
}
int Engine::priv::Culling::pointIntersectTest(const glm_vec3& position, const Camera& camera) {
    return Engine::priv::Culling::PointIntersectTest::pointIntersectTest(position, camera);
}
int Engine::priv::Culling::pointIntersectTest(const glm_vec3& position, const Camera* const camera) {
    return camera ? Engine::priv::Culling::PointIntersectTest::pointIntersectTest(position, *camera) : 1;
}
int Engine::priv::Culling::pointIntersectTest(const glm_vec3& position, const ComponentCamera& componentCamera) {
    return Engine::priv::Culling::PointIntersectTest::pointIntersectTest(position, componentCamera);
}