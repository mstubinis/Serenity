#include <core/engine/lights/ProjectionLight.h>
#include <ecs/ComponentBody.h>
#include <core/engine/utils/Utils.h>

using namespace std;
using namespace Engine;

ProjectionLight::ProjectionLight(Texture* texture, const glm::vec3& direction, Scene* scene) : SunLight(glm::vec3(0.0f), LightType::Projection, scene) {
    getComponent<ComponentBody>()->alignTo(direction);
    setTexture(texture);
    if (m_Type == LightType::Projection) {
        auto& projLights = priv::InternalScenePublicInterface::GetProjectionLights(*scene);
        projLights.push_back(this);
    }
}
ProjectionLight::ProjectionLight(Handle textureHandle, const glm::vec3& direction, Scene* scene) : SunLight(glm::vec3(0.0f), LightType::Projection, scene) {
    getComponent<ComponentBody>()->alignTo(direction);
    setTexture(textureHandle);
    if (m_Type == LightType::Projection) {
        auto& projLights = priv::InternalScenePublicInterface::GetProjectionLights(*scene);
        projLights.push_back(this);
    }
}
void ProjectionLight::recalculateViewMatrix() noexcept {
    auto* body   = getComponent<ComponentBody>();
    m_ViewMatrix = glm::lookAt(body->getPositionRender(), body->getPositionRender() + glm::vec3(0, 0, -1), glm::vec3(body->up()));
}
void ProjectionLight::internal_recalculate_projection_matrix() noexcept {
    m_ProjectionMatrix = glm::perspective(m_FOV, m_AspectRatio, m_Near, m_Far);
}
void ProjectionLight::setTexture(Handle textureHandle) noexcept {
    m_Texture = textureHandle.get<Texture>();
}
void ProjectionLight::free() noexcept {
    Entity::destroy();
    removeFromVector(priv::InternalScenePublicInterface::GetProjectionLights(scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(scene()), this);
}