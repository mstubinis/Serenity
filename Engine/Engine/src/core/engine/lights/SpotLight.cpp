#include "core/engine/lights/SpotLight.h"

#include "core/engine/resources/Engine_BuiltInResources.h"
#include "core/engine/mesh/Mesh.h"

using namespace Engine;
using namespace std;

SpotLight::SpotLight(glm::vec3 pos, glm::vec3 direction, float cutoff, float outerCutoff, Scene* scene) : PointLight(LightType::Spot, pos, scene) {
    auto& body = *m_Entity.getComponent<ComponentBody>();
    body.alignTo(direction, 0);
    setCutoff(cutoff);
    setCutoffOuter(outerCutoff);

    if (m_Type == LightType::Spot)
        epriv::InternalScenePublicInterface::GetSpotLights(*scene).push_back(this);
}
SpotLight::~SpotLight() {
}
void SpotLight::setCutoff(float cutoff) {
    m_Cutoff = glm::cos(glm::radians(cutoff));
}
void SpotLight::setCutoffOuter(float outerCutoff) {
    m_OuterCutoff = glm::cos(glm::radians(outerCutoff));
}