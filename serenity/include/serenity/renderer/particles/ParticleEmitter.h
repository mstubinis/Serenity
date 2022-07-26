#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_EMITTER_H
#define ENGINE_RENDERER_PARTICLE_EMITTER_H

class  Particle;
class  ParticleEmissionProperties;
namespace Engine::priv {
    struct PublicScene;
    class  ParticleSystem;
};

#include <serenity/ecs/entity/Entity.h>
#include <serenity/renderer/particles/ParticleEmissionPropertiesHandle.h>
#include <mutex>

/* Class that spawns particles */
class ParticleEmitter final : public Entity {
    friend class  Engine::priv::ParticleSystem;
    friend class  Particle;
    friend struct Engine::priv::PublicScene;
    public:
        //glm::vec4                      m_UserData       = glm::vec4{ 0.0f };
    private:
        //update_func                    m_UpdateFunctor  = [](ParticleEmitter*, const float, std::mutex&) {};
        ParticleEmissionPropertiesHandle    m_Properties;
        float                          m_SpawningTimer  = 0.0f; //TODO: move to properties, all emitters spawn uniformly, wont really make a difference, but will save space for sure
        float                          m_Timer          = 0.0f;
        float                          m_Lifetime       = 0.0f;
        //Entity                         m_Parent;
        bool                           m_Active         = true;

    public:
        ParticleEmitter() = default;
        ParticleEmitter(Scene&, ParticleEmissionPropertiesHandle, float lifetime, Entity parent = {});

        ParticleEmitter(const ParticleEmitter&)                = delete;
        ParticleEmitter& operator=(const ParticleEmitter&)     = delete;
        ParticleEmitter(ParticleEmitter&&) noexcept;
        ParticleEmitter& operator=(ParticleEmitter&&) noexcept;

        void init(Scene&, ParticleEmissionPropertiesHandle, float lifetime, Entity parent);

      //  inline void setUpdateFunctor(update_func&& functor) noexcept { m_UpdateFunctor = std::move(functor); }

        //void applyLinearVelocity(decimal x, decimal y, decimal z, bool local = true);
        //void applyLinearVelocity(glm_vec3& velocity, bool local = true);

        [[nodiscard]] inline bool isActive() const noexcept { return m_Active; }
        [[nodiscard]] inline float getLifetime() const noexcept { return m_Lifetime; }
        [[nodiscard]] inline float getTimer() const noexcept { return m_Timer; }
        inline void incrementTimer(const float amount) noexcept { m_Timer += amount; }

        inline void activate() noexcept { m_Active = true; m_Timer = 0.0f; }
        inline void deactivate() noexcept { m_Active = false; m_Timer = 0.0f; }

        [[nodiscard]] inline ParticleEmissionPropertiesHandle getProperties() const noexcept { return m_Properties; }
        void setProperties(ParticleEmissionPropertiesHandle);

        void update(size_t index, const float dt, Engine::priv::ParticleSystem&, bool multi_threaded);
};

#endif