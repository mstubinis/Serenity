#include <serenity/lights/RodLight.h>
#include <serenity/utils/Utils.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/scene/Scene.h>
#include <serenity/resources/Engine_Resources.h>

RodLight::RodLight(Scene* scene, decimal x, decimal y, decimal z, float rodLength)
    : Entity{ (!scene) ? *Engine::Resources::getCurrentScene() : *scene }
    , LightBaseData<RodLight>{ LightType::Rod }
{
    addComponent<ComponentTransform>();
    getComponent<ComponentTransform>()->setPosition(x, y, z);
    setRodLength(rodLength);
}
RodLight::RodLight(Scene* scene, const glm_vec3& pos, float rodLength)
    : RodLight{ scene, pos.x, pos.y, pos.z, rodLength }
{}
void RodLight::recalculateTransformScale() {
    getComponent<ComponentTransform>()->setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
}
void RodLight::calculateCullingRadius() {
    LightAttenuationData::calculateCullingRadius(getComponent<ComponentTransform>(), m_Color);
    recalculateTransformScale();
}
void RodLight::setConstant(float constant) {
    m_C = constant;
    calculateCullingRadius();
}
void RodLight::setLinear(float linear) {
    m_L = linear;
    calculateCullingRadius();
}
void RodLight::setExponent(float exponent) {
    m_E = exponent;
    calculateCullingRadius();
}
void RodLight::setAttenuation(float constant, float linear, float exponent) {
    m_C = constant;
    m_L = linear;
    m_E = exponent;
    calculateCullingRadius();
}
void RodLight::setAttenuation(LightRange range) {
    LightAttenuationData::setAttenuation(getComponent<ComponentTransform>(), range, m_Color);
}
void RodLight::setAttenuationModel(LightAttenuation model) {
    if (m_AttenuationModel != model) {
        m_AttenuationModel = model;
        calculateCullingRadius();
    }
}
void RodLight::setRodLength(float length) {
    m_RodLength = length;
    calculateCullingRadius();
}