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

Particle::Particle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmitter& emitter){
    auto& properties = *emitter.m_Properties;
    init(emitterPosition, emitterRotation, emitter);
}
void Particle::init(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmitter& emitter) {
    m_Timer = 0.001f;
    m_Position          = emitterPosition;

    m_EmitterSource = &emitter;
    m_Material = &const_cast<Material&>(emitter.m_Properties->getParticleMaterialRandom());

    auto& emitterBody = *emitter.getComponent<ComponentBody>();
    const auto emitterScale = glm::vec3(emitterBody.getScale());


    m_Velocity = emitter.m_Properties->m_InitialVelocityFunctor(*emitter.m_Properties, emitter, *this) * emitterScale;
    auto rotated_initial_velocity = Math::rotate_vec3(emitter.rotation(), m_Velocity);
    if (!emitter.m_Parent.null()) {
        auto* body = emitter.m_Parent.getComponent<ComponentBody>();
        if (body) {
            m_Velocity = glm::vec3(body->getLinearVelocity()) /* * emitter.m_Properties->m_Drag */ ;
        }
    }
    m_Velocity += rotated_initial_velocity;


    m_Scale           = emitter.m_Properties->m_InitialScaleFunctor(*emitter.m_Properties, emitter, *this) * Math::Max(emitterScale.x, emitterScale.y, emitterScale.z);
    m_AngularVelocity = emitter.m_Properties->m_InitialAngularVelocityFunctor(*emitter.m_Properties, emitter, *this);
    m_Color           = emitter.m_Properties->m_ColorFunctor(*emitter.m_Properties, m_Timer, 0.0f, &emitter, *this);
}
Particle::Particle(Particle&& other) noexcept{
    m_Velocity          = std::move(other.m_Velocity);
    m_Timer             = std::move(other.m_Timer);
    m_Color             = std::move(other.m_Color);
    m_Angle             = std::move(other.m_Angle);
    m_Scale             = std::move(other.m_Scale);
    m_AngularVelocity   = std::move(other.m_AngularVelocity);
    m_Position          = std::move(other.m_Position);
    m_Material          = std::exchange(other.m_Material, nullptr);
    m_EmitterSource     = std::exchange(other.m_EmitterSource, nullptr);
}
Particle& Particle::operator=(Particle&& other) noexcept {
    m_Velocity          = std::move(other.m_Velocity);
    m_Timer             = std::move(other.m_Timer);
    m_Color             = std::move(other.m_Color);
    m_Angle             = std::move(other.m_Angle);
    m_Scale             = std::move(other.m_Scale);
    m_AngularVelocity   = std::move(other.m_AngularVelocity);
    m_Position          = std::move(other.m_Position);
    m_Material          = std::exchange(other.m_Material, nullptr);
    m_EmitterSource     = std::exchange(other.m_EmitterSource, nullptr);
    return *this;
}
void Particle::setPosition(const glm::vec3& newPosition) {
    m_Position = newPosition;
}
float Particle::lifetime() const {
    return m_EmitterSource->m_Properties->m_Lifetime;
}
