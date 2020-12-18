#include <serenity/core/engine/lights/RodLight.h>
#include <serenity/core/engine/utils/Utils.h>
#include <serenity/ecs/ComponentBody.h>
#include <serenity/core/engine/scene/Scene.h>

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