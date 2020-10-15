#include <core/engine/utils/PrecompiledHeader.h>
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

    auto& emitterBody             = *emitter.getComponent<ComponentBody>();
    auto emitterScale             = glm::vec3(emitterBody.getScale());

    m_Velocity                    = emitter.m_Properties->m_InitialVelocityFunctor(*this) * emitterScale;
    auto rotated_initial_velocity = Engine::Math::rotate_vec3(emitter.getRotation(), m_Velocity);
    if (!emitter.m_Parent.null()) {
        auto parentBody = emitter.m_Parent.getComponent<ComponentBody>();
        if (parentBody) {
            m_Velocity = glm::vec3(parentBody->getLinearVelocity()) /* * emitter.m_Properties->m_Drag */ ;
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

void Particle::setPosition(const glm::vec3& newPosition) noexcept {
    setPosition(newPosition.x, newPosition.y, newPosition.z);
}
void Particle::setPosition(float x, float y, float z) noexcept {
    m_Position.x = x;
    m_Position.y = y;
    m_Position.z = z;
}
float Particle::lifetime() const noexcept {
    return m_EmitterSource->m_Properties->m_Lifetime;
}
