#include "core/engine/lights/RodLight.h"

#include "core/engine/resources/Engine_BuiltInResources.h"
#include "core/engine/mesh/Mesh.h"

using namespace Engine;
using namespace std;

RodLight::RodLight(glm::vec3 pos, float rodLength, Scene* scene) : PointLight(pos, scene) {
    setRodLength(rodLength);
    m_Type = LightType::Rod;
    auto& body = *m_Entity.getComponent<ComponentBody>();
    body.setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
}
RodLight::~RodLight() {
}
float RodLight::calculateCullingRadius() {
    float res = PointLight::calculateCullingRadius();
    auto& body = *m_Entity.getComponent<ComponentBody>();
    body.setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
    return res;
}
void RodLight::setRodLength(float length) {
    m_RodLength = length;
    auto& body = *m_Entity.getComponent<ComponentBody>();
    body.setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
}
void RodLight::lighten() {
    if (!isActive()) return;
    Camera& c = *Resources::getCurrentScene()->getActiveCamera();
    auto& body = *m_Entity.getComponent<ComponentBody>();
    glm::vec3 pos = body.position();
    float cullingDistance = m_RodLength + (m_CullingRadius * 2.0f);
    if (!c.sphereIntersectTest(pos, cullingDistance) || (c.getDistance(pos) > 1100.0f * cullingDistance))
        return;
    float half = m_RodLength / 2.0f;
    glm::vec3 firstEndPt = pos + (body.forward() * half);
    glm::vec3 secndEndPt = pos - (body.forward() * half);
    Renderer::sendUniform4("LightDataA", m_AmbientIntensity, m_DiffuseIntensity, m_SpecularIntensity, firstEndPt.x);
    Renderer::sendUniform4("LightDataB", firstEndPt.y, firstEndPt.z, m_C, m_L);
    Renderer::sendUniform4("LightDataC", m_E, secndEndPt.x, secndEndPt.y, secndEndPt.z);
    Renderer::sendUniform4("LightDataD", m_Color.x, m_Color.y, m_Color.z, float(m_Type));
    Renderer::sendUniform4Safe("LightDataE", m_RodLength, 0.0f, float(m_AttenuationModel), 0.0f);
    Renderer::sendUniform1Safe("Type", 1.0f);

    glm::vec3 camPos = c.getPosition();
    glm::mat4 model = body.modelMatrix();
    glm::mat4 vp = c.getViewProjection();

    Renderer::sendUniformMatrix4("Model", model);
    Renderer::sendUniformMatrix4("VP", vp);

    Renderer::GLEnable(GLState::DEPTH_TEST);
    if (glm::distance(c.getPosition(), pos) <= cullingDistance) {
        Renderer::Settings::cullFace(GL_FRONT);
        Renderer::setDepthFunc(DepthFunc::GEqual);
    }
    auto& rodLightMesh = *epriv::InternalMeshes::RodLightBounds;

    rodLightMesh.bind();
    rodLightMesh.render(false); //this can bug out if we pass in custom uv's like in the renderQuad method
    rodLightMesh.unbind();
    Renderer::Settings::cullFace(GL_BACK);
    Renderer::setDepthFunc(DepthFunc::LEqual);
    Renderer::GLDisable(GLState::DEPTH_TEST);

    Renderer::sendUniform1Safe("Type", 0.0f); //is this really needed?
}
float RodLight::rodLength() { return m_RodLength; }
