#include <serenity/lights/SpotLight.h>
#include <serenity/utils/Utils.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/scene/Scene.h>

SpotLight::SpotLight(Scene* scene, const glm_vec3& position, const glm_vec3& direction, float innerCutoffInDegrees, float outerCutoffInDegrees)
    : PointLight{ scene, LightType::Spot, position }
{
    setCutoffDegrees(innerCutoffInDegrees);
    setCutoffOuterDegrees(outerCutoffInDegrees);
    setDirection(direction);
}
void SpotLight::setDirection(float xDir, float yDir, float zDir) noexcept {
    auto transform = getComponent<ComponentTransform>();
    transform->alignTo(xDir, yDir, zDir);
}
void SpotLight::setDirection(const glm::vec3& direction) noexcept {
    setDirection(direction.x, direction.y, direction.z);
}