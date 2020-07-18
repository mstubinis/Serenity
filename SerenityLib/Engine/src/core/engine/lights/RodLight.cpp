#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/RodLight.h>
#include <core/engine/utils/Utils.h>
#include <ecs/ComponentBody.h>

using namespace Engine;
using namespace std;

RodLight::RodLight(const glm_vec3& pos, float rodLength, Scene* scene) : PointLight(LightType::Rod, pos, scene) {
    setRodLength(rodLength);

    auto* body = getComponent<ComponentBody>();
    if (body) { //evil, but needed. find out why...
        body->setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
    }

    if (m_Type == LightType::Rod) {
        auto& rodLights = priv::InternalScenePublicInterface::GetRodLights(*scene);
        rodLights.push_back(this);
    }
}
float RodLight::calculateCullingRadius() {
    float res = PointLight::calculateCullingRadius();
    auto& body = *getComponent<ComponentBody>();
    body.setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
    return res;
}
void RodLight::setRodLength(float length) {
    m_RodLength = length;
    auto& body = *getComponent<ComponentBody>();
    body.setScale(m_CullingRadius, m_CullingRadius, (m_RodLength / 2.0f) + m_CullingRadius);
}
void RodLight::free() noexcept {
    Entity::destroy();
    removeFromVector(priv::InternalScenePublicInterface::GetRodLights(scene()), this);
    removeFromVector(priv::InternalScenePublicInterface::GetLights(scene()), this);
}