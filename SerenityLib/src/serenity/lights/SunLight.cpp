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
    addComponent<ComponentTransform>();
    getComponent<ComponentTransform>()->setPosition(x, y, z);
}
SunLight::SunLight(Scene* scene, const glm_vec3& pos)
    : SunLight{ scene, pos.x, pos.y, pos.z }
{}
bool SunLight::setShadowCaster(bool castsShadow) noexcept {
    if ((m_IsShadowCaster && castsShadow == false) || (!m_IsShadowCaster && castsShadow == true)) {
        m_IsShadowCaster = castsShadow;
        Engine::priv::Core::m_Engine->m_RenderModule.setShadowCaster(*this, castsShadow);
        return Engine::priv::PublicScene::SetLightShadowCaster<SunLight>(*scene(), *this, castsShadow);
    }
    return false;
}