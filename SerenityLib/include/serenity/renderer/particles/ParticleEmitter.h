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
#include <mutex>

/* Class that spawns particles */
class ParticleEmitter final : public Entity {
    friend class  Engine::priv::ParticleSystem;
    friend class  Particle;
    friend struct Engine::priv::PublicScene;

    using update_func = std::function<void(ParticleEmitter*, const float dt, std::mutex&)>;

    public:
        glm::vec4                      m_UserData       = glm::vec4{ 0.0f };
    private:
        update_func                    m_UpdateFunctor  = [](ParticleEmitter*, const float, std::mutex&) {};
        ParticleEmissionProperties*    m_Properties     = nullptr;
        float                          m_SpawningTimer  = 0.0f;
        float                          m_Timer          = 0.0f;
        float                          m_Lifetime       = 0.0f;
        Entity                         m_Parent;
        bool                           m_Active         = false;

        ParticleEmitter() = delete;
    public:
        ParticleEmitter(ParticleEmissionProperties&, Scene&, float lifetime, Entity parent = {});
        ~ParticleEmitter() = default;

        ParticleEmitter(const ParticleEmitter&)                = delete;
        ParticleEmitter& operator=(const ParticleEmitter&)     = delete;
        ParticleEmitter(ParticleEmitter&&) noexcept;
        ParticleEmitter& operator=(ParticleEmitter&&) noexcept;

        void init(ParticleEmissionProperties&, Scene&, float lifetime, Entity parent);

        inline void setUpdateFunctor(update_func&& functor) noexcept { m_UpdateFunctor = std::move(functor); }

        //void applyLinearVelocity(decimal x, decimal y, decimal z, bool local = true);
        //void applyLinearVelocity(glm_vec3& velocity, bool local = true);

        [[nodiscard]] inline bool isActive() const noexcept { return m_Active; }

        inline void activate() noexcept { m_Active = true; }
        inline void deactivate() noexcept { m_Active = false; }

        [[nodiscard]] inline ParticleEmissionProperties* getProperties() const noexcept { return m_Properties; }
        void setProperties(ParticleEmissionProperties&);

        void update(size_t index, const float dt, Engine::priv::ParticleSystem&, bool multi_threaded);
};

#endif