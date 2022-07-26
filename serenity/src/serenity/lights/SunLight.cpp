#include <serenity/lights/SunLight.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/utils/Utils.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/Engine.h>

SunLight::SunLight(Scene* scene, decimal x, decimal y, decimal z)
    : Entity{ (!scene) ? *Engine::Resources::getCurrentScene() : *scene }
    , LightBaseData<SunLight>{ LightType::Sun }
{
    addComponent<ComponentTransform>(glm_vec3{ x, y, z });
    //setPosition(x, y, z);
}
SunLight::SunLight(Scene* scene, const glm_vec3& pos)
    : SunLight{ scene, pos.x, pos.y, pos.z }
{}
bool SunLight::setShadowCaster(bool castsShadow, uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType frustumType, float nearFactor, float farFactor) noexcept {
    auto setupRenderingAPIStuff = [=]() -> bool {
        auto& renderer = Engine::getRenderer();
        renderer.setShadowCaster(*this, castsShadow, shadowMapWidth, shadowMapHeight, frustumType, nearFactor, farFactor);
        if ((m_IsShadowCaster && castsShadow == false) || (!m_IsShadowCaster && castsShadow == true)) {
            m_IsShadowCaster = castsShadow;
            return Engine::priv::PublicScene::SetLightShadowCaster<SunLight>(*scene(), *this, castsShadow);
        }
        return false;
    };
    Engine::priv::threading::addJobWithPostCallback([]() {}, std::move(setupRenderingAPIStuff));
    return false;
}
glm_vec3 SunLight::getPosition() const noexcept {
    return getComponent<ComponentTransform>()->getPosition();
}
void SunLight::setPosition(decimal x, decimal y, decimal z) noexcept {
    getComponent<ComponentTransform>()->setPosition(x, y, z);
}
void SunLight::setPosition(const glm_vec3& position) noexcept {
    setPosition(position.x, position.y, position.z);
}