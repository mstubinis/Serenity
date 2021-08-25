#include <serenity/lights/LightIncludes.h>
#include <glm/glm.hpp>
#include <serenity/math/Engine_Math.h>
#include <serenity/ecs/components/ComponentTransform.h>

constexpr std::array<LightDefaultAttenuationData, (size_t)LightRange::_TOTAL> LIGHT_RANGES{ {
    { 1.0f, 0.7f, 1.8f },
    { 1.0f, 0.35f, 0.44f },
    { 1.0f, 0.22f, 0.20f },
    { 1.0f, 0.14f, 0.07f },
    { 1.0f, 0.09f, 0.032f },
    { 1.0f, 0.07f, 0.017f },
    { 1.0f, 0.045f, 0.0075f },
    { 1.0f, 0.027f, 0.0028f },
    { 1.0f, 0.022f, 0.0019f },
    { 1.0f, 0.014f, 0.0007f },
    { 1.0f, 0.007f, 0.0002f },
    { 1.0f, 0.0014f, 0.000007f },
} };

void LightAttenuationData::calculateCullingRadius(ComponentTransform* transform, const glm::vec4& color) {
    float lightMax = Engine::Math::Max(color.x, color.y, color.z);
    float almostDark = 256.0f / 5.0f;
    float radius = 0;
    switch (m_AttenuationModel) {
        case LightAttenuation::Constant_Linear_Exponent: {
            radius = (-m_L + glm::sqrt(m_L * m_L - 4.0f * m_E * (m_C - almostDark * lightMax))) / (2.0f * m_E);
            break;
        } case LightAttenuation::Distance_Squared: {
            radius = glm::sqrt(lightMax * almostDark);
            break;
        } case LightAttenuation::Distance: {
            radius = (lightMax * almostDark);
            break;
        } case LightAttenuation::Constant: {
            radius = (lightMax * almostDark);
            break;
        } case LightAttenuation::Distance_Radius_Squared: {
            //float rad = 1.0f;
            //attenuation = pow((Dist / rad) + 1.0, 2.0);
            radius = (lightMax * almostDark);
            break;
        }
    }
    transform->setScale(radius);
    m_CullingRadius = radius;
}

void LightAttenuationData::setConstant(ComponentTransform* transform, float constant, const glm::vec4& lightColor) {
    m_C = constant;
    calculateCullingRadius(transform, lightColor);
}
void LightAttenuationData::setLinear(ComponentTransform* transform, float linear, const glm::vec4& lightColor) {
    m_L = linear;
    calculateCullingRadius(transform, lightColor);
}
void LightAttenuationData::setExponent(ComponentTransform* transform, float exponent, const glm::vec4& lightColor) {
    m_E = exponent;
    calculateCullingRadius(transform, lightColor);
}
void LightAttenuationData::setAttenuation(ComponentTransform* transform, float constant, float linear, float exponent, const glm::vec4& lightColor) {
    m_C = constant;
    m_L = linear;
    m_E = exponent;
    calculateCullingRadius(transform, lightColor);
}
void LightAttenuationData::setAttenuation(ComponentTransform* transform, LightRange range, const glm::vec4& lightColor) {
    const auto& data = LIGHT_RANGES[static_cast<size_t>(range)];
    LightAttenuationData::setAttenuation(transform, data.constant, data.linear, data.exponent, lightColor);
}
void LightAttenuationData::setAttenuationModel(ComponentTransform* transform, LightAttenuation model, const glm::vec4& lightColor) {
    if (m_AttenuationModel != model) {
        m_AttenuationModel = model;
        calculateCullingRadius(transform, lightColor);
    }
}