#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_EMITTER_H
#define ENGINE_RENDERER_PARTICLE_EMITTER_H

#include <core/engine/renderer/Particle.h>
#include <ecs/Entity.h>
#include <vector>
#include <mutex>

class  ParticleEmissionProperties;
namespace Engine {
    namespace epriv {
        struct InternalScenePublicInterface;
    };
};
class ParticleEmitter final : public EntityWrapper{
    friend struct Engine::epriv::InternalScenePublicInterface;
    private:
        ParticleEmissionProperties*    m_Properties;
        bool                           m_Active;
        double                         m_SpawningTimer;
        double                         m_Timer;
        double                         m_Lifetime;
        void internal_init();
    public:
        ParticleEmitter();
        ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, const double lifetime);
        ~ParticleEmitter();


        ParticleEmitter(const ParticleEmitter& other);
        ParticleEmitter& operator=(const ParticleEmitter& other);
        ParticleEmitter(ParticleEmitter&& other) noexcept;
        ParticleEmitter& operator=(ParticleEmitter&& other) noexcept;

        const bool spawnParticle(std::vector<Particle>& particles, std::stack<unsigned int>& freelist);
        const bool spawnParticle(std::vector<Particle>& particles, std::stack<unsigned int>& freelist, const glm::vec3& emitterPosition, const glm::quat& emitterRotation);
        const bool& isActive() const;

        void activate();
        void deactivate();

        void setProperties(ParticleEmissionProperties& properties);

        void update(const unsigned int& index, const double& dt, std::vector<Particle>& particles, std::stack<unsigned int>& particle_freelist, std::stack<unsigned int>& emitter_freelist);
        void update_multithreaded(const unsigned int& index, const double& dt, std::mutex& mutex_, std::vector<Particle>& particles, std::stack<unsigned int>& particle_freelist, std::stack<unsigned int>& emitter_freelist);
};

#endif