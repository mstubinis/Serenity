#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_EMITTER_H
#define ENGINE_RENDERER_PARTICLE_EMITTER_H

class  Particle;
class  ParticleEmissionProperties;
namespace Engine::priv {
    struct InternalScenePublicInterface;
    class  ParticleSystem;
};

#include <ecs/Entity.h>

/*
Class that spawns particles
*/
class ParticleEmitter final : public Entity {
    friend class  Engine::priv::ParticleSystem;
    friend class  Particle;
    friend struct Engine::priv::InternalScenePublicInterface;

    using update_func = std::function<void(ParticleEmitter*, const float dt, ParticleEmissionProperties&, std::mutex&)>;

    public:
        glm::vec4                      m_UserData       = glm::vec4(0.0f);
    private:
        ParticleEmissionProperties*    m_Properties     = nullptr;
        bool                           m_Active         = false;
        float                          m_SpawningTimer  = 0.0f;
        float                          m_Timer          = 0.0f;
        float                          m_Lifetime       = 0.0f;
        Entity                         m_Parent         = Entity();
        update_func                    m_UpdateFunctor  = [](ParticleEmitter*, const float, ParticleEmissionProperties&, std::mutex&) {};

        ParticleEmitter() = delete;
    public:
        ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent = Entity());
        ~ParticleEmitter() {}

        ParticleEmitter(const ParticleEmitter& other) = delete;
        ParticleEmitter& operator=(const ParticleEmitter& other) = delete;
        ParticleEmitter(ParticleEmitter&& other) noexcept;
        ParticleEmitter& operator=(ParticleEmitter&& other) noexcept;

        void init(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent);

        void setUpdateFunctor(update_func functor) noexcept { m_UpdateFunctor = functor; }

        void setPosition(decimal x, decimal y, decimal z);
        void setPosition(const glm_vec3& position);


        void setRotation(decimal x, decimal y, decimal z, decimal w);
        void setRotation(const glm_quat& rotation);

        void rotate(decimal x, decimal y, decimal z);

        void setScale(decimal x, decimal y, decimal z);
        void setScale(const glm_vec3& scale);

        void setLinearVelocity(decimal x, decimal y, decimal z, bool local = true);
        void setLinearVelocity(const glm_vec3& velocity, bool local = true);
 
        void applyLinearVelocity(decimal x, decimal y, decimal z, bool local = true);
        void applyLinearVelocity(glm_vec3& velocity, bool local = true);

        glm_vec3 getScale() const;

        glm_vec3 position() const;

        glm_quat rotation() const;

        glm_vec3 linearVelocity() const;

        inline CONSTEXPR bool isActive() const noexcept { return m_Active; }

        inline void activate() noexcept { m_Active = true; }
        inline void deactivate() noexcept { m_Active = false; }

        inline CONSTEXPR ParticleEmissionProperties* getProperties() const noexcept { return m_Properties; }
        void setProperties(ParticleEmissionProperties& properties);

        void update(size_t index, const float dt, Engine::priv::ParticleSystem& particleSystem, bool multi_threaded);
};

#endif