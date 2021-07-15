#include <serenity/lights/DirectionalLight.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/utils/Utils.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/Engine.h>

DirectionalLight::DirectionalLight(Scene* scene, const glm::vec3& direction)
    : DirectionalLight{ scene, direction.x, direction.y, direction.z }
{}
DirectionalLight::DirectionalLight(Scene* scene, float directionX, float directionY, float directionZ)
    : SunLight{ scene, glm::vec3(0.0f), LightType::Directional }
{
    setDirection(directionX, directionY, directionZ);
}
DirectionalLight::~DirectionalLight() {
}

bool DirectionalLight::setShadowCaster(bool castsShadow) noexcept {
    if ((m_IsShadowCaster && castsShadow == false) || (!m_IsShadowCaster && castsShadow == true)) {
        m_IsShadowCaster = castsShadow;
        auto& renderer = Engine::priv::Core::m_Engine->m_RenderModule;
        renderer.setShadowCaster(*this, castsShadow);
        return Engine::priv::PublicScene::SetLightShadowCaster<DirectionalLight>(*scene(), *this, castsShadow);
    }
    return false;
}
void DirectionalLight::setDirection(float directionX, float directionY, float directionZ) {
    if (m_IsShadowCaster) {
        auto& renderer = Engine::priv::Core::m_Engine->m_RenderModule;
        renderer.m_Pipeline->setShadowDirectionalLightDirection(*this, glm::vec3{ directionX, directionY, directionZ });
    }
    getComponent<ComponentTransform>()->alignTo(directionX, directionY, directionZ);
}
void DirectionalLight::setDirection(const glm::vec3& direction) {
    DirectionalLight::setDirection(direction.x, direction.y, direction.z);
}
glm::vec3 DirectionalLight::getDirection() const noexcept {
    return getComponent<ComponentTransform>()->getForward();
}