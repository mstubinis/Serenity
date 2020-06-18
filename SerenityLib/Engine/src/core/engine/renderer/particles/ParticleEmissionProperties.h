#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_EMISSION_PROPERTIES_H
#define ENGINE_RENDERER_PARTICLE_EMISSION_PROPERTIES_H

class  Handle;
class  Particle;
class  ParticleEmitter;
class  Material;
namespace Engine::priv {
    class ParticleSystem;
};

#include <functional>
#include <core/engine/renderer/RendererIncludes.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

/*
Class that contains the properties of particle emitters
*/
class ParticleEmissionProperties final {
    friend class  Particle;
    friend class  ParticleEmitter;
    friend class  Engine::priv::ParticleSystem;
    private:
        std::function<Engine::color_vector_4(ParticleEmissionProperties&, const float lifetime, const float dt, ParticleEmitter*, Particle&)>   
            m_ColorFunctor = [](ParticleEmissionProperties&, const float, const float, ParticleEmitter*, Particle&) {
                return Engine::color_vector_4(255_uc);
            };
        std::function<float(ParticleEmissionProperties&, const float lifetime, const float dt, ParticleEmitter*, Particle&)>       
            m_ChangeInAngularVelocityFunctor = [](ParticleEmissionProperties&, const float, const float, ParticleEmitter*, Particle&) {
                return 0.0f;
            };
        std::function<glm::vec3(ParticleEmissionProperties&, const float lifetime, const float dt, ParticleEmitter*, Particle&)>   
            m_ChangeInVelocityFunctor = [](ParticleEmissionProperties&, const float, const float, ParticleEmitter*, Particle&) {
                return glm::vec3(0.0f);
            };
        std::function<glm::vec2(ParticleEmissionProperties&, const float lifetime, const float dt, ParticleEmitter*, Particle&)>   
            m_ChangeInScaleFunctor = [](ParticleEmissionProperties&, const float, const float, ParticleEmitter*, Particle&) {
                return glm::vec2(0.0f);
            };

        std::function<glm::vec3(ParticleEmissionProperties&, ParticleEmitter&, Particle&)>   
            m_InitialVelocityFunctor = [](ParticleEmissionProperties&, ParticleEmitter&, Particle&) {
                return glm::vec3(0.0f);
            };
        std::function<glm::vec2(ParticleEmissionProperties&, ParticleEmitter&, Particle&)>   
            m_InitialScaleFunctor = [](ParticleEmissionProperties&, ParticleEmitter&, Particle&) {
                return glm::vec2(0.3f);
            };
        std::function<float(ParticleEmissionProperties&, ParticleEmitter&, Particle&)>       
            m_InitialAngularVelocityFunctor = [](ParticleEmissionProperties&, ParticleEmitter&, Particle&) {
                return 0.0f;
            };

        std::vector<Material*>  m_ParticleMaterials;
        float                   m_Lifetime             = 4.0f;
        float                   m_SpawnRate            = 0.4f;
        unsigned int            m_ParticlesPerSpawn    = 0;
    public:
        ParticleEmissionProperties() = default;
        ParticleEmissionProperties(Handle& materialHandle, const float lifeTime, const float spawnRate, const unsigned int ParticlesPerSpawn = 1, const float drag = 1.0f);
        ~ParticleEmissionProperties() = default;
   
        ParticleEmissionProperties(const ParticleEmissionProperties& other) = delete;
        ParticleEmissionProperties& operator=(const ParticleEmissionProperties& other) = delete;
        ParticleEmissionProperties(ParticleEmissionProperties&& other) noexcept;
        ParticleEmissionProperties& operator=(ParticleEmissionProperties&& other) noexcept;
        
        float getLifetime() const;
        float getSpawnRate() const;

        bool addMaterial(Handle& materialHandle);
        bool addMaterial(Material& material);

        const Material& getParticleMaterialRandom() const;
        const Material& getParticleMaterial(const size_t index = 0) const;

        template<typename T> void setColorFunctor(const T& functor) {
            m_ColorFunctor = std::bind<Engine::color_vector_4>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
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

        template<typename T> void setInitialVelocityFunctor(const T& functor) {
            m_InitialVelocityFunctor = std::bind<glm::vec3>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }
        template<typename T> void setInitialScaleFunctor(const T& functor) {
            m_InitialScaleFunctor = std::bind<glm::vec2>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }
        template<typename T> void setInitialAngularVelocityFunctor(const T& functor) {
            m_InitialAngularVelocityFunctor = std::bind<float>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }
};

#endif