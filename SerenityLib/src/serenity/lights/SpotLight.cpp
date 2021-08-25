#include <serenity/lights/SpotLight.h>
#include <serenity/utils/Utils.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/scene/Scene.h>
#include <serenity/resources/Engine_Resources.h>

SpotLight::SpotLight(Scene* scene, decimal x, decimal y, decimal z, const glm_vec3& direction, float innerCutoffInDegrees, float outerCutoffInDegrees)
    : EntityBody{ (!scene) ? *Engine::Resources::getCurrentScene() : *scene }
    , LightBaseData<SpotLight>{ LightType::Spot }
{
    addComponent<ComponentTransform>();
    setPosition(x, y, z);
    setCutoffDegrees(innerCutoffInDegrees);
    setCutoffOuterDegrees(outerCutoffInDegrees);
    setDirection(direction);
}
SpotLight::SpotLight(Scene* scene, const glm_vec3& position, const glm_vec3& direction, float innerCutoffInDegrees, float outerCutoffInDegrees)
    : SpotLight{ scene, position.x, position.y, position.z, direction, innerCutoffInDegrees, outerCutoffInDegrees }
{}
void SpotLight::setDirection(float xDir, float yDir, float zDir) noexcept {
    getComponent<ComponentTransform>()->alignTo(xDir, yDir, zDir);
}
void SpotLight::setDirection(const glm::vec3& direction) noexcept {
    setDirection(direction.x, direction.y, direction.z);
}
void SpotLight::calculateCullingRadius() {
    LightAttenuationData::calculateCullingRadius(getComponent<ComponentTransform>(), m_Color);
}
void SpotLight::setConstant(float constant) {
    m_C = constant;
    calculateCullingRadius();
}
void SpotLight::setLinear(float linear) {
    m_L = linear;
    calculateCullingRadius();
}
void SpotLight::setExponent(float exponent) {
    m_E = exponent;
    calculateCullingRadius();
}
void SpotLight::setAttenuation(float constant, float linear, float exponent) {
    m_C = constant;
    m_L = linear;
    m_E = exponent;
    calculateCullingRadius();
}
void SpotLight::setAttenuation(LightRange range) {
    LightAttenuationData::setAttenuation(getComponent<ComponentTransform>(), range, m_Color);
}
void SpotLight::setAttenuationModel(LightAttenuation model) {
    if (m_AttenuationModel != model) {
        m_AttenuationModel = model;
        calculateCullingRadius();
    }
}