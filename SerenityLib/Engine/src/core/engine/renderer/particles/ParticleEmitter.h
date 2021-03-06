#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_EMITTER_H
#define ENGINE_RENDERER_PARTICLE_EMITTER_H

class  Particle;
class  ParticleEmissionProperties;
namespace Engine::priv {
    struct InternalScenePublicInterface;
    class  ParticleSystem;
};

#include <ecs/EntityBody.h>

/*
Class that spawns particles
*/
class ParticleEmitter final : public EntityBody {
    friend class  Engine::priv::ParticleSystem;
    friend class  Particle;
    friend struct Engine::priv::InternalScenePublicInterface;

    using update_func = std::function<void(ParticleEmitter*, const float dt, std::shared_mutex&)>;

    public:
        glm::vec4                      m_UserData       = glm::vec4(0.0f);
    private:
        ParticleEmissionProperties*    m_Properties     = nullptr;
        bool                           m_Active         = false;
        float                          m_SpawningTimer  = 0.0f;
        float                          m_Timer          = 0.0f;
        float                          m_Lifetime       = 0.0f;
        Entity                         m_Parent         = Entity();
        update_func                    m_UpdateFunctor  = [](ParticleEmitter*, const float, std::shared_mutex&) {};

        ParticleEmitter() = delete;
    public:
        ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent = Entity());
        ~ParticleEmitter() = default;

        ParticleEmitter(const ParticleEmitter& other)                = delete;
        ParticleEmitter& operator=(const ParticleEmitter& other)     = delete;
        ParticleEmitter(ParticleEmitter&& other) noexcept;
        ParticleEmitter& operator=(ParticleEmitter&& other) noexcept;

        void init(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent);

        inline void setUpdateFunctor(update_func&& functor) noexcept { m_UpdateFunctor = std::move(functor); }

        void applyLinearVelocity(decimal x, decimal y, decimal z, bool local = true);
        void applyLinearVelocity(glm_vec3& velocity, bool local = true);

        inline CONSTEXPR bool isActive() const noexcept { return m_Active; }

        inline void activate() noexcept { m_Active = true; }
        inline void deactivate() noexcept { m_Active = false; }

        inline CONSTEXPR ParticleEmissionProperties* getProperties() const noexcept { return m_Properties; }
        void setProperties(ParticleEmissionProperties& properties);

        void update(size_t index, const float dt, Engine::priv::ParticleSystem& particleSystem, bool multi_threaded);
};

#endif