#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/PointLight.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/utils/Utils.h>
#include <ecs/ComponentBody.h>
#include <core/engine/scene/Scene.h>

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

PointLight::PointLight(Scene* scene, LightType type, const glm_vec3& pos)
    : SunLight{ scene, pos, type }
    , m_CullingRadius{ calculateCullingRadius() }
{}
PointLight::PointLight(Scene* scene, const glm_vec3& pos)
    : PointLight{ scene, LightType::Point, pos }
{}
PointLight::~PointLight() {
}
void PointLight::destroy() noexcept {
    Entity::destroy();
    Scene* scene_ptr = scene();
    if (scene_ptr) {
        removeFromVector(Engine::priv::InternalScenePublicInterface::GetPointLights(*scene_ptr), this);
    }
}
float PointLight::calculateCullingRadius() {
    float lightMax = Engine::Math::Max(m_Color.x, m_Color.y, m_Color.z);
    float radius = 0;
    //if(m_AttenuationModel == LightAttenuation::Constant_Linear_Exponent){
          radius = (-m_L + glm::sqrt(m_L * m_L - 4.0f * m_E * (m_C - (256.0f / 5.0f) * lightMax))) / (2.0f * m_E);
    //}
    //else if(m_AttenuationModel == LightAttenuation::Distance_Squared){
    //    radius = glm::sqrt(lightMax * (256.0f / 5.0f)); // 51.2f   is   256.0f / 5.0f
    //}
    //else if(m_AttenuationModel == LightAttenuation::Distance){
    //    radius = (lightMax * (256.0f / 5.0f));
    //}
    auto& body = *getComponent<ComponentBody>();
    body.setScale(radius, radius, radius);
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
    m_AttenuationModel = model; 
    m_CullingRadius = calculateCullingRadius();
}