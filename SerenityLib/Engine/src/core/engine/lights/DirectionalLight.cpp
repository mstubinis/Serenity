#include <core/engine/lights/DirectionalLight.h>
#include <ecs/ComponentBody.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace std;

DirectionalLight::DirectionalLight(const glm::vec3& dir, Scene* scene) : SunLight(glm::vec3(0), LightType::Directional, scene) {
    getComponent<ComponentBody>()->alignTo(dir);

    if (m_Type == LightType::Directional) {
        auto& dirLights = priv::InternalScenePublicInterface::GetDirectionalLights(*scene);
        dirLights.push_back(this);
    }
}
DirectionalLight::~DirectionalLight() {
}
void DirectionalLight::free() {
    Entity::destroy();
    removeFromVector(priv::InternalScenePublicInterface::GetDirectionalLights(scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(scene()), this);
}