#include <core/engine/lights/SpotLight.h>

#include <ecs/ComponentBody.h>

using namespace Engine;
using namespace std;

SpotLight::SpotLight(const glm_vec3& pos, const glm::vec3& direction, const float cutoff, const float outerCutoff, Scene* scene) : PointLight(LightType::Spot, pos, scene) {
    setCutoff(cutoff);
    setCutoffOuter(outerCutoff);

    auto body = getComponent<ComponentBody>();
    if (body) {//evil, but needed for now... find out why...
        body->alignTo(direction);
    }

    if (m_Type == LightType::Spot) {
        auto& spotLights = priv::InternalScenePublicInterface::GetSpotLights(*scene);
        spotLights.push_back(this);
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
    removeFromVector(priv::InternalScenePublicInterface::GetSpotLights(m_Entity.scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(m_Entity.scene()), this);
}