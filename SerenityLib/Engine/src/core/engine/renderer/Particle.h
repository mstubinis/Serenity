#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_H
#define ENGINE_RENDERER_PARTICLE_H

#include <ecs/Entity.h>
#include <core/engine/renderer/RendererIncludes.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <mutex>

class ParticleEmitter;
class ParticleEmissionProperties;
class ComponentModel;
struct ParticleData final {
    ParticleEmissionProperties*  m_Properties;
    glm::vec2                    m_Scale;
    glm::vec3                    m_Velocity;
    glm::vec4                    m_Color;
    float                        m_Depth;
    float                        m_Angle;
    float                        m_AngularVelocity;
    double                       m_Timer;
    bool                         m_Active;

    ParticleData();
    ParticleData(ParticleEmissionProperties& properties);

    ParticleData(const ParticleData& other);
    ParticleData& operator=(const ParticleData& other);
    ParticleData(ParticleData&& other) noexcept;
    ParticleData& operator=(ParticleData&& other) noexcept;
};

class Particle : public EntityWrapper {
    private:
        ParticleData m_Data;

    public:
        Particle();
        Particle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmissionProperties& properties, Scene& scene, const RenderStage::Stage = RenderStage::ForwardParticles);
        ~Particle();

        Particle(const Particle& other);
        Particle& operator=(const Particle& other);
        Particle(Particle&& other) noexcept;
        Particle& operator=(Particle&& other) noexcept;

        void init(ParticleData& data, const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ComponentBody& particleBody, ComponentModel& particleModel);

        const bool& isActive() const;
        const glm::vec4& color() const;
        const glm::vec3& velocity() const;
        const double lifetime() const;
        void update(const double& dt);
        void update_multithreaded(const double& dt, std::mutex& mutex_);

};

#endif