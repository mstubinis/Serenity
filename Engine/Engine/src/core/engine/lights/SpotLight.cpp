#include "core/engine/lights/SpotLight.h"

#include "core/engine/resources/Engine_BuiltInResources.h"
#include "core/engine/mesh/Mesh.h"

using namespace Engine;
using namespace std;

SpotLight::SpotLight(glm::vec3 pos, glm::vec3 direction, float cutoff, float outerCutoff, Scene* scene) : PointLight(pos, scene) {
    auto& body = *m_Entity.getComponent<ComponentBody>();
    body.alignTo(direction, 0);
    setCutoff(cutoff);
    setCutoffOuter(outerCutoff);
    m_Type = LightType::Spot;
}
SpotLight::~SpotLight() {
}
void SpotLight::setCutoff(float cutoff) {
    m_Cutoff = glm::cos(glm::radians(cutoff));
}
void SpotLight::setCutoffOuter(float outerCutoff) {
    m_OuterCutoff = glm::cos(glm::radians(outerCutoff));
}

void SpotLight::lighten() {
    if (!isActive()) return;
    Camera& c = *Resources::getCurrentScene()->getActiveCamera();
    auto& body = *m_Entity.getComponent<ComponentBody>();
    glm::vec3 pos = body.position();
    glm::vec3 _forward = body.forward();
    if (!c.sphereIntersectTest(pos, m_CullingRadius) || (c.getDistance(pos) > 1100.0f * m_CullingRadius))
        return;
    Renderer::sendUniform4("LightDataA", m_AmbientIntensity, m_DiffuseIntensity, m_SpecularIntensity, _forward.x);
    Renderer::sendUniform4("LightDataB", _forward.y, _forward.z, m_C, m_L);
    Renderer::sendUniform4("LightDataC", m_E, pos.x, pos.y, pos.z);
    Renderer::sendUniform4("LightDataD", m_Color.x, m_Color.y, m_Color.z, float(m_Type));
    Renderer::sendUniform4Safe("LightDataE", m_Cutoff, m_OuterCutoff, float(m_AttenuationModel), 0.0f);
    Renderer::sendUniform2Safe("VertexShaderData", m_OuterCutoff, m_CullingRadius);
    Renderer::sendUniform1Safe("Type", 2.0f);

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
    auto& spotLightMesh = *epriv::InternalMeshes::SpotLightBounds;

    spotLightMesh.bind();
    spotLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    spotLightMesh.unbind();
    Renderer::Settings::cullFace(GL_BACK);
    Renderer::setDepthFunc(DepthFunc::LEqual);

    Renderer::sendUniform1Safe("Type", 0.0f); //is this really needed?
    Renderer::GLDisable(GLState::DEPTH_TEST);
}