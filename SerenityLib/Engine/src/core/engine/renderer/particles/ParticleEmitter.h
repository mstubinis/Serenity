#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_EMITTER_H
#define ENGINE_RENDERER_PARTICLE_EMITTER_H

class  ParticleEmissionProperties;
namespace Engine::priv {
    struct InternalScenePublicInterface;
    class  ParticleSystem;
};

#include <core/engine/renderer/particles/Particle.h>
#include <ecs/EntityWrapper.h>
#include <vector>
#include <mutex>
#include <core/engine/math/Numbers.h>

class ParticleEmitter final : public EntityWrapper{
    friend class  Engine::priv::ParticleSystem;
    friend class  Particle;
    friend struct ParticleData;
    friend struct Engine::priv::InternalScenePublicInterface;
    public:
        glm::vec4                      m_UserData;
    private:
        ParticleEmissionProperties*    m_Properties;
        bool                           m_Active;
        double                         m_SpawningTimer;
        double                         m_Timer;
        double                         m_Lifetime;
        Entity                         m_Parent;
        std::function<void(const float& dt, ParticleEmissionProperties&, std::mutex&)> m_UpdateFunctor;

        void internal_init();
    public:
        ParticleEmitter();
        ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, const double lifetime, EntityWrapper* parent = nullptr);
        ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, const double lifetime, const Entity& parent = Entity::null_);
        ~ParticleEmitter();

        ParticleEmitter(const ParticleEmitter& other);
        ParticleEmitter& operator=(const ParticleEmitter& other);
        ParticleEmitter(ParticleEmitter&& other) noexcept;
        ParticleEmitter& operator=(ParticleEmitter&& other) noexcept;

        template<typename T> void setUpdateFunctor(const T& functor) {
            m_UpdateFunctor = std::bind<void>(std::move(functor), this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }

        void setPosition(const decimal& x, const decimal& y, const decimal& z, EntityDataRequest& request);
        void setPosition(const glm_vec3& position, EntityDataRequest& request);
        void setPosition(const decimal& x, const decimal& y, const decimal& z);
        void setPosition(const glm_vec3& position);


        void setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w, EntityDataRequest& request);
        void setRotation(const glm_quat& rotation, EntityDataRequest& request);
        void setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w);
        void setRotation(const glm_quat& rotation);

        void rotate(const decimal& x, const decimal& y, const decimal& z, EntityDataRequest& request);
        void rotate(const decimal& x, const decimal& y, const decimal& z);


        void setScale(const decimal& x, const decimal& y, const decimal& z, EntityDataRequest& request);
        void setScale(const glm_vec3& scale, EntityDataRequest& request);
        void setScale(const decimal& x, const decimal& y, const decimal& z);
        void setScale(const glm_vec3& scale);

        void setLinearVelocity(const decimal& x, const decimal& y, const decimal& z, EntityDataRequest& request, const bool local = true);
        void setLinearVelocity(const glm_vec3& velocity, EntityDataRequest& request, const bool local = true);
        void setLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void setLinearVelocity(const glm_vec3& velocity, const bool local = true);
 
        void applyLinearVelocity(const decimal& x, const decimal& y, const decimal& z, EntityDataRequest& request, const bool local = true);
        void applyLinearVelocity(glm_vec3& velocity, EntityDataRequest& request, const bool local = true);
        void applyLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void applyLinearVelocity(glm_vec3& velocity, const bool local = true);

        const glm_vec3 getScale(EntityDataRequest& request) const;
        const glm_vec3 getScale() const;

        const glm_vec3 position(EntityDataRequest& request) const;
        const glm_vec3 position() const;

        const glm_quat rotation(EntityDataRequest& request) const;
        const glm_quat rotation() const;

        const glm_vec3 linearVelocity(EntityDataRequest& request) const;
        const glm_vec3 linearVelocity() const;

        const bool& isActive() const;

        void activate();
        void deactivate();

        ParticleEmissionProperties* getProperties() const;
        void setProperties(ParticleEmissionProperties& properties);

        void update(const size_t& index, const float& dt, Engine::priv::ParticleSystem& particleSystem);
        void update_multithreaded(const size_t& index, const float& dt, Engine::priv::ParticleSystem& particleSystem);
};

#endif