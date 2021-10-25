
#include <serenity/renderer/Decal.h>
#include <serenity/system/Engine.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/material/Material.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/scene/Camera.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/system/Engine.h>
#include <serenity/renderer/pipelines/IRenderingPipeline.h>

#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentModel.h>

using namespace Engine;

namespace {
    constexpr auto Default_Decal_Bind_Functor = [](ModelInstance* i, const Engine::priv::RenderModule* renderer) {
        renderer->m_Pipeline->renderDecal(*i);
    };
    constexpr auto Default_Decal_Unbind_Functor = [](ModelInstance* i, const Engine::priv::RenderModule* renderer) {
    };
};

Decal::Decal(Handle materialHandle, const glm_vec3& localPosition, const glm::vec3& hitNormal, float size, Scene& scene, float lifetimeMax, RenderStage stage)
    : Entity{ scene }
    , m_InitialPosition{ localPosition }
    , m_LifetimeMax{ lifetimeMax}
{
    addComponent<ComponentTransform>();
    addComponent<ComponentModel>(priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), materialHandle, ShaderProgram::Decal, stage);

    auto& transform  = *getComponent<ComponentTransform>();
    auto& model      = *getComponent<ComponentModel>();

    const auto factor = (0.2f * size);

    //transform.setScale(factor, factor, (decimal)0.04);
    model.getModel(0).setScale(factor, factor, 0.04f);

    transform.setPosition(localPosition);
    auto q = Math::alignTo(hitNormal);
    m_InitialRotation = q;
    transform.setRotation(q);
      
    model.setCustomBindFunctor(Default_Decal_Bind_Functor);
    model.setCustomUnbindFunctor(Default_Decal_Unbind_Functor);
}
Decal::~Decal() {
    Entity::destroy();
}
Decal::Decal(Decal&& other) noexcept 
    : Entity{ std::move(other) }
{
    m_InitialRotation = std::move(other.m_InitialRotation);
    m_InitialPosition = std::move(other.m_InitialPosition);
    m_LifetimeCurrent = std::move(other.m_LifetimeCurrent);
    m_LifetimeMax     = std::move(other.m_LifetimeMax);
    m_Active          = std::move(other.m_Active);
}
Decal& Decal::operator=(Decal&& other) noexcept {
    Entity::operator=(std::move(other));
    m_InitialRotation = std::move(other.m_InitialRotation);
    m_InitialPosition = std::move(other.m_InitialPosition);
    m_LifetimeCurrent = std::move(other.m_LifetimeCurrent);
    m_LifetimeMax     = std::move(other.m_LifetimeMax);
    m_Active          = std::move(other.m_Active);
    return *this;
}
void Decal::update(const float dt) {
    if (m_Active) {
        m_LifetimeCurrent += dt;
        if (m_LifetimeCurrent < m_LifetimeMax && m_LifetimeCurrent >= m_LifetimeMax - 1.0f) {
            auto& model       = *getComponent<ComponentModel>();
            auto& instance    = model.getModel();
            const auto& color = instance.getColor();
            const float alpha = m_LifetimeMax - m_LifetimeCurrent;
            instance.setColor(color.r(), color.g(), color.b(), alpha);
        }
        if (m_LifetimeCurrent >= m_LifetimeMax) {
            m_Active = false;
        }
    }
}
