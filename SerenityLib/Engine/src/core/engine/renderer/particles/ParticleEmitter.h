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
#include <mutex>
#include <functional>

/*
Class that spawns particles
*/
class ParticleEmitter final : public Entity {
    friend class  Engine::priv::ParticleSystem;
    friend class  Particle;
    friend struct Engine::priv::InternalScenePublicInterface;
    public:
        glm::vec4                      m_UserData       = glm::vec4(0.0f);
    private:
        ParticleEmissionProperties*    m_Properties     = nullptr;
        bool                           m_Active         = false;
        float                          m_SpawningTimer  = 0.0f;
        float                          m_Timer          = 0.0f;
        float                          m_Lifetime       = 0.0f;
        Entity                         m_Parent         = Entity();
        std::function<void(ParticleEmitter*, const float dt, ParticleEmissionProperties&, std::mutex&)> m_UpdateFunctor = [](ParticleEmitter*, const float, ParticleEmissionProperties&, std::mutex&) {};

        ParticleEmitter() = delete;
    public:
        ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent = Entity());
        ~ParticleEmitter();

        ParticleEmitter(const ParticleEmitter& other) = delete;
        ParticleEmitter& operator=(const ParticleEmitter& other) = delete;
        ParticleEmitter(ParticleEmitter&& other) noexcept;
        ParticleEmitter& operator=(ParticleEmitter&& other) noexcept;

        void init(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent);

        template<typename T> void setUpdateFunctor(const T& functor) {
            m_UpdateFunctor = std::bind<void>(std::move(functor), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        }

        void setPosition(const decimal& x, const decimal& y, const decimal& z);
        void setPosition(const glm_vec3& position);


        void setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w);
        void setRotation(const glm_quat& rotation);

        void rotate(const decimal& x, const decimal& y, const decimal& z);

        void setScale(const decimal& x, const decimal& y, const decimal& z);
        void setScale(const glm_vec3& scale);

        void setLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void setLinearVelocity(const glm_vec3& velocity, const bool local = true);
 
        void applyLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void applyLinearVelocity(glm_vec3& velocity, const bool local = true);

        glm_vec3 getScale() const;

        glm_vec3 position() const;

        glm_quat rotation() const;

        glm_vec3 linearVelocity() const;

        bool isActive() const;

        void activate();
        void deactivate();

        ParticleEmissionProperties* getProperties() const;
        void setProperties(ParticleEmissionProperties& properties);

        void update(size_t index, const float dt, Engine::priv::ParticleSystem& particleSystem, bool multi_threaded);
};

#endif