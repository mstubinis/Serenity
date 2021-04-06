#include <serenity/lights/RodLight.h>
#include <serenity/utils/Utils.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/scene/Scene.h>

RodLight::RodLight(Scene* scene, const glm_vec3& pos, float rodLength)
    : PointLight{ scene, LightType::Rod, pos }
{
    setRodLength(rodLength);

    auto body = getComponent<ComponentTransform>();
    if (body) { //evil, but needed. find out why...
        body->setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
    }
}
RodLight::~RodLight() {
}
float RodLight::calculateCullingRadius() {
    float res = PointLight::calculateCullingRadius();
    auto body = getComponent<ComponentTransform>();
    body->setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
    return res;
}
void RodLight::setRodLength(float length) {
    m_RodLength = length;
    auto body = getComponent<ComponentTransform>();
    body->setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
}