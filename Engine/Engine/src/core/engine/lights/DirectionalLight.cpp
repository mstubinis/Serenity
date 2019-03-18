#include "core/engine/lights/DirectionalLight.h"

using namespace Engine;
using namespace std;

DirectionalLight::DirectionalLight(glm::vec3 dir, Scene* scene) :SunLight(glm::vec3(0), LightType::Directional, scene) {
    m_Entity.getComponent<ComponentBody>()->alignTo(dir, 0);
}
DirectionalLight::~DirectionalLight() {
}
void DirectionalLight::lighten() {
    if (!isActive()) return;
    auto& body = *m_Entity.getComponent<ComponentBody>();
    glm::vec3 _forward = body.forward();
    Renderer::sendUniform4("LightDataA", m_AmbientIntensity, m_DiffuseIntensity, m_SpecularIntensity, _forward.x);
    Renderer::sendUniform4("LightDataB", _forward.y, _forward.z, 0.0f, 0.0f);
    Renderer::sendUniform4("LightDataD", m_Color.x, m_Color.y, m_Color.z, float(m_Type));
    Renderer::sendUniform1Safe("Type", 0.0f);
    Renderer::renderFullscreenTriangle(Resources::getWindowSize().x, Resources::getWindowSize().y);
}