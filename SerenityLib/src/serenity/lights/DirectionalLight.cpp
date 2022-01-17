#include <serenity/lights/DirectionalLight.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/utils/Utils.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/Engine.h>

DirectionalLight::DirectionalLight(Scene* scene, float directionX, float directionY, float directionZ)
    : Entity{ (!scene) ? *Engine::Resources::getCurrentScene() : *scene }
    , LightBaseData<DirectionalLight>{ LightType::Directional }
{
    addComponent<ComponentTransform>();
    setDirection(directionX, directionY, directionZ);
}
DirectionalLight::DirectionalLight(Scene* scene, const glm::vec3& direction)
    : DirectionalLight{ scene, direction.x, direction.y, direction.z }
{}

bool DirectionalLight::setShadowCaster(bool castsShadow, uint32_t shadowMapWidth, uint32_t shadowMapHeight, LightShadowFrustumType frustumType, float nearFactor, float farFactor) noexcept {
    auto task = [=]() -> bool {
        auto& renderer = Engine::priv::Core::m_Engine->m_RenderModule;
        renderer.setShadowCaster(*this, castsShadow, shadowMapWidth, shadowMapHeight, frustumType, nearFactor, farFactor);
        if ((m_IsShadowCaster && castsShadow == false) || (!m_IsShadowCaster && castsShadow == true)) {
            m_IsShadowCaster = castsShadow;
            return Engine::priv::PublicScene::SetLightShadowCaster<DirectionalLight>(*scene(), *this, castsShadow);
        }
        return false;
    };
    if (Engine::priv::threading::isOpenGLThread()) {
        return task();
    } else {
        Engine::priv::threading::submitTaskForMainThread(task);
    }
    return false;
}

void DirectionalLight::setDirection(float directionX, float directionY, float directionZ) {
    getComponent<ComponentTransform>()->alignTo(directionX, directionY, directionZ);
}
void DirectionalLight::setDirection(const glm::vec3& direction) {
    DirectionalLight::setDirection(direction.x, direction.y, direction.z);
}
glm::vec3 DirectionalLight::getDirection() const noexcept {
    return getComponent<ComponentTransform>()->getForward();
}