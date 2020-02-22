#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_EMISSION_PROPERTIES_H
#define ENGINE_RENDERER_PARTICLE_EMISSION_PROPERTIES_H

struct Handle;
class  Particle;
struct ParticleData;
class  ParticleEmitter;
class  Material;

#include <functional>
#include <core/engine/renderer/RendererIncludes.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class ParticleEmissionProperties final {
    friend class  Particle;
    friend class  ParticleEmitter;
    friend struct ParticleData;
    private:
        static ParticleEmissionProperties DefaultProperties;
    private:

        std::function<glm::vec4(ParticleEmissionProperties&, const double& lifetime, const float& dt, ParticleEmitter*, Particle& particle)>   m_ColorFunctor;
        std::function<float(ParticleEmissionProperties&, const double& lifetime, const float& dt, ParticleEmitter*, Particle& particle)>       m_ChangeInAngularVelocityFunctor;
        std::function<glm::vec3(ParticleEmissionProperties&, const double& lifetime, const float& dt, ParticleEmitter*, Particle& particle)>   m_ChangeInVelocityFunctor;
        std::function<glm::vec2(ParticleEmissionProperties&, const double& lifetime, const float& dt, ParticleEmitter*, Particle& particle)>   m_ChangeInScaleFunctor;
        std::function<float(ParticleEmissionProperties&, const double& lifetime, const float& dt, ParticleEmitter*, Particle& particle)>       m_DepthFunctor;

        std::function<glm::vec3(ParticleEmissionProperties&, ParticleEmitter& emitter, Particle& particle, ParticleData& data)>   m_InitialVelocityFunctor;
        std::function<glm::vec2(ParticleEmissionProperties&, ParticleEmitter& emitter, Particle& particle, ParticleData& data)>   m_InitialScaleFunctor;
        std::function<float(ParticleEmissionProperties&, ParticleEmitter& emitter, Particle& particle, ParticleData& data)>       m_InitialAngularVelocityFunctor;

        std::vector<Material*>  m_ParticleMaterials;
        double                  m_Lifetime;
        double                  m_SpawnRate;
        float                   m_Drag;
        unsigned int            m_ParticlesPerSpawn;
    public:
        ParticleEmissionProperties();
        ParticleEmissionProperties(Handle& materialHandle, const double lifeTime, const double spawnRate, const unsigned int ParticlesPerSpawn = 1, const float drag = 1.0f);
        ~ParticleEmissionProperties();    

        
        ParticleEmissionProperties(const ParticleEmissionProperties& other) = delete;
        ParticleEmissionProperties& operator=(const ParticleEmissionProperties& other) = delete;
        ParticleEmissionProperties(ParticleEmissionProperties&& other) noexcept;
        ParticleEmissionProperties& operator=(ParticleEmissionProperties&& other) noexcept;
        

        const double& getLifetime() const;
        const double& getSpawnRate() const;

        const bool addMaterial(Handle& materialHandle);
        const bool addMaterial(Material& material);

        const Material& getParticleMaterialRandom() const;
        const Material& getParticleMaterial(const size_t index = 0) const;

        template<typename T> void setColorFunctor(const T& functor) {
            m_ColorFunctor = std::bind<glm::vec4>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        }
        template<typename T> void setChangeInAngularVelocityFunctor(const T& functor) {
            m_ChangeInAngularVelocityFunctor = std::bind<float>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        }
        template<typename T> void setChangeInVelocityFunctor(const T& functor) {
            m_ChangeInVelocityFunctor = std::bind<glm::vec3>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        }
        template<typename T> void setChangeInScaleFunctor(const T& functor) {
            m_ChangeInScaleFunctor = std::bind<glm::vec2>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        }
        template<typename T> void setDepthFunctor(const T& functor) {
            m_DepthFunctor = std::bind<float>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        }


        template<typename T> void setInitialVelocityFunctor(const T& functor) {
            m_InitialVelocityFunctor = std::bind<glm::vec3>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        }
        template<typename T> void setInitialScaleFunctor(const T& functor) {
            m_InitialScaleFunctor = std::bind<glm::vec2>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        }
        template<typename T> void setInitialAngularVelocityFunctor(const T& functor) {
            m_InitialAngularVelocityFunctor = std::bind<float>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        }
};

#endif