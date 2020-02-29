#include <core/engine/renderer/particles/Particle.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/system/Engine.h>
#include <core/engine/renderer/GBuffer.h>

#include <core/engine/model/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>
#include <core/engine/math/Engine_Math.h>

#include <iostream>

using namespace std;
using namespace Engine;

Particle::Particle(){
    m_Material          = Material::Checkers;
}

Particle::Particle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmitter& emitter){
    m_EmitterSource  = &emitter;
    auto& properties = *emitter.m_Properties;
    m_Position       = emitterPosition;

    init(emitterPosition, emitterRotation, emitter);
}
Particle::~Particle() {

}
void Particle::init(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmitter& emitter) {
    m_PassedRenderCheck = false;
    m_Active     = true;
    m_Position          = emitterPosition;

    m_Properties = emitter.m_Properties;

    m_Material = &const_cast<Material&>(m_Properties->getParticleMaterialRandom());

    auto& emitterBody = *emitter.getComponent<ComponentBody>();
    const auto emitterScale = glm::vec3(emitterBody.getScale());


    m_Velocity = m_Properties->m_InitialVelocityFunctor(*m_Properties, emitter, *this) * emitterScale;
    auto rotated_initial_velocity = Math::rotate_vec3(emitter.rotation(), m_Velocity);
    if (!emitter.m_Parent.null()) {
        auto* body = emitter.m_Parent.getComponent<ComponentBody>();
        if (body) {
            m_Velocity = glm::vec3(body->getLinearVelocity()) * m_Properties->m_Drag;
        }
    }
    m_Velocity += rotated_initial_velocity;


    m_Scale           = m_Properties->m_InitialScaleFunctor(*m_Properties, emitter, *this) * Math::Max(emitterScale.x, emitterScale.y, emitterScale.z);
    m_AngularVelocity = m_Properties->m_InitialAngularVelocityFunctor(*m_Properties, emitter, *this);
    m_Color           = m_Properties->m_ColorFunctor(*m_Properties, m_Timer, 0.0f, &emitter, *this);
}
Particle::Particle(Particle&& other) noexcept{
    m_Active            = std::move(other.m_Active);
    m_Velocity          = std::move(other.m_Velocity);
    m_Timer             = std::move(other.m_Timer);
    m_Color             = std::move(other.m_Color);
    m_Angle             = std::move(other.m_Angle);
    m_Scale             = std::move(other.m_Scale);
    m_Depth             = std::move(other.m_Depth);
    m_AngularVelocity   = std::move(other.m_AngularVelocity);
    m_UserData          = std::move(other.m_UserData);
    m_Properties        = std::exchange(other.m_Properties, nullptr);

    m_Position          = std::move(other.m_Position);
    m_PassedRenderCheck = std::move(other.m_PassedRenderCheck);
    m_Material          = std::exchange(other.m_Material, nullptr);
    m_EmitterSource     = std::exchange(other.m_EmitterSource, nullptr);
}
Particle& Particle::operator=(Particle&& other) noexcept {
    if (&other != this) {
        m_Active            = std::move(other.m_Active);
        m_Velocity          = std::move(other.m_Velocity);
        m_Timer             = std::move(other.m_Timer);
        m_Color             = std::move(other.m_Color);
        m_Angle             = std::move(other.m_Angle);
        m_Scale             = std::move(other.m_Scale);
        m_Depth             = std::move(other.m_Depth);
        m_AngularVelocity   = std::move(other.m_AngularVelocity);
        m_UserData          = std::move(other.m_UserData);
        m_Properties        = std::exchange(other.m_Properties, nullptr);

        m_Position          = std::move(other.m_Position);
        m_PassedRenderCheck = std::move(other.m_PassedRenderCheck);
        m_Material          = std::exchange(other.m_Material, nullptr);
        m_EmitterSource     = std::exchange(other.m_EmitterSource, nullptr);
    }
    return *this;
}


void Particle::setUserDataX(const float x) {
    m_UserData.x = x;
}
void Particle::setUserDataY(const float y) {
    m_UserData.y = y;
}
void Particle::setUserDataZ(const float z) {
    m_UserData.z = z;
}
void Particle::setUserDataW(const float w) {
    m_UserData.w = w;
}
void Particle::setUserData(const float x, const float y, const float z, const float w) {
    m_UserData.x = x;
    m_UserData.y = y;
    m_UserData.z = z;
    m_UserData.w = w;
}
void Particle::setUserData(const glm::vec4& data) {
    m_UserData = data;
}

const float Particle::getUserDataX() const {
    return m_UserData.x;
}
const float Particle::getUserDataY() const {
    return m_UserData.y;
}
const float Particle::getUserDataZ() const {
    return m_UserData.z;
}
const float Particle::getUserDataW() const {
    return m_UserData.w;
}
const glm::vec4& Particle::getUserData() const {
    return m_UserData;
}
void Particle::setPosition(const glm::vec3& newPosition) {
    m_Position = newPosition;
}
Material* Particle::getMaterial() const {
    return m_Material;
}
const float Particle::angle() const {
    return m_Angle;
}
const glm::vec2& Particle::getScale() const {
    return m_Scale;
}
const glm::vec3& Particle::position() const {
    return m_Position;
}
const bool Particle::isActive() const {
    return m_Active;
}
const Engine::color_vector_4& Particle::color() const {
    return m_Color;
}
const glm::vec3& Particle::velocity() const {
    return m_Velocity;
}
const double Particle::lifetime() const {
    return m_Properties->m_Lifetime;
}

void Particle::update(const size_t index, const float dt, Engine::priv::ParticleSystem& particleSystem, const glm::vec3& cameraPosition, const bool multi_threaded) {
    if (m_Active) {
        m_Timer           += dt;
        auto& prop         = *m_Properties;
        
        m_Scale           += prop.m_ChangeInScaleFunctor(prop, m_Timer, dt , m_EmitterSource, *this);
        m_Color            = prop.m_ColorFunctor(prop, m_Timer, dt, m_EmitterSource, *this);
        m_AngularVelocity += prop.m_ChangeInAngularVelocityFunctor(prop, m_Timer, dt, m_EmitterSource, *this);
        m_Angle           += m_AngularVelocity;
        m_Velocity        += prop.m_ChangeInVelocityFunctor(prop, m_Timer, dt, m_EmitterSource, *this);
        m_Depth            = prop.m_DepthFunctor(prop, m_Timer, dt, m_EmitterSource, *this);

        m_Position        += (m_Velocity * dt);

        auto vec           = glm::normalize(m_Position - cameraPosition) * m_Depth;
        m_Position        += vec;
        if (m_Timer >= prop.m_Lifetime) {
            m_Active       = false;
            m_Timer        = 0.0f;

            if (multi_threaded) {
                std::lock_guard lock(particleSystem.m_Mutex);
                particleSystem.m_ParticleFreelist.push(index);
            }else{
                particleSystem.m_ParticleFreelist.push(index);
            }
        }
    }
}
