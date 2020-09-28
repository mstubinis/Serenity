#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/Decal.h>
#include <core/engine/system/Engine.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/materials/Material.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/system/Engine.h>
#include <core/engine/renderer/pipelines/IRenderingPipeline.h>

#include <ecs/ComponentBody.h>
#include <ecs/ComponentModel.h>

using namespace Engine;

namespace Engine::priv {
    struct DefaultDecalBindFunctor { void operator()(ModelInstance* i, const Engine::priv::Renderer* renderer) const {
        renderer->m_Pipeline->renderDecal(*i);
    }};
    struct DefaultDecalUnbindFunctor { void operator()(ModelInstance* i, const Engine::priv::Renderer* renderer) const {
    }};
};

Decal::Decal(Material& material, const glm_vec3& localPosition, const glm::vec3& hitNormal, float size, Scene& scene, float lifetimeMax, RenderStage stage) 
    : EntityBody{ scene }
    , m_InitialPosition{ localPosition }
    , m_LifetimeMax{ lifetimeMax}
{
    addComponent<ComponentBody>();
    addComponent<ComponentModel>(&priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), &material, ShaderProgram::Decal, stage);

    auto& body  = *getComponent<ComponentBody>();
    auto& model = *getComponent<ComponentModel>();

    body.setPosition(localPosition);
    glm_quat q = glm_quat(1.0, 0.0, 0.0, 0.0);
    Math::alignTo(q, hitNormal);
    m_InitialRotation = q;
    body.setRotation(q);
    const decimal factor = (decimal)(0.2f * size);
    body.setScale(factor, factor, (decimal)0.04);

    model.setCustomBindFunctor(Engine::priv::DefaultDecalBindFunctor());
    model.setCustomUnbindFunctor(Engine::priv::DefaultDecalUnbindFunctor());
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
