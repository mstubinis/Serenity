#include <core/engine/renderer/Decal.h>
#include <core/engine/Engine.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/materials/Material.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/renderer/GBuffer.h>

#include <ecs/ComponentBody.h>
#include <ecs/ComponentModel.h>

using namespace std;
using namespace Engine;

namespace Engine {
    namespace epriv {
        struct DefaultDecalBindFunctor { void operator()(EngineResource* r) const {
            auto& i = *static_cast<ModelInstance*>(r);
            const auto& stage = i.stage();
            Entity& parent = i.parent();
            auto& scene = parent.scene();
            Camera& cam = *scene.getActiveCamera();
            glm::vec3 camPos = cam.getPosition();
            auto& body = *(parent.getComponent<ComponentBody>());
            glm::mat4 parentModel = body.modelMatrix();

            auto& renderMgr = epriv::Core::m_Engine->m_RenderManager;
            auto maxTextures = renderMgr.OpenGLStateMachine.getMaxTextureUnits() - 1;
            auto& gbuffer = renderMgr.getGbuffer();
            Renderer::sendTextureSafe("gDepthMap", gbuffer.getTexture(GBufferType::Depth), maxTextures);

            Renderer::sendUniform4Safe("Object_Color", i.color());
            Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());

            glm::mat4 modelMatrix = parentModel * i.modelMatrix();

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

            const auto& winSize = Resources::getWindowSize();
            Renderer::sendUniform4Safe("Resolution", static_cast<float>(winSize.x), static_cast<float>(winSize.y), 0.0f, 0.0f);
            Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
            Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
        }};
        struct DefaultDecalUnbindFunctor { void operator()(EngineResource* r) const {
            //auto& i = *static_cast<ModelInstance*>(r);
        }};
    };
};

Decal::Decal(Material& material, const glm_vec3& position, const glm::vec3& hitNormal, const float& size, Scene& scene, const float& lifetimeMax, const RenderStage::Stage stage):EntityWrapper(scene) {
    auto& body = *addComponent<ComponentBody>();
    auto& model = *addComponent<ComponentModel>(Mesh::Cube, &material, ShaderProgram::Decal, stage);

    body.setPosition(position);
    glm_quat q = glm_quat(1.0, 0.0, 0.0, 0.0);
    Math::alignTo(q, hitNormal);
    body.setRotation(q);
    const decimal factor = static_cast<decimal>(0.2f * size);
    body.setScale(factor, factor, static_cast<decimal>(0.04));

    model.setCustomBindFunctor(Engine::epriv::DefaultDecalBindFunctor());
    model.setCustomUnbindFunctor(Engine::epriv::DefaultDecalUnbindFunctor());

    m_LifetimeCurrent = 0.0f;
    m_LifetimeMax = lifetimeMax;
    m_Active = true;

    m_InitialPosition = position;
    m_InitialRotation = q;
}
Decal::~Decal() {

}
void Decal::update(const double& dt) {
    const float fdt = static_cast<float>(dt);
    if (m_Active) {
        m_LifetimeCurrent += fdt;
        if (m_LifetimeCurrent < m_LifetimeMax && m_LifetimeCurrent >= m_LifetimeMax - 1.0f) {
            auto& model = *getComponent<ComponentModel>();
            auto& instance = model.getModel();
            const auto& color = instance.color();
            const float alpha = m_LifetimeMax - m_LifetimeCurrent;
            instance.setColor(color.x, color.y, color.z, alpha);
        }
        if (m_LifetimeCurrent >= m_LifetimeMax) {
            m_Active = false;
            destroy();
        }
    }
}
const bool& Decal::active() const {
    return m_Active;
}
const glm_vec3& Decal::initialPosition() const {
    return m_InitialPosition;
}
const glm_quat& Decal::initialRotation() const {
    return m_InitialRotation;
}
const glm_vec3 Decal::position() {
    return getComponent<ComponentBody>()->position();
}
const glm_quat Decal::rotation() {
    return getComponent<ComponentBody>()->rotation();
}
const glm::vec3 Decal::getScale() {
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