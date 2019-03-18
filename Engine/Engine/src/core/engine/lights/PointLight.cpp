#include "core/engine/lights/PointLight.h"
#include "core/engine/Engine_Math.h"
#include "core/engine/Engine_Resources.h"

#include "core/engine/Engine_BuiltInResources.h"
#include "core/engine/mesh/Mesh.h"

using namespace Engine;
using namespace std;

std::vector<boost::tuple<float, float, float>> LIGHT_RANGES = []() {
    std::vector<boost::tuple<float, float, float>> m;
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

PointLight::PointLight(glm::vec3 pos, Scene* scene) : SunLight(pos, LightType::Point, scene) {
    m_C                = 0.1f;
    m_L                = 0.1f;
    m_E                = 0.1f;
    m_CullingRadius    = calculateCullingRadius();
    m_AttenuationModel = LightAttenuation::Constant_Linear_Exponent;
}
PointLight::~PointLight() {
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
    auto& body = *m_Entity.getComponent<ComponentBody>();
    body.setScale(radius, radius, radius);
    return radius;
}
float PointLight::getCullingRadius() { return m_CullingRadius; }
float PointLight::getConstant() { return m_C; }
float PointLight::getLinear() { return m_L; }
float PointLight::getExponent() { return m_E; }
void PointLight::setConstant(float c) { m_C = c; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setLinear(float l) { m_L = l; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setExponent(float e) { m_E = e; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setAttenuation(float c, float l, float e) { m_C = c; m_L = l; m_E = e; m_CullingRadius = calculateCullingRadius(); }
void PointLight::setAttenuation(LightRange::Range r) { auto& d = LIGHT_RANGES[uint(r)]; PointLight::setAttenuation(d.get<0>(), d.get<1>(), d.get<2>()); }
void PointLight::setAttenuationModel(LightAttenuation::Model model) {
    m_AttenuationModel = model; m_CullingRadius = calculateCullingRadius();
}
void PointLight::lighten() {
    if (!isActive()) return;
    Camera& c = *Resources::getCurrentScene()->getActiveCamera();
    auto& body = *m_Entity.getComponent<ComponentBody>();
    glm::vec3 pos = body.position();
    if ((!c.sphereIntersectTest(pos, m_CullingRadius)) || (c.getDistance(pos) > 1100.0f * m_CullingRadius)) //1100.0f is the visibility threshold
        return;
    Renderer::sendUniform4("LightDataA", m_AmbientIntensity, m_DiffuseIntensity, m_SpecularIntensity, 0.0f);
    Renderer::sendUniform4("LightDataB", 0.0f, 0.0f, m_C, m_L);
    Renderer::sendUniform4("LightDataC", m_E, pos.x, pos.y, pos.z);
    Renderer::sendUniform4("LightDataD", m_Color.x, m_Color.y, m_Color.z, float(m_Type));
    Renderer::sendUniform4Safe("LightDataE", 0.0f, 0.0f, float(m_AttenuationModel), 0.0f);
    Renderer::sendUniform1Safe("Type", 1.0f);

    glm::vec3 camPos = c.getPosition();
    glm::mat4 model = body.modelMatrix();
    glm::mat4 vp = c.getViewProjection();

    Renderer::sendUniformMatrix4("Model", model);
    Renderer::sendUniformMatrix4("VP", vp);

    Renderer::GLEnable(GLState::DEPTH_TEST);
    if (glm::distance(c.getPosition(), pos) <= m_CullingRadius) { //inside the light volume
        Renderer::Settings::cullFace(GL_FRONT);
        Renderer::setDepthFunc(DepthFunc::GEqual);
    }
    auto& pointLightMesh = *epriv::InternalMeshes::PointLightBounds;

    pointLightMesh.bind();
    pointLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    pointLightMesh.unbind();
    Renderer::Settings::cullFace(GL_BACK);
    Renderer::setDepthFunc(DepthFunc::LEqual);
    Renderer::GLDisable(GLState::DEPTH_TEST);
}