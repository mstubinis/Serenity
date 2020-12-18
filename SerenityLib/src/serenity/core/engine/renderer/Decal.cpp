
#include <serenity/core/engine/renderer/Decal.h>
#include <serenity/core/engine/system/Engine.h>
#include <serenity/core/engine/math/Engine_Math.h>
#include <serenity/core/engine/materials/Material.h>
#include <serenity/core/engine/mesh/Mesh.h>
#include <serenity/core/engine/shaders/ShaderProgram.h>
#include <serenity/core/engine/scene/Camera.h>
#include <serenity/core/engine/renderer/GBuffer.h>
#include <serenity/core/engine/system/Engine.h>
#include <serenity/core/engine/renderer/pipelines/IRenderingPipeline.h>

#include <serenity/ecs/ComponentBody.h>
#include <serenity/ecs/ComponentModel.h>

using namespace Engine;

namespace Engine::priv {
    constexpr auto DefaultDecalBindFunctor = [](ModelInstance* i, const Engine::priv::RenderModule* renderer) {
        renderer->m_Pipeline->renderDecal(*i);
    };
    constexpr auto DefaultDecalUnbindFunctor = [](ModelInstance* i, const Engine::priv::RenderModule* renderer) {
    };
};

Decal::Decal(Handle materialHandle, const glm_vec3& localPosition, const glm::vec3& hitNormal, float size, Scene& scene, float lifetimeMax, RenderStage stage) 
    : EntityBody{ scene }
    , m_InitialPosition{ localPosition }
    , m_LifetimeMax{ lifetimeMax}
{
    addComponent<ComponentBody>();
    addComponent<ComponentModel>(priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), materialHandle, ShaderProgram::Decal, stage);

    auto& body  = *getComponent<ComponentBody>();
    auto& model = *getComponent<ComponentModel>();

    body.setPosition(localPosition);
    auto q = Math::alignTo(hitNormal);
    m_InitialRotation = q;
    body.setRotation(q);
    const decimal factor = (decimal)(0.2f * size);
    body.setScale(factor, factor, (decimal)0.04);

    model.setCustomBindFunctor(Engine::priv::DefaultDecalBindFunctor);
    model.setCustomUnbindFunctor(Engine::priv::DefaultDecalUnbindFunctor);
}
Decal::Decal(Decal&& other) noexcept 
    : EntityBody{ std::move(other) }
    , m_LifetimeCurrent{ std::move(other.m_LifetimeCurrent) }
    , m_LifetimeMax{ std::move(other.m_LifetimeMax) }
    , m_Active{ std::move(other.m_Active) }
    , m_InitialPosition{ std::move(other.m_InitialPosition) }
    , m_InitialRotation{ std::move(other.m_InitialRotation) }
{}
Decal& Decal::operator=(Decal&& other) noexcept {
    EntityBody::operator=(std::move(other));
    m_LifetimeCurrent   = std::move(other.m_LifetimeCurrent);
    m_LifetimeMax       = std::move(other.m_LifetimeMax);
    m_Active            = std::move(other.m_Active);
    m_InitialPosition   = std::move(other.m_InitialPosition);
    m_InitialRotation   = std::move(other.m_InitialRotation);
    return *this;
}
Decal::~Decal() {
    Entity::destroy();
}
void Decal::update(const float dt) {
    if (m_Active) {
        m_LifetimeCurrent += dt;
        if (m_LifetimeCurrent < m_LifetimeMax && m_LifetimeCurrent >= m_LifetimeMax - 1.0f) {
            auto& model       = *getComponent<ComponentModel>();
            auto& instance    = model.getModel();
            const auto& color = instance.color();
            const float alpha = m_LifetimeMax - m_LifetimeCurrent;
            instance.setColor(color.r(), color.g(), color.b(), alpha);
        }
        if (m_LifetimeCurrent >= m_LifetimeMax) {
            m_Active = false;
            Entity::destroy();
        }
    }
}
