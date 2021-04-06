#include <serenity/lights/SpotLight.h>
#include <serenity/utils/Utils.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/scene/Scene.h>

SpotLight::SpotLight(Scene* scene, const glm_vec3& pos, const glm_vec3& direction, float innerCutoffInDegrees, float outerCutoffInDegrees)
    : PointLight{ scene, LightType::Spot, pos }
{
    setCutoff(innerCutoffInDegrees);
    setCutoffOuter(outerCutoffInDegrees);

    setDirection(direction);
}
SpotLight::~SpotLight() {
}
void SpotLight::setDirection(decimal xDir, decimal yDir, decimal zDir) noexcept {
    auto body = getComponent<ComponentTransform>();
    if (body) {
        body->alignTo(xDir, yDir, zDir);
    }
}
void SpotLight::setDirection(const glm_vec3& direction) noexcept {
    setDirection(direction.x, direction.y, direction.z);
}