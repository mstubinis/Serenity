#include <core/engine/renderer/particles/Particle.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/GBuffer.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>
#include <core/engine/math/Engine_Math.h>

using namespace std;
using namespace Engine;

ParticleData::ParticleData(){
    m_Properties = &ParticleEmissionProperties::DefaultProperties;
    m_Velocity = m_Properties->m_InitialVelocityFunctor();
    m_Scale = m_Properties->m_InitialScaleFunctor();
    m_AngularVelocity = m_Properties->m_InitialAngularVelocityFunctor();
    m_Active = true;
    m_Timer = 0.0;
    m_Depth = 0.0f;
    m_Angle = 0.0f;
    m_Color = glm::vec4(1.0f);
}
ParticleData::ParticleData(ParticleEmissionProperties& properties){
    m_Properties = &properties;
    m_Active = true;
    m_Timer = 0.0;
    m_Depth = 0.0f;
    m_Angle = 0.0f;
    m_Color = glm::vec4(1.0f);

    auto& randMat = const_cast<Material&>(properties.getParticleMaterialRandom());

    m_Velocity = properties.m_InitialVelocityFunctor();
    m_Scale = properties.m_InitialScaleFunctor();
    m_AngularVelocity = properties.m_InitialAngularVelocityFunctor();
}
ParticleData::ParticleData(const ParticleData& other){
    m_Active = other.m_Active;
    m_Timer = other.m_Timer;
    m_Velocity = other.m_Velocity;
    m_Color = other.m_Color;
    m_Angle = other.m_Angle;
    m_Scale = other.m_Scale;
    m_Depth = other.m_Depth;
    m_AngularVelocity = other.m_AngularVelocity;
    m_Properties = other.m_Properties;
}
ParticleData& ParticleData::operator=(const ParticleData& other) {
    if (&other == this)
        return *this;
    m_Active = other.m_Active;
    m_Timer = other.m_Timer;
    m_Velocity = other.m_Velocity;
    m_Color = other.m_Color;
    m_Angle = other.m_Angle;
    m_Scale = other.m_Scale;
    m_Depth = other.m_Depth;
    m_AngularVelocity = other.m_AngularVelocity;
    m_Properties = other.m_Properties;
    return *this;
}
ParticleData::ParticleData(ParticleData&& other) noexcept{
    using std::swap;
    swap(m_Active, other.m_Active);
    swap(m_Timer, other.m_Timer);
    swap(m_Velocity, other.m_Velocity);
    swap(m_Color, other.m_Color);
    swap(m_Angle, other.m_Angle);
    swap(m_Scale, other.m_Scale);
    swap(m_Depth, other.m_Depth);
    swap(m_AngularVelocity, other.m_AngularVelocity);
    swap(m_Properties, other.m_Properties);
}
ParticleData& ParticleData::operator=(ParticleData&& other) noexcept {
    using std::swap;
    swap(m_Active, other.m_Active);
    swap(m_Timer, other.m_Timer);
    swap(m_Velocity, other.m_Velocity);
    swap(m_Color, other.m_Color);
    swap(m_Angle, other.m_Angle);
    swap(m_Scale, other.m_Scale);
    swap(m_Depth, other.m_Depth);
    swap(m_AngularVelocity, other.m_AngularVelocity);
    swap(m_Properties, other.m_Properties);
    return *this;
}



Particle::Particle(){
    m_Scene = Resources::getCurrentScene();
    m_Hidden = false;
    m_PassedRenderCheck = false;
    m_Position = glm::vec3(0.0f);
    m_Material = Material::Checkers;
}

Particle::Particle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmissionProperties& properties, Scene& scene){
    m_Scene = &scene;
    m_Material = &const_cast<Material&>(properties.getParticleMaterialRandom());

    auto data = ParticleData(properties);
    init(data, emitterPosition, emitterRotation);
}
Particle::~Particle() {

}
void Particle::init(ParticleData& data, const glm::vec3& emitterPosition, const glm::quat& emitterRotation) {
    m_Data = data;
    m_Data.m_Active = true;
    m_Position = emitterPosition;
    m_Data.m_Velocity = Math::rotate_vec3(emitterRotation, m_Data.m_Velocity);
    m_Hidden = false;
    m_PassedRenderCheck = false;
}

Particle::Particle(const Particle& other){
    m_Data = other.m_Data;
    m_Scene = other.m_Scene;
    m_Hidden = other.m_Hidden;
    m_Position = other.m_Position;
    m_PassedRenderCheck = other.m_PassedRenderCheck;
    m_Material = other.m_Material;
}
Particle& Particle::operator=(const Particle& other) {
    if (&other == this)
        return *this;
    m_Data = other.m_Data;
    m_Scene = other.m_Scene;
    m_Hidden = other.m_Hidden;
    m_Position = other.m_Position;
    m_PassedRenderCheck = other.m_PassedRenderCheck;
    m_Material = other.m_Material;
    return *this;
}
Particle::Particle(Particle&& other) noexcept {
    using std::swap;
    swap(m_Data, other.m_Data);
    swap(m_Scene, other.m_Scene);
    swap(m_Hidden, other.m_Hidden);
    swap(m_Position, other.m_Position);
    swap(m_PassedRenderCheck, other.m_PassedRenderCheck);
    swap(m_Material, other.m_Material);
}
Particle& Particle::operator=(Particle&& other) noexcept {
    using std::swap;
    swap(m_Data, other.m_Data);
    swap(m_Scene, other.m_Scene);
    swap(m_Hidden, other.m_Hidden);
    swap(m_Position, other.m_Position);
    swap(m_PassedRenderCheck, other.m_PassedRenderCheck);
    swap(m_Material, other.m_Material);
    return *this;
}

Scene& Particle::scene() const {
    return *m_Scene;
}
void Particle::setPosition(const glm::vec3& newPosition) {
    m_Position = newPosition;
}
Material* Particle::getMaterial() {
    return m_Material;
}
const float& Particle::angle() const {
    return m_Data.m_Angle;
}
const glm::vec2& Particle::getScale() const {
    return m_Data.m_Scale;
}
const glm::vec3& Particle::position() const {
    return m_Position;
}
const bool& Particle::isActive() const {
    return m_Data.m_Active;
}
const glm::vec4& Particle::color() const {
    return m_Data.m_Color;
}
const glm::vec3& Particle::velocity() const {
    return m_Data.m_Velocity;
}
const double Particle::lifetime() const {
    return m_Data.m_Properties->m_Lifetime;
}



void Particle::update(const size_t& index, const double& dt, Engine::epriv::ParticleSystem& particleSystem) {
    if (m_Data.m_Active) {
        m_Data.m_Timer += dt;
        const auto fdt = static_cast<float>(dt);
        auto& prop = *m_Data.m_Properties;
        
        m_Data.m_Scale           += prop.m_ChangeInScaleFunctor(m_Data.m_Timer, dt);
        m_Data.m_Color            = prop.m_ColorFunctor(m_Data.m_Timer, dt);
        m_Data.m_AngularVelocity += prop.m_ChangeInAngularVelocityFunctor(m_Data.m_Timer, dt);
        m_Data.m_Angle           += m_Data.m_AngularVelocity;
        m_Data.m_Velocity        += prop.m_ChangeInVelocityFunctor(m_Data.m_Timer, dt);
        //m_Data.m_Depth            = prop.m_DepthFunctor(m_Data.m_Timer, dt);

        m_Position               += (m_Data.m_Velocity * fdt);

        
        //auto& camera = *m_Scene->getActiveCamera();
        //auto vec = glm::normalize(bodyComponent.position() - camera.getPosition()) * static_cast<decimal>(m_Data.m_Depth);
        //instance.setPosition(glm::vec3(vec));
        if (m_Data.m_Timer >= prop.m_Lifetime) {
            m_Data.m_Active = false;
            m_Data.m_Timer = 0.0;
            m_Hidden = true;
            particleSystem.m_ParticleFreelist.push(index);
        }
    }
}
void Particle::update_multithreaded(const size_t& index, const double& dt, Engine::epriv::ParticleSystem& particleSystem) {
    if (m_Data.m_Active) {
        m_Data.m_Timer += dt;
        const auto fdt = static_cast<float>(dt);
        auto& prop = *m_Data.m_Properties;

        m_Data.m_Scale           += prop.m_ChangeInScaleFunctor(m_Data.m_Timer, dt);
        m_Data.m_Color            = prop.m_ColorFunctor(m_Data.m_Timer, dt);
        m_Data.m_AngularVelocity += prop.m_ChangeInAngularVelocityFunctor(m_Data.m_Timer, dt);
        m_Data.m_Angle           += m_Data.m_AngularVelocity;
        m_Data.m_Velocity        += prop.m_ChangeInVelocityFunctor(m_Data.m_Timer, dt);
        //m_Data.m_Depth            = prop.m_DepthFunctor(m_Data.m_Timer, dt);

        m_Position               += (m_Data.m_Velocity * fdt);


        //auto& camera = *m_Scene->getActiveCamera();
        //auto vec = glm::normalize(bodyComponent.position() - camera.getPosition()) * static_cast<decimal>(m_Data.m_Depth);
        //instance.setPosition(glm::vec3(vec));
        if (m_Data.m_Timer >= prop.m_Lifetime) {
            m_Data.m_Active = false;
            m_Data.m_Timer = 0.0;
            m_Hidden = true;
            particleSystem.m_Mutex.lock();
            particleSystem.m_ParticleFreelist.push(index);
            particleSystem.m_Mutex.unlock();
        }
    }
}
void Particle::render(Engine::epriv::GBuffer& gBuffer) {
    m_Material->bind();

    auto maxTextures = epriv::Core::m_Engine->m_RenderManager.OpenGLStateMachine.getMaxTextureUnits() - 1;

    Camera& camera = *m_Scene->getActiveCamera();
    Renderer::sendTextureSafe("gDepthMap", gBuffer.getTexture(Engine::epriv::GBufferType::Depth), maxTextures);
    Renderer::sendUniform4Safe("Object_Color", m_Data.m_Color);
    Renderer::sendUniform2Safe("ScreenData", glm::vec2(Resources::getWindowSize()));

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, m_Position);
    modelMatrix *= glm::mat4_cast(camera.getOrientation());
    modelMatrix = glm::rotate(modelMatrix, m_Data.m_Angle, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(m_Data.m_Scale.x, m_Data.m_Scale.y, 1.0f));

    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);

    Mesh::Plane->render();
}
