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

Decal::Decal(Material& material, const glm_vec3& localPosition, const glm::vec3& hitNormal, const float size, Scene& scene, const float lifetimeMax, const RenderStage::Stage stage) : EntityWrapper(scene) {
    auto& cubeMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh();
    
    addComponent<ComponentBody>();
    addComponent<ComponentModel>(&cubeMesh, &material, ShaderProgram::Decal, stage);

    auto& body  = *getComponent<ComponentBody>();
    auto& model = *getComponent<ComponentModel>();

    m_InitialPosition = localPosition;

    body.setPosition(localPosition);
    glm_quat q = glm_quat(1.0, 0.0, 0.0, 0.0);
    Math::alignTo(q, hitNormal);
    m_InitialRotation = q;
    body.setRotation(q);
    const decimal factor = static_cast<decimal>(0.2f * size);
    body.setScale(factor, factor, static_cast<decimal>(0.04));

    model.setCustomBindFunctor(Engine::priv::DefaultDecalBindFunctor());
    model.setCustomUnbindFunctor(Engine::priv::DefaultDecalUnbindFunctor());

    m_LifetimeCurrent = 0.0f;
    m_LifetimeMax = lifetimeMax;
    m_Active = true;
}
Decal::~Decal() {
    destroy();
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
            destroy();
        }
    }
}
const bool Decal::active() const {
    return m_Active;
}

const glm_vec3& Decal::initialPosition() const {
    return m_InitialPosition;
}
const glm_quat& Decal::initialRotation() const {
    return m_InitialRotation;
}

const glm_vec3 Decal::position() const {
    return getComponent<ComponentBody>()->position();
}
const glm_vec3 Decal::localPosition() const {
    return getComponent<ComponentBody>()->localPosition();
}
const glm_quat Decal::rotation() const {
    return getComponent<ComponentBody>()->rotation();
}

const glm::vec3 Decal::getScale() const {
    return getComponent<ComponentBody>()->getScale();
}
void Decal::setPosition(const glm_vec3& position) {
    getComponent<ComponentBody>()->setPosition(position);
}
void Decal::setPosition(const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>()->setPosition(x, y, z);
}
void Decal::setRotation(const glm_quat& rotation) {
    getComponent<ComponentBody>()->setRotation(rotation);
}
void Decal::setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w) {
    getComponent<ComponentBody>()->setRotation(x, y, z, w);
}


const glm_vec3 Decal::position(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->position();
}
const glm_vec3 Decal::localPosition(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->localPosition();
}
const glm_quat Decal::rotation(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->rotation();
}

const glm::vec3 Decal::getScale(const EntityDataRequest& request) const {
    return getComponent<ComponentBody>(request)->getScale();
}
void Decal::setPosition(const EntityDataRequest& request, const glm_vec3& position) {
    getComponent<ComponentBody>(request)->setPosition(position);
}
void Decal::setPosition(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z) {
    getComponent<ComponentBody>(request)->setPosition(x, y, z);
}
void Decal::setRotation(const EntityDataRequest& request, const glm_quat& rotation) {
    getComponent<ComponentBody>(request)->setRotation(rotation);
}
void Decal::setRotation(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const decimal& w) {
    getComponent<ComponentBody>(request)->setRotation(x, y, z, w);
}