#include <serenity/lights/RodLight.h>
#include <serenity/utils/Utils.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/scene/Scene.h>

RodLight::RodLight(Scene* scene, const glm_vec3& pos, float rodLength)
    : PointLight{ scene, LightType::Rod, pos }
{
    setRodLength(rodLength);
}
RodLight::~RodLight() {
}
void RodLight::recalculateTransformScale() {
    auto transform = getComponent<ComponentTransform>();
    transform->setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
}
void RodLight::calculateCullingRadius() {
    PointLight::calculateCullingRadius();
    recalculateTransformScale();
}
void RodLight::setRodLength(float length) {
    m_RodLength = length;
    calculateCullingRadius();
}