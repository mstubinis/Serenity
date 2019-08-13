#include <core/engine/lights/PointLight.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/resources/Engine_Resources.h>

#include <core/engine/resources/Engine_BuiltInResources.h>
#include <core/engine/mesh/Mesh.h>

using namespace Engine;
using namespace std;

vector<boost::tuple<float, float, float>> LIGHT_RANGES = []() {
    vector<boost::tuple<float, float, float>> m;
    m.resize(LightRange::_TOTAL, boost::make_tuple(0.0f, 0.0f, 0.0f));

    m[LightRange::_7]    = boost::make_tuple(1.0f, 0.7f, 1.8f);
    m[LightRange::_13]   = boost::make_tuple(1.0f, 0.35f, 0.44f);
    m[LightRange::_20]   = boost::make_tuple(1.0f, 0.22f, 0.20f);
    m[LightRange::_32]   = boost::make_tuple(1.0f, 0.14f, 0.07f);
    m[LightRange::_50]   = boost::make_tuple(1.0f, 0.09f, 0.032f);
    m[LightRange::_65]   = boost::make_tuple(1.0f, 0.07f, 0.017f);
    m[LightRange::_100]  = boost::make_tuple(1.0f, 0.045f, 0.0075f);
    m[LightRange::_160]  = boost::make_tuple(1.0f, 0.027f, 0.0028f);
    m[LightRange::_200]  = boost::make_tuple(1.0f, 0.022f, 0.0019f);
    m[LightRange::_325]  = boost::make_tuple(1.0f, 0.014f, 0.0007f);
    m[LightRange::_600]  = boost::make_tuple(1.0f, 0.007f, 0.0002f);
    m[LightRange::_3250] = boost::make_tuple(1.0f, 0.0014f, 0.000007f);

    return m;
}();

PointLight::PointLight(const LightType::Type type, const glm::vec3 pos, Scene* scene):SunLight(pos, type, scene) {
    m_C = 0.1f;
    m_L = 0.1f;
    m_E = 0.1f;
    m_AttenuationModel = LightAttenuation::Constant_Linear_Exponent;
    m_CullingRadius = calculateCullingRadius();

    if (m_Type == LightType::Point) {
        epriv::InternalScenePublicInterface::GetPointLights(*scene).push_back(this);
    }
}
PointLight::PointLight(const glm::vec3 pos, Scene* scene):PointLight(LightType::Point, pos, scene) {

}
PointLight::~PointLight() {
}
void PointLight::destroy() {
    EntityWrapper::destroy();
    removeFromVector(epriv::InternalScenePublicInterface::GetPointLights(m_Entity.scene()), this);
    removeFromVector(epriv::InternalScenePublicInterface::GetLights(m_Entity.scene()), this);
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
const float PointLight::getCullingRadius() const {
    return m_CullingRadius; 
}
const float PointLight::getConstant() const {
    return m_C; 
}
const float PointLight::getLinear() const {
    return m_L; 
}
const float PointLight::getExponent() const {
    return m_E; 
}
const LightAttenuation::Model& PointLight::getAttenuationModel() const {
    return m_AttenuationModel;
}
void PointLight::setConstant(const float c) { 
    m_C = c; 
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setLinear(const float l) { 
    m_L = l;
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setExponent(const float e) { 
    m_E = e;
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setAttenuation(const float c, const float l, const float e) { 
    m_C = c;
    m_L = l;
    m_E = e;
    m_CullingRadius = calculateCullingRadius(); 
}
void PointLight::setAttenuation(const LightRange::Range r) { 
    auto& d = LIGHT_RANGES[static_cast<uint>(r)];
    PointLight::setAttenuation(d.get<0>(), d.get<1>(), d.get<2>()); 
}
void PointLight::setAttenuationModel(const LightAttenuation::Model model) {
    m_AttenuationModel = model; 
    m_CullingRadius = calculateCullingRadius();
}