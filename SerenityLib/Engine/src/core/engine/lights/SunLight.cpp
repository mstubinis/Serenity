#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/SunLight.h>
#include <core/engine/resources/Engine_Resources.h>

#include <ecs/ComponentBody.h>

using namespace Engine;

SunLight::SunLight(const glm_vec3& pos, LightType type, Scene* scene) 
    : Entity{ *scene }
    , m_Type{ type }
{
    if (!scene) {
        scene = Resources::getCurrentScene();
    }
    if (type == LightType::Sun) {
        auto& sunLights = priv::InternalScenePublicInterface::GetSunLights(*scene);
        sunLights.emplace_back(this);
    }
    auto& allLights = priv::InternalScenePublicInterface::GetLights(*scene);
    allLights.emplace_back(this);

    addComponent<ComponentBody>();
    SunLight::setPosition(pos);
}
glm_vec3 SunLight::position() const {
    return getComponent<ComponentBody>()->getPosition();
}
void SunLight::setPosition(decimal x, decimal y, decimal z) {
    getComponent<ComponentBody>()->setPosition(x, y, z); 
}
void SunLight::setPosition(decimal position) {
    getComponent<ComponentBody>()->setPosition(position, position, position);
}
void SunLight::setPosition(const glm_vec3& position) {
    getComponent<ComponentBody>()->setPosition(position);
}
void SunLight::free() noexcept {
    Entity::destroy();
    removeFromVector(priv::InternalScenePublicInterface::GetSunLights(scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(scene()), this);
}