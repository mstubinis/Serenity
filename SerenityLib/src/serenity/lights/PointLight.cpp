#include <serenity/lights/PointLight.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/utils/Utils.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/scene/Scene.h>

constexpr std::array<PointLightDefaultAttenuationData, (size_t)LightRange::_TOTAL> LIGHT_RANGES { {
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

PointLight::PointLight(Scene* scene, LightType type, decimal x, decimal y, decimal z)
    : SunLight{ scene, x, y, z, type }
    , m_CullingRadius{ calculateCullingRadius() }
{}
PointLight::PointLight(Scene* scene, LightType type, const glm_vec3& pos)
    : PointLight{ scene, type, pos.x, pos.y, pos.z }
{}
PointLight::PointLight(Scene* scene, decimal x, decimal y, decimal z)
    : PointLight{ scene, LightType::Point, x, y, z }
{}
PointLight::PointLight(Scene* scene, const glm_vec3& pos)
    : PointLight{ scene, LightType::Point, pos.x, pos.y, pos.z }
{}
float PointLight::calculateCullingRadius() {
    float lightMax   = Engine::Math::Max(m_Color.x, m_Color.y, m_Color.z);
    float almostDark = 256.0f / 5.0f;
    float radius     = 0;
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
    auto& transform = *getComponent<ComponentTransform>();
    transform.setScale(radius);
    return radius;
}
void PointLight::setConstant(float constant) {
    m_C = constant;
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setLinear(float linear) {
    m_L = linear;
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setExponent(float exponent) { 
    m_E = exponent;
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setAttenuation(float constant, float linear, float exponent) {
    m_C = constant;
    m_L = linear;
    m_E = exponent;
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setAttenuation(LightRange r) { 
    const auto& data = LIGHT_RANGES[(uint32_t)r];
    PointLight::setAttenuation(data.constant, data.linear, data.exponent);
}
void PointLight::setAttenuationModel(LightAttenuation model) {
    if (m_AttenuationModel != model) {
        m_AttenuationModel = model;
        m_CullingRadius    = calculateCullingRadius();
    }
}