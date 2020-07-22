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

using namespace std;
using namespace Engine;

namespace Engine::priv {
    struct DefaultDecalBindFunctor { void operator()(ModelInstance* i, const Engine::priv::Renderer* renderer) const {
        renderer->m_Pipeline->renderDecal(*i);
    }};
    struct DefaultDecalUnbindFunctor { void operator()(ModelInstance* i, const Engine::priv::Renderer* renderer) const {
        //auto& i = *static_cast<ModelInstance*>(r);
    }};
};

Decal::Decal(Material& material, const glm_vec3& localPosition, const glm::vec3& hitNormal, float size, Scene& scene, float lifetimeMax, RenderStage stage) : EntityBody(scene) {

    addComponent<ComponentBody>();
    addComponent<ComponentModel>(&priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), &material, ShaderProgram::Decal, stage);

    auto& body  = *getComponent<ComponentBody>();
    auto& model = *getComponent<ComponentModel>();

    m_InitialPosition = localPosition;
    m_LifetimeMax     = lifetimeMax;

    body.setPosition(localPosition);
    glm_quat q = glm_quat(1.0, 0.0, 0.0, 0.0);
    Math::alignTo(q, hitNormal);
    m_InitialRotation = q;
    body.setRotation(q);
    const decimal factor = static_cast<decimal>(0.2f * size);
    body.setScale(factor, factor, static_cast<decimal>(0.04));

    model.setCustomBindFunctor(Engine::priv::DefaultDecalBindFunctor());
    model.setCustomUnbindFunctor(Engine::priv::DefaultDecalUnbindFunctor());
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
bool Decal::active() const {
    return m_Active;
}
const glm_vec3& Decal::initialPosition() const {
    return m_InitialPosition;
}
const glm_quat& Decal::initialRotation() const {
    return m_InitialRotation;
}