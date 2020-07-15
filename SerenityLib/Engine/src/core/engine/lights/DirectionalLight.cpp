#include <core/engine/lights/DirectionalLight.h>
#include <ecs/ComponentBody.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace std;

DirectionalLight::DirectionalLight(const glm::vec3& direction, Scene* scene) : SunLight(glm::vec3(0.0f), LightType::Directional, scene) {
    getComponent<ComponentBody>()->alignTo(direction);

    if (m_Type == LightType::Directional) {
        auto& dirLights = priv::InternalScenePublicInterface::GetDirectionalLights(*scene);
        dirLights.push_back(this);
    }
}
void DirectionalLight::free() noexcept {
    Entity::destroy();
    removeFromVector(priv::InternalScenePublicInterface::GetDirectionalLights(scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(scene()), this);
}