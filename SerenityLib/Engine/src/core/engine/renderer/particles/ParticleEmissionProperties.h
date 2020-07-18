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

#include <core/engine/renderer/RendererIncludes.h>

/*
Class that contains the properties of particle emitters
*/
class ParticleEmissionProperties final {
    friend class  Particle;
    friend class  ParticleEmitter;
    friend class  Engine::priv::ParticleSystem;

    using color_func = std::function<Engine::color_vector_4(ParticleEmissionProperties&, const float lifetime, const float dt, ParticleEmitter*, Particle&)>;
    using change_in_angular_velocity_func = std::function<float(ParticleEmissionProperties&, const float lifetime, const float dt, ParticleEmitter*, Particle&)>;
    using change_in_velocity_func = std::function<glm::vec3(ParticleEmissionProperties&, const float lifetime, const float dt, ParticleEmitter*, Particle&)>;
    using change_in_scale_func = std::function<glm::vec2(ParticleEmissionProperties&, const float lifetime, const float dt, ParticleEmitter*, Particle&)>;
    using initial_velocity_func = std::function<glm::vec3(ParticleEmissionProperties&, ParticleEmitter&, Particle&)>;
    using initial_scale_func = std::function<glm::vec2(ParticleEmissionProperties&, ParticleEmitter&, Particle&)>;
    using initial_angular_velocity_func = std::function<float(ParticleEmissionProperties&, ParticleEmitter&, Particle&)>;
    private:
        color_func m_ColorFunctor = [](ParticleEmissionProperties&, const float, const float, ParticleEmitter*, Particle&) {
            return Engine::color_vector_4(255_uc);
        };
        change_in_angular_velocity_func m_ChangeInAngularVelocityFunctor = [](ParticleEmissionProperties&, const float, const float, ParticleEmitter*, Particle&) {
            return 0.0f;
        };
        change_in_velocity_func m_ChangeInVelocityFunctor = [](ParticleEmissionProperties&, const float, const float, ParticleEmitter*, Particle&) {
            return glm::vec3(0.0f);
        };
        change_in_scale_func m_ChangeInScaleFunctor = [](ParticleEmissionProperties&, const float, const float, ParticleEmitter*, Particle&) {
            return glm::vec2(0.0f);
        };
        initial_velocity_func m_InitialVelocityFunctor = [](ParticleEmissionProperties&, ParticleEmitter&, Particle&) {
            return glm::vec3(0.0f);
        };
        initial_scale_func m_InitialScaleFunctor = [](ParticleEmissionProperties&, ParticleEmitter&, Particle&) {
            return glm::vec2(0.3f);
        };
        initial_angular_velocity_func m_InitialAngularVelocityFunctor = [](ParticleEmissionProperties&, ParticleEmitter&, Particle&) {
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
        
        constexpr float getLifetime() const noexcept { return m_Lifetime; }
        constexpr float getSpawnRate() const noexcept { return m_SpawnRate; }

        bool addMaterial(Handle& materialHandle);
        bool addMaterial(Material& material);

        const Material& getParticleMaterialRandom() const;
        const Material& getParticleMaterial(const size_t index = 0) const;

        void setColorFunctor(color_func functor) noexcept {
            m_ColorFunctor = functor;
        }
        void setChangeInAngularVelocityFunctor(change_in_angular_velocity_func functor) noexcept {
            m_ChangeInAngularVelocityFunctor = functor;
        }
        void setChangeInVelocityFunctor(change_in_velocity_func functor) noexcept {
            m_ChangeInVelocityFunctor = functor;
        }
        void setChangeInScaleFunctor(change_in_scale_func functor) noexcept {
            m_ChangeInScaleFunctor = functor;
        }
        void setInitialVelocityFunctor(initial_velocity_func functor) noexcept {
            m_InitialVelocityFunctor = functor;
        }
        void setInitialScaleFunctor(initial_scale_func functor) noexcept {
            m_InitialScaleFunctor = functor;
        }
        void setInitialAngularVelocityFunctor(initial_angular_velocity_func functor) noexcept {
            m_InitialAngularVelocityFunctor = functor;
        }
};

#endif