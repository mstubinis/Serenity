#include <serenity/lights/PointLight.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/utils/Utils.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/scene/Scene.h>

PointLight::PointLight(Scene* scene, decimal x, decimal y, decimal z)
    : Entity{ (!scene) ? *Engine::Resources::getCurrentScene() : *scene }
    , LightBaseData<PointLight>{ LightType::Point }
{
    addComponent<ComponentTransform>();
    getComponent<ComponentTransform>()->setPosition(x, y, z);
    calculateCullingRadius();
}
PointLight::PointLight(Scene* scene, const glm_vec3& pos)
    : PointLight{ scene, pos.x, pos.y, pos.z }
{}

void PointLight::calculateCullingRadius() {
    LightAttenuationData::calculateCullingRadius(getComponent<ComponentTransform>(), m_Color);
}
void PointLight::setConstant(float constant) {
    m_C = constant;
    calculateCullingRadius();
}
void PointLight::setLinear(float linear) {
    m_L = linear;
    calculateCullingRadius();
}
void PointLight::setExponent(float exponent) { 
    m_E = exponent;
    calculateCullingRadius();
}
void PointLight::setAttenuation(float constant, float linear, float exponent) {
    m_C = constant;
    m_L = linear;
    m_E = exponent;
    calculateCullingRadius();
}
void PointLight::setAttenuation(LightRange range) { 
    LightAttenuationData::setAttenuation(getComponent<ComponentTransform>(), range, m_Color);
}
void PointLight::setAttenuationModel(LightAttenuation model) {
    if (m_AttenuationModel != model) {
        m_AttenuationModel = model;
        calculateCullingRadius();
    }
}