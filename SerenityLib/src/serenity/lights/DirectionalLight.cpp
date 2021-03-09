#include <serenity/lights/DirectionalLight.h>
#include <serenity/ecs/components/ComponentBody.h>
#include <serenity/utils/Utils.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/Engine.h>

DirectionalLight::DirectionalLight(Scene* scene, const glm::vec3& direction)
    : SunLight{ scene, glm::vec3(0.0f), LightType::Directional }
{
    getComponent<ComponentBody>()->alignTo(direction);
}
DirectionalLight::~DirectionalLight() {
}

bool DirectionalLight::setShadowCaster(bool castsShadow) noexcept {
    if ((m_IsShadowCaster && castsShadow == false) || (!m_IsShadowCaster && castsShadow == true)) {
        m_IsShadowCaster = castsShadow;
        Engine::priv::Core::m_Engine->m_RenderModule.setShadowCaster(*this, castsShadow);
        return Engine::priv::PublicScene::SetLightShadowCaster<DirectionalLight>(*scene(), *this, castsShadow);
    }
    return false;
}