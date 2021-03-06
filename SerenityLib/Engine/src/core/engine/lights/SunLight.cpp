#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/SunLight.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/utils/Utils.h>
#include <ecs/ComponentBody.h>
#include <core/engine/scene/Scene.h>

SunLight::SunLight(Scene* scene, const glm_vec3& pos, LightType type)
    : EntityBody{ (!scene) ? *Engine::Resources::getCurrentScene() : *scene }
    , m_Type{ type }
{
    addComponent<ComponentBody>();
    SunLight::setPosition(pos);
}
SunLight::~SunLight() {
}
glm_vec3 SunLight::position() const { return getComponent<ComponentBody>()->getPosition(); }
void SunLight::setPosition(decimal x, decimal y, decimal z) { getComponent<ComponentBody>()->setPosition(x, y, z); }
void SunLight::setPosition(decimal position) { getComponent<ComponentBody>()->setPosition(position, position, position); }
void SunLight::setPosition(const glm_vec3& position) { getComponent<ComponentBody>()->setPosition(position); }