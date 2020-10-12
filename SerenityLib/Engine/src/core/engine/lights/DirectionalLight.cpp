#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/DirectionalLight.h>
#include <ecs/ComponentBody.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/scene/Scene.h>

DirectionalLight::DirectionalLight(Scene* scene, const glm::vec3& direction)
    : SunLight{ scene, glm::vec3(0.0f), LightType::Directional }
{
    getComponent<ComponentBody>()->alignTo(direction);
}
DirectionalLight::~DirectionalLight() {
}
void DirectionalLight::destroy() noexcept {
    Entity::destroy();
    Scene* scene_ptr = scene();
    if (scene_ptr) {
        removeFromVector(Engine::priv::InternalScenePublicInterface::GetDirectionalLights(*scene_ptr), this);
        removeFromVector(Engine::priv::InternalScenePublicInterface::GetLights(*scene_ptr), this);
    }
}