#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/SpotLight.h>
#include <core/engine/utils/Utils.h>
#include <ecs/ComponentBody.h>
#include <core/engine/scene/Scene.h>

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
    auto body = getComponent<ComponentBody>();
    if (body) {
        body->alignTo(xDir, yDir, zDir);
    }
}
void SpotLight::setDirection(const glm_vec3& direction) noexcept {
    setDirection(direction.x, direction.y, direction.z);
}