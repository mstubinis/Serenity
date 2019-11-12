#include <core/engine/lights/DirectionalLight.h>
#include <ecs/ComponentBody.h>

using namespace Engine;
using namespace std;

DirectionalLight::DirectionalLight(const glm::vec3 dir, Scene* scene) :SunLight(glm::vec3(0), LightType::Directional, scene) {
    m_Entity.getComponent<ComponentBody>()->alignTo(dir, 0);

    if (m_Type == LightType::Directional) {
        epriv::InternalScenePublicInterface::GetDirectionalLights(*scene).push_back(this);
    }
}
DirectionalLight::~DirectionalLight() {
}
void DirectionalLight::destroy() {
    EntityWrapper::destroy();
    removeFromVector(epriv::InternalScenePublicInterface::GetDirectionalLights(m_Entity.scene()), this);
    removeFromVector(epriv::InternalScenePublicInterface::GetLights(m_Entity.scene()), this);
}