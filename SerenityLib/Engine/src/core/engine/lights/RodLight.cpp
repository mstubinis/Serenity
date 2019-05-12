#include <core/engine/lights/RodLight.h>

#include <core/engine/resources/Engine_BuiltInResources.h>
#include <core/engine/mesh/Mesh.h>

using namespace Engine;
using namespace std;

RodLight::RodLight(glm::vec3 pos, float rodLength, Scene* scene) : PointLight(LightType::Rod, pos, scene) {
    setRodLength(rodLength);
    auto& body = *m_Entity.getComponent<ComponentBody>();
    body.setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);

    if (m_Type == LightType::Rod)
        epriv::InternalScenePublicInterface::GetRodLights(*scene).push_back(this);
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
float RodLight::rodLength() { return m_RodLength; }
