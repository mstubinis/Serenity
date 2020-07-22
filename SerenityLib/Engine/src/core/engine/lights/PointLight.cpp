#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/PointLight.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/resources/Engine_Resources.h>

#include <ecs/ComponentBody.h>

using namespace Engine;

constexpr std::array<std::tuple<float, float, float>, (size_t)LightRange::_TOTAL> LIGHT_RANGES{ {
    std::make_tuple(1.0f, 0.7f, 1.8f),
    std::make_tuple(1.0f, 0.35f, 0.44f),
    std::make_tuple(1.0f, 0.22f, 0.20f),
    std::make_tuple(1.0f, 0.14f, 0.07f),
    std::make_tuple(1.0f, 0.09f, 0.032f),
    std::make_tuple(1.0f, 0.07f, 0.017f),
    std::make_tuple(1.0f, 0.045f, 0.0075f),
    std::make_tuple(1.0f, 0.027f, 0.0028f),
    std::make_tuple(1.0f, 0.022f, 0.0019f),
    std::make_tuple(1.0f, 0.014f, 0.0007f),
    std::make_tuple(1.0f, 0.007f, 0.0002f),
    std::make_tuple(1.0f, 0.0014f, 0.000007f),
} };

PointLight::PointLight(LightType type, const glm_vec3& pos, Scene* scene) : SunLight(pos, type, scene) {
    m_CullingRadius = calculateCullingRadius();

    if (m_Type == LightType::Point) {
        auto& ptLights = priv::InternalScenePublicInterface::GetPointLights(*scene);
        ptLights.push_back(this);
    }
}
PointLight::PointLight(const glm_vec3& pos, Scene* scene) : PointLight(LightType::Point, pos, scene) {

}
void PointLight::free() noexcept {
    Entity::destroy();
    removeFromVector(priv::InternalScenePublicInterface::GetPointLights(scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(scene()), this);
}
float PointLight::calculateCullingRadius() {
    float lightMax = Math::Max(m_Color.x, m_Color.y, m_Color.z);
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
void PointLight::setConstant(float c) { 
    m_C = c; 
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setLinear(float l) { 
    m_L = l;
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setExponent(float e) { 
    m_E = e;
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setAttenuation(float c, float l, float e) { 
    m_C = c;
    m_L = l;
    m_E = e;
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setAttenuation(LightRange r) { 
    const auto& d = LIGHT_RANGES[(unsigned int)r];
    PointLight::setAttenuation(std::get<0>(d), std::get<1>(d), std::get<2>(d) );
}
void PointLight::setAttenuationModel(LightAttenuation model) {
    m_AttenuationModel = model; 
    m_CullingRadius = calculateCullingRadius();
}