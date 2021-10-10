
#include <serenity/renderer/particles/Particle.h>
#include <serenity/renderer/particles/ParticleEmitter.h>
#include <serenity/renderer/particles/ParticleEmissionProperties.h>
#include <serenity/renderer/particles/ParticleSystem.h>
#include <serenity/system/Engine.h>
#include <serenity/renderer/GBuffer.h>

#include <serenity/model/ModelInstance.h>
#include <serenity/scene/Camera.h>
#include <serenity/scene/Scene.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/material/Material.h>
#include <serenity/math/Engine_Math.h>

Particle::Particle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmitter& emitter) {
    init(emitterPosition, emitterRotation, emitter);
}
void Particle::init(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmitter& emitter) noexcept {
    float maxFloat = std::numeric_limits<float>().max();
    setPosition(maxFloat, maxFloat, maxFloat);

    m_Timer                       = 0.001f;
    m_Position                    = emitterPosition;

    m_EmitterSource               = &emitter;
    m_Material                    = &const_cast<Material&>(emitter.m_Properties->getParticleMaterialRandom());

    auto& emitterTransform        = *emitter.getComponent<ComponentTransform>();
    auto emitterScale             = emitterTransform.getScale();

    m_Velocity                    = emitter.m_Properties->m_InitialVelocityFunctor(*this) * emitterScale;
    auto rotated_initial_velocity = Engine::Math::rotate_vec3(emitter.getComponent<ComponentTransform>()->getRotation(), m_Velocity);
    if (!emitter.m_Parent.null()) {
        auto parentBody = emitter.m_Parent.getComponent<ComponentTransform>();
        if (parentBody) {
            //m_Velocity = glm::vec3(parentBody->getLinearVelocity()) /* * emitter.m_Properties->m_Drag */ ;
        }
    }
    m_Velocity += rotated_initial_velocity;

    m_Scale           = emitter.m_Properties->m_InitialScaleFunctor(*this) * Engine::Math::Max(emitterScale.x, emitterScale.y, emitterScale.z);
    m_AngularVelocity = emitter.m_Properties->m_InitialAngularVelocityFunctor(*this);
    m_Color           = emitter.m_Properties->m_ColorFunctor(0.0f, *this);
}

Particle::Particle(Particle&& other) noexcept 
    : m_Velocity        { std::move(other.m_Velocity) }
    , m_Timer           { std::move(other.m_Timer) }
    , m_Color           { std::move(other.m_Color) }
    , m_Angle           { std::move(other.m_Angle) }
    , m_Scale           { std::move(other.m_Scale) }
    , m_AngularVelocity { std::move(other.m_AngularVelocity) }
    , m_Position        { std::move(other.m_Position) }
    , m_Material        { std::move(other.m_Material) }
    , m_EmitterSource   { std::move(other.m_EmitterSource) }
{}
Particle& Particle::operator=(Particle&& other) noexcept {
    m_Velocity        = std::move(other.m_Velocity);
    m_Timer           = std::move(other.m_Timer);
    m_Color           = std::move(other.m_Color);
    m_Angle           = std::move(other.m_Angle);
    m_Scale           = std::move(other.m_Scale);
    m_AngularVelocity = std::move(other.m_AngularVelocity);
    m_Position        = std::move(other.m_Position);
    m_Material        = std::move(other.m_Material);
    m_EmitterSource   = std::move(other.m_EmitterSource);
    return *this;
}
float Particle::lifetime() const noexcept {
    return m_EmitterSource->m_Properties->m_Lifetime;
}
void Particle::setScale(float x, float y) noexcept {
    m_Scale.x = x;
    m_Scale.y = y;
}
void Particle::setScale(const glm::vec2& newScale) noexcept {
    setScale(newScale.x, newScale.y);
}
void Particle::setPosition(const glm::vec3& newPosition) noexcept {
    setPosition(newPosition.x, newPosition.y, newPosition.z);
}
void Particle::setPosition(float x, float y, float z) noexcept {
    m_Position.x = x;
    m_Position.y = y;
    m_Position.z = z;
}
void Particle::translate(const glm::vec3& newPosition) noexcept {
    translate(newPosition.x, newPosition.y, newPosition.z);
}
void Particle::translate(float x, float y, float z) noexcept {
    m_Position.x += x;
    m_Position.y += y;
    m_Position.z += z;
}

