#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/DirectionalLight.h>
#include <ecs/ComponentBody.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/system/Engine.h>

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
        return Engine::priv::InternalScenePublicInterface::SetLightShadowCaster<DirectionalLight>(*scene(), *this, castsShadow);
    }
    return false;
}