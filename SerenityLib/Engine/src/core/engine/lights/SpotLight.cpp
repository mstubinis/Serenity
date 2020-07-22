#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/SpotLight.h>
#include <core/engine/utils/Utils.h>
#include <ecs/ComponentBody.h>

SpotLight::SpotLight(const glm_vec3& pos, const glm_vec3& direction, float innerCutoffInDegrees, float outerCutoffInDegrees, Scene* scene) : PointLight(LightType::Spot, pos, scene) {
    setCutoff(innerCutoffInDegrees);
    setCutoffOuter(outerCutoffInDegrees);

    setDirection(direction);

    if (m_Type == LightType::Spot) {
        auto& spotLights = Engine::priv::InternalScenePublicInterface::GetSpotLights(*scene);
        spotLights.push_back(this);
    }
}
SpotLight::~SpotLight() {
    free();
}
void SpotLight::setDirection(decimal xDir, decimal yDir, decimal zDir) noexcept {
    auto body = getComponent<ComponentBody>();
    if (body) {
        body->alignTo(xDir, yDir, zDir);
    }
}
void SpotLight::setDirection(const glm_vec3& direction) noexcept {
    setDirection(direction.x, direction.y, direction.z);
}
void SpotLight::free() noexcept {
    Entity::destroy();
    removeFromVector(Engine::priv::InternalScenePublicInterface::GetSpotLights(scene()), this);
    removeFromVector(Engine::priv::InternalScenePublicInterface::GetLights(scene()), this);
}
