#include <core/engine/lights/DirectionalLight.h>

using namespace Engine;
using namespace std;

DirectionalLight::DirectionalLight(glm::vec3 dir, Scene* scene) :SunLight(glm::vec3(0), LightType::Directional, scene) {
    m_Entity.getComponent<ComponentBody>()->alignTo(dir, 0);

    if (m_Type == LightType::Directional)
        epriv::InternalScenePublicInterface::GetDirectionalLights(*scene).push_back(this);
}
DirectionalLight::~DirectionalLight() {
}
