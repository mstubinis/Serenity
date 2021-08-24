#include <serenity/lights/SunLight.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/utils/Utils.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/Engine.h>

SunLight::SunLight(Scene* scene, decimal x, decimal y, decimal z, LightType type)
    : EntityBody{ (!scene) ? *Engine::Resources::getCurrentScene() : *scene }
    , m_Type{ type }
{
    addComponent<ComponentTransform>();
    setPosition(x, y, z);
}
SunLight::SunLight(Scene* scene, const glm_vec3& pos, LightType type)
    : SunLight{ scene, pos.x, pos.y, pos.z, type }
{}
glm_vec3 SunLight::getPosition() const { return getComponent<ComponentTransform>()->getPosition(); }
void SunLight::setPosition(decimal x, decimal y, decimal z) { getComponent<ComponentTransform>()->setPosition(x, y, z); }
void SunLight::setPosition(decimal position) { getComponent<ComponentTransform>()->setPosition(position, position, position); }
void SunLight::setPosition(const glm_vec3& position) { getComponent<ComponentTransform>()->setPosition(position); }

bool SunLight::isShadowCaster() const noexcept {
    return m_IsShadowCaster; 
}
bool SunLight::setShadowCaster(bool castsShadow) noexcept {
    if ((m_IsShadowCaster && castsShadow == false) || (!m_IsShadowCaster && castsShadow == true)) {
        m_IsShadowCaster = castsShadow;
        Engine::priv::Core::m_Engine->m_RenderModule.setShadowCaster(*this, castsShadow);
        return Engine::priv::PublicScene::SetLightShadowCaster<SunLight>(*scene(), *this, castsShadow);
    }
    return false;
}