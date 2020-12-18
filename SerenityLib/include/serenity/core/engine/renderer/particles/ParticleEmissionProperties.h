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

#include <serenity/core/engine/renderer/RendererIncludes.h>
#include <serenity/core/engine/types/ColorVector.h>
#include <vector>
/*
Class that contains the properties of particle emitters
*/
class ParticleEmissionProperties final {
    friend class  Particle;
    friend class  ParticleEmitter;
    friend class  Engine::priv::ParticleSystem;

    using color_func                      = Engine::color_vector_4(*)(const float dt, Particle&);
    using change_in_angular_velocity_func = float(*)(const float dt, Particle&);
    using change_in_velocity_func         = glm::vec3(*)(const float dt, Particle&);
    using change_in_scale_func            = glm::vec2(*)(const float dt, Particle&);
    using initial_velocity_func           = glm::vec3(*)(Particle&);
    using initial_scale_func              = glm::vec2(*)(Particle&);
    using initial_angular_velocity_func   = float(*)(Particle&);
    private:
        color_func                      m_ColorFunctor                   = [](const float, Particle&) { return Engine::color_vector_4(255_uc); };
        change_in_angular_velocity_func m_ChangeInAngularVelocityFunctor = [](const float, Particle&) { return 0.0f; };
        change_in_velocity_func         m_ChangeInVelocityFunctor        = [](const float, Particle&) { return glm::vec3(0.0f); };
        change_in_scale_func            m_ChangeInScaleFunctor           = [](const float, Particle&) { return glm::vec2(0.0f); };
        initial_velocity_func           m_InitialVelocityFunctor         = [](Particle&) { return glm::vec3(0.0f); };
        initial_scale_func              m_InitialScaleFunctor            = [](Particle&) { return glm::vec2(0.3f); };
        initial_angular_velocity_func   m_InitialAngularVelocityFunctor  = [](Particle&) { return 0.0f; };

        std::vector<Material*>  m_ParticleMaterials;
        float                   m_Lifetime             = 4.0f;
        float                   m_SpawnRate            = 0.4f;
        unsigned int            m_ParticlesPerSpawn    = 0;
    public:
        ParticleEmissionProperties() = default;
        ParticleEmissionProperties(Handle materialHandle, float lifeTime, float spawnRate, unsigned int ParticlesPerSpawn = 1, float drag = 1.0f);
        ~ParticleEmissionProperties() = default;
 
        ParticleEmissionProperties(const ParticleEmissionProperties& other)                = delete;
        ParticleEmissionProperties& operator=(const ParticleEmissionProperties& other)     = delete;
        ParticleEmissionProperties(ParticleEmissionProperties&& other) noexcept            = default;
        ParticleEmissionProperties& operator=(ParticleEmissionProperties&& other) noexcept = default;
        
        inline constexpr float getLifetime() const noexcept { return m_Lifetime; }
        inline constexpr float getSpawnRate() const noexcept { return m_SpawnRate; }

        bool addMaterial(Handle materialHandle);
        bool addMaterial(Material& material);

        const Material& getParticleMaterialRandom() const noexcept;
        const Material& getParticleMaterial(size_t index = 0) const noexcept;

        inline void setColorFunctor(color_func&& functor) noexcept { m_ColorFunctor = std::move(functor); }
        inline void setChangeInAngularVelocityFunctor(change_in_angular_velocity_func&& functor) noexcept { m_ChangeInAngularVelocityFunctor = std::move(functor); }
        inline void setChangeInVelocityFunctor(change_in_velocity_func&& functor) noexcept { m_ChangeInVelocityFunctor = std::move(functor); }
        inline void setChangeInScaleFunctor(change_in_scale_func&& functor) noexcept { m_ChangeInScaleFunctor = std::move(functor); }
        inline void setInitialVelocityFunctor(initial_velocity_func&& functor) noexcept { m_InitialVelocityFunctor = std::move(functor); }
        inline void setInitialScaleFunctor(initial_scale_func&& functor) noexcept { m_InitialScaleFunctor = std::move(functor); }
        inline void setInitialAngularVelocityFunctor(initial_angular_velocity_func&& functor) noexcept { m_InitialAngularVelocityFunctor = std::move(functor); }
};

#endif