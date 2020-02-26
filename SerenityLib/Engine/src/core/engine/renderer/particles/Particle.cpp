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

ParticleData::ParticleData() {
    m_Properties      = &ParticleEmissionProperties::DefaultProperties;
}
ParticleData::ParticleData(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle){
    m_Properties      = &properties;

    particle.m_Material = &const_cast<Material&>(properties.getParticleMaterialRandom());

    auto& emitterBody = *emitter.getComponent<ComponentBody>();
    const auto emitterScale = glm::vec3(emitterBody.getScale());


    m_Velocity        = properties.m_InitialVelocityFunctor(properties, emitter, particle, *this) * emitterScale;
    auto rotated_initial_velocity = Math::rotate_vec3(emitter.rotation(), m_Velocity);
    if (!emitter.m_Parent.null()) {
        auto* body = emitter.m_Parent.getComponent<ComponentBody>();
        if (body) {
            m_Velocity = glm::vec3(body->getLinearVelocity()) * m_Properties->m_Drag;
        }
    }
    m_Velocity        += rotated_initial_velocity;


    m_Scale           = properties.m_InitialScaleFunctor(properties, emitter, particle, *this) * Math::Max(emitterScale.x, emitterScale.y, emitterScale.z);
    m_AngularVelocity = properties.m_InitialAngularVelocityFunctor(properties, emitter, particle, *this);

    m_Color           = properties.m_ColorFunctor(properties, m_Timer, 0.0f, &emitter, particle);
}

ParticleData::ParticleData(ParticleData&& other) noexcept{
    m_Active          = std::move(other.m_Active);
    m_Velocity        = std::move(other.m_Velocity);
    m_Timer           = std::move(other.m_Timer);
    m_Color           = std::move(other.m_Color);
    m_Angle           = std::move(other.m_Angle);
    m_Scale           = std::move(other.m_Scale);
    m_Depth           = std::move(other.m_Depth);
    m_AngularVelocity = std::move(other.m_AngularVelocity);
    m_UserData        = std::move(other.m_UserData);
    m_Properties      = std::exchange(other.m_Properties, nullptr);

}
ParticleData& ParticleData::operator=(ParticleData&& other) noexcept {
    if (&other != this) {
        m_Active          = std::move(other.m_Active);
        m_Velocity        = std::move(other.m_Velocity);
        m_Timer           = std::move(other.m_Timer);
        m_Color           = std::move(other.m_Color);
        m_Angle           = std::move(other.m_Angle);
        m_Scale           = std::move(other.m_Scale);
        m_Depth           = std::move(other.m_Depth);
        m_AngularVelocity = std::move(other.m_AngularVelocity);
        m_UserData        = std::move(other.m_UserData);
        m_Properties      = std::exchange(other.m_Properties, nullptr);
    }
    return *this;
}


Particle::Particle(){
    m_Scene             = Resources::getCurrentScene();
    m_Material          = Material::Checkers;
}

Particle::Particle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmitter& emitter){
    m_EmitterSource  = &emitter;
    m_Scene          = &emitter.entity().scene();
    auto& properties = *emitter.m_Properties;
    m_Position       = emitterPosition;

    auto data        = ParticleData(properties, emitter, *this);
    init(std::move(data), emitterPosition, emitterRotation, emitter.m_Parent);
}
Particle::~Particle() {

}
void Particle::init(ParticleData&& data, const glm::vec3& emitterPosition, const glm::quat& emitterRotation, Entity& parent) {
    m_Hidden            = false;
    m_PassedRenderCheck = false;
    m_Data.m_Active     = true;
    m_Position          = emitterPosition;
    m_Data              = std::move(data);
}
Particle::Particle(Particle&& other) noexcept{
    m_Data              = std::move(other.m_Data);
    m_Hidden            = std::move(other.m_Hidden);
    m_Position          = std::move(other.m_Position);
    m_PassedRenderCheck = std::move(other.m_PassedRenderCheck);
    m_Scene             = std::exchange(other.m_Scene, nullptr);
    m_Material          = std::exchange(other.m_Material, nullptr);
    m_EmitterSource     = std::exchange(other.m_EmitterSource, nullptr);
}
Particle& Particle::operator=(Particle&& other) noexcept {
    if (&other != this) {
        m_Data              = std::move(other.m_Data);
        m_Hidden            = std::move(other.m_Hidden);
        m_Position          = std::move(other.m_Position);
        m_PassedRenderCheck = std::move(other.m_PassedRenderCheck);
        m_Scene             = std::exchange(other.m_Scene, nullptr);
        m_Material          = std::exchange(other.m_Material, nullptr);
        m_EmitterSource     = std::exchange(other.m_EmitterSource, nullptr);
    }
    return *this;
}


void Particle::setUserDataX(const float x) {
    m_Data.m_UserData.x = x;
}
void Particle::setUserDataY(const float y) {
    m_Data.m_UserData.y = y;
}
void Particle::setUserDataZ(const float z) {
    m_Data.m_UserData.z = z;
}
void Particle::setUserDataW(const float w) {
    m_Data.m_UserData.w = w;
}
void Particle::setUserData(const float x, const float y, const float z, const float w) {
    m_Data.m_UserData.x = x;
    m_Data.m_UserData.y = y;
    m_Data.m_UserData.z = z;
    m_Data.m_UserData.w = w;
}
void Particle::setUserData(const glm::vec4& data) {
    m_Data.m_UserData = data;
}

const float Particle::getUserDataX() const {
    return m_Data.m_UserData.x;
}
const float Particle::getUserDataY() const {
    return m_Data.m_UserData.y;
}
const float Particle::getUserDataZ() const {
    return m_Data.m_UserData.z;
}
const float Particle::getUserDataW() const {
    return m_Data.m_UserData.w;
}
const glm::vec4& Particle::getUserData() const {
    return m_Data.m_UserData;
}



Scene& Particle::scene() const {
    return *m_Scene;
}
void Particle::setPosition(const glm::vec3& newPosition) {
    m_Position = newPosition;
}
Material* Particle::getMaterial() const {
    return m_Material;
}
const float Particle::angle() const {
    return m_Data.m_Angle;
}
const glm::vec2& Particle::getScale() const {
    return m_Data.m_Scale;
}
const glm::vec3& Particle::position() const {
    return m_Position;
}
const bool Particle::isActive() const {
    return m_Data.m_Active;
}
const Engine::color_vector_4& Particle::color() const {
    return m_Data.m_Color;
}
const glm::vec3& Particle::velocity() const {
    return m_Data.m_Velocity;
}
const double Particle::lifetime() const {
    return m_Data.m_Properties->m_Lifetime;
}



void Particle::update(const size_t index, const float dt, Engine::priv::ParticleSystem& particleSystem, const glm::vec3& cameraPosition, const bool multi_threaded) {
    if (m_Data.m_Active) {
        m_Data.m_Timer           += dt;
        auto& prop                = *m_Data.m_Properties;
        
        m_Data.m_Scale           += prop.m_ChangeInScaleFunctor(prop, m_Data.m_Timer, dt , m_EmitterSource, *this);
        m_Data.m_Color            = prop.m_ColorFunctor(prop, m_Data.m_Timer, dt, m_EmitterSource, *this);
        m_Data.m_AngularVelocity += prop.m_ChangeInAngularVelocityFunctor(prop, m_Data.m_Timer, dt, m_EmitterSource, *this);
        m_Data.m_Angle           += m_Data.m_AngularVelocity;
        m_Data.m_Velocity        += prop.m_ChangeInVelocityFunctor(prop, m_Data.m_Timer, dt, m_EmitterSource, *this);
        m_Data.m_Depth            = prop.m_DepthFunctor(prop, m_Data.m_Timer, dt, m_EmitterSource, *this);

        m_Position               += (m_Data.m_Velocity * dt);

        auto vec                  = glm::normalize(m_Position - cameraPosition) * m_Data.m_Depth;
        m_Position               += vec;
        if (m_Data.m_Timer >= prop.m_Lifetime) {
            m_Data.m_Active       = false;
            m_Data.m_Timer        = 0.0f;
            m_Hidden              = true;

            if (multi_threaded) {
                std::lock_guard lock(particleSystem.m_Mutex);
                particleSystem.m_ParticleFreelist.push(index);
            }else{
                particleSystem.m_ParticleFreelist.push(index);
            }
        }
    }
}
