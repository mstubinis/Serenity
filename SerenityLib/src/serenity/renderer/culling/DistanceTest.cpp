#include <serenity/renderer/culling/DistanceTest.h>
#include <serenity/scene/Camera.h>
#include <serenity/model/ModelInstance.h>

bool Engine::priv::Culling::distanceTest(const glm_vec3& objectPosition, const float objectRadius, const glm_vec3& cameraPosition) {
    const decimal comparison = decimal(objectRadius) * ModelInstance::getGlobalDistanceFactor();
    return glm::distance2(objectPosition, cameraPosition) <= comparison * comparison;
}
bool Engine::priv::Culling::distanceTest(const glm_vec3& objectPosition, const float objectRadius, const Camera& camera) {
    return Engine::priv::Culling::distanceTest(objectPosition, objectRadius, camera.getPosition());
}
bool Engine::priv::Culling::distanceTest(const glm_vec3& objectPosition, const float objectRadius, const Camera* const camera) {
    return camera ? Engine::priv::Culling::distanceTest(objectPosition, objectRadius, camera->getPosition()) : true;
}