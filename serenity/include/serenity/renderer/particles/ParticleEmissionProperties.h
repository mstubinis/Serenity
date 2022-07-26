#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_EMISSION_PROPERTIES_H
#define ENGINE_RENDERER_PARTICLE_EMISSION_PROPERTIES_H

class  Handle;
class  Particle;
class  ParticleEmitter;
class  Material;
namespace Engine::priv {
    class ParticleSystem;
    class ParticleSystemNew;
};

#include <serenity/renderer/RendererIncludes.h>
#include <serenity/types/ColorVector.h>
#include <vector>
/*
Class that contains the properties of particle emitters
*/
class ParticleEmissionProperties final {
    friend class  Particle;
    friend class  ParticleEmitter;
    friend class  Engine::priv::ParticleSystem;
    friend class  Engine::priv::ParticleSystemNew;
    public:
        using color_func                      = Engine::color_vector_4(*)(const float dt, float particleLifetime, float particleTimer);
        using change_in_angular_velocity_func = float(*)(const float dt);
        using change_in_velocity_func         = glm::vec3(*)(const float dt);
        using change_in_scale_func            = glm::vec2(*)(const float dt);
        using initial_velocity_func           = glm::vec3(*)();
        using initial_scale_func              = glm::vec2(*)();
        using initial_angular_velocity_func   = float(*)();
    private:
        color_func                      m_ColorFunctor                   = [](const float, float, float) { return Engine::color_vector_4{ 255_uc }; };
        change_in_angular_velocity_func m_ChangeInAngularVelocityFunctor = [](const float) { return 0.0f; };
        change_in_velocity_func         m_ChangeInVelocityFunctor        = [](const float) { return glm::vec3{ 0.0f }; };
        change_in_scale_func            m_ChangeInScaleFunctor           = [](const float) { return glm::vec2{ 0.0f }; };
        initial_velocity_func           m_InitialVelocityFunctor         = []() { return glm::vec3{ 0.0f }; };
        initial_scale_func              m_InitialScaleFunctor            = []() { return glm::vec2{ 0.3f }; };
        initial_angular_velocity_func   m_InitialAngularVelocityFunctor  = []() { return 0.0f; };

        std::vector<Material*>  m_ParticleMaterials;
        float                   m_Lifetime             = 4.0f;
        float                   m_Timer                = 0.0f;
        float                   m_SpawningTimer        = 0.0f;
        float                   m_SpawnRate            = 0.4f;
        uint8_t                 m_ParticlesPerSpawn    = 0;
    public:
        ParticleEmissionProperties() = default;
        ParticleEmissionProperties(Handle materialHandle, float lifeTime, float spawnRate, uint8_t ParticlesPerSpawn = 1, float drag = 1.0f);

        ParticleEmissionProperties(const ParticleEmissionProperties&)                = default;
        ParticleEmissionProperties& operator=(const ParticleEmissionProperties&)     = default;
        ParticleEmissionProperties(ParticleEmissionProperties&&) noexcept            = default;
        ParticleEmissionProperties& operator=(ParticleEmissionProperties&&) noexcept = default;
        
        [[nodiscard]] inline float getTimer() const noexcept { return m_Timer; }
        [[nodiscard]] inline float getLifetime() const noexcept { return m_Lifetime; }
        [[nodiscard]] inline float getSpawnRate() const noexcept { return m_SpawnRate; }
        [[nodiscard]] inline float getSpawnTimer() const noexcept { return m_SpawningTimer; }
        [[nodiscard]] Material& getParticleMaterialRandom() const noexcept;
        [[nodiscard]] Material& getParticleMaterial(size_t index = 0) const noexcept;

        bool addMaterial(Handle materialHandle);
        bool addMaterial(Material&);

        inline void setSpawnTimer(const float amount) noexcept { m_SpawningTimer = amount; }
        inline void setColorFunctor(color_func&& functor) noexcept { m_ColorFunctor = std::move(functor); }
        inline void setChangeInAngularVelocityFunctor(change_in_angular_velocity_func&& functor) noexcept { m_ChangeInAngularVelocityFunctor = std::move(functor); }
        inline void setChangeInVelocityFunctor(change_in_velocity_func&& functor) noexcept { m_ChangeInVelocityFunctor = std::move(functor); }
        inline void setChangeInScaleFunctor(change_in_scale_func&& functor) noexcept { m_ChangeInScaleFunctor = std::move(functor); }
        inline void setInitialVelocityFunctor(initial_velocity_func&& functor) noexcept { m_InitialVelocityFunctor = std::move(functor); }
        inline void setInitialScaleFunctor(initial_scale_func&& functor) noexcept { m_InitialScaleFunctor = std::move(functor); }
        inline void setInitialAngularVelocityFunctor(initial_angular_velocity_func&& functor) noexcept { m_InitialAngularVelocityFunctor = std::move(functor); }
};

#endif