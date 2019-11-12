#include <core/engine/lights/SpotLight.h>

#include <core/engine/resources/Engine_BuiltInResources.h>
#include <core/engine/mesh/Mesh.h>

#include <ecs/ComponentBody.h>

using namespace Engine;
using namespace std;

SpotLight::SpotLight(const glm::vec3 pos, const glm::vec3 direction, const float cutoff, const float outerCutoff, Scene* scene) : PointLight(LightType::Spot, pos, scene) {
    setCutoff(cutoff);
    setCutoffOuter(outerCutoff);

    auto body = getComponent<ComponentBody>();
    if (body) {//evil, but needed for now... find out why...
        body->alignTo(direction, 0);
    }

    if (m_Type == LightType::Spot) {
        epriv::InternalScenePublicInterface::GetSpotLights(*scene).push_back(this);
    }
}
SpotLight::~SpotLight() {
}
void SpotLight::setCutoff(const float cutoff) {
    m_Cutoff = glm::cos(glm::radians(cutoff));
}
void SpotLight::setCutoffOuter(const float outerCutoff) {
    m_OuterCutoff = glm::cos(glm::radians(outerCutoff));
}

const float SpotLight::getCutoff() const {
    return m_Cutoff;
}
const float SpotLight::getCutoffOuter() const {
    return m_OuterCutoff;
}
void SpotLight::destroy() {
    EntityWrapper::destroy();
    removeFromVector(epriv::InternalScenePublicInterface::GetSpotLights(m_Entity.scene()), this);
    removeFromVector(epriv::InternalScenePublicInterface::GetLights(m_Entity.scene()), this);
}