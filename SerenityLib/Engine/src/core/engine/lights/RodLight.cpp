#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/RodLight.h>
#include <core/engine/utils/Utils.h>
#include <ecs/ComponentBody.h>
#include <core/engine/scene/Scene.h>

RodLight::RodLight(Scene* scene, const glm_vec3& pos, float rodLength)
    : PointLight{ scene, LightType::Rod, pos }
{
    setRodLength(rodLength);

    auto body = getComponent<ComponentBody>();
    if (body) { //evil, but needed. find out why...
        body->setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
    }
}
RodLight::~RodLight() {
}
void RodLight::destroy() noexcept {
    Entity::destroy();
    Scene* scene_ptr = scene();
    if (scene_ptr) {
        removeFromVector(Engine::priv::InternalScenePublicInterface::GetRodLights(*scene_ptr), this);
        removeFromVector(Engine::priv::InternalScenePublicInterface::GetLights(*scene_ptr), this);
    }
}
float RodLight::calculateCullingRadius() {
    float res = PointLight::calculateCullingRadius();
    auto body = getComponent<ComponentBody>();
    body->setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
    return res;
}
void RodLight::setRodLength(float length) {
    m_RodLength = length;
    auto body = getComponent<ComponentBody>();
    body->setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
}