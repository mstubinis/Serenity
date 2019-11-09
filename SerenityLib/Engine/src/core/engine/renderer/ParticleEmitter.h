#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_EMITTER_H
#define ENGINE_RENDERER_PARTICLE_EMITTER_H

#include <ecs/Entity.h>
#include <core/engine/renderer/Particle.h>
#include <mutex>

class ParticleEmissionProperties;
class ParticleEmitter final : public EntityWrapper{
    private:
        ParticleEmissionProperties*    m_Properties;
        std::vector<Particle>          m_Particles;
        size_t                         m_LastIndex;
        bool                           m_Active;
        double                         m_SpawningTimer;

        void internal_init();
        void internal_particles_update(const double& dt);
        void internal_particles_update_multithreaded(const double& dt, std::mutex& mutex_);
    public:
        ParticleEmitter();
        ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, const size_t maxParticles);
        ~ParticleEmitter();


        ParticleEmitter(const ParticleEmitter& other);
        ParticleEmitter& operator=(const ParticleEmitter& other);
        ParticleEmitter(ParticleEmitter&& other) noexcept;
        ParticleEmitter& operator=(ParticleEmitter&& other) noexcept;

        const bool spawnParticle(const RenderStage::Stage = RenderStage::ForwardParticles);
        const bool spawnParticle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, const RenderStage::Stage = RenderStage::ForwardParticles);
        const bool& isActive() const;

        void activate();
        void deactivate();

        void setProperties(ParticleEmissionProperties& properties);

        void update(const double& dt);
        void update_multithreaded(const double& dt, std::mutex& mutex_);
};

#endif