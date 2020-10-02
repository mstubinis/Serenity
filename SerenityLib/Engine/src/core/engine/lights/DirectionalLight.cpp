#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/DirectionalLight.h>
#include <ecs/ComponentBody.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/scene/Scene.h>

using namespace Engine;

DirectionalLight::DirectionalLight(const glm::vec3& direction, Scene* scene) 
    : SunLight{ glm::vec3(0.0f), LightType::Directional, scene }
{
    getComponent<ComponentBody>()->alignTo(direction);

    if (m_Type == LightType::Directional) {
        auto& dirLights = priv::InternalScenePublicInterface::GetDirectionalLights(*scene);
        dirLights.emplace_back(this);
    }
}
void DirectionalLight::free() noexcept {
    Entity::destroy();
    removeFromVector(priv::InternalScenePublicInterface::GetDirectionalLights(scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(scene()), this);
}