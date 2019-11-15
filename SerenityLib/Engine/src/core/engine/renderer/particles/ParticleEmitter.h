#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_EMITTER_H
#define ENGINE_RENDERER_PARTICLE_EMITTER_H

#include <core/engine/renderer/particles/Particle.h>
#include <ecs/EntityWrapper.h>
#include <vector>
#include <mutex>
#include <core/engine/math/Numbers.h>

class  ParticleEmissionProperties;
namespace Engine {
    namespace epriv {
        struct InternalScenePublicInterface;
        class  ParticleSystem;
    };
};
class ParticleEmitter final : public EntityWrapper{
    friend class  Engine::epriv::ParticleSystem;
    friend class  Particle;
    friend struct ParticleData;
    friend struct Engine::epriv::InternalScenePublicInterface;
    private:
        ParticleEmissionProperties*    m_Properties;
        bool                           m_Active;
        double                         m_SpawningTimer;
        double                         m_Timer;
        double                         m_Lifetime;
        Entity                         m_Parent;
        void internal_init();
    public:
        ParticleEmitter();
        ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, const double lifetime, EntityWrapper* parent = nullptr);
        ParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, const double lifetime, Entity& parent = Entity::_null);
        ~ParticleEmitter();

        ParticleEmitter(const ParticleEmitter& other);
        ParticleEmitter& operator=(const ParticleEmitter& other);
        ParticleEmitter(ParticleEmitter&& other) noexcept;
        ParticleEmitter& operator=(ParticleEmitter&& other) noexcept;


        void setPosition(const decimal& x, const decimal& y, const decimal& z, EntityDataRequest& request);
        void setPosition(const glm_vec3& position, EntityDataRequest& request);
        void setPosition(const decimal& x, const decimal& y, const decimal& z);
        void setPosition(const glm_vec3& position);


        void setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w, EntityDataRequest& request);
        void setRotation(const glm_quat& rotation, EntityDataRequest& request);
        void setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w);
        void setRotation(const glm_quat& rotation);


        void setScale(const decimal& x, const decimal& y, const decimal& z, EntityDataRequest& request);
        void setScale(const glm_vec3& scale, EntityDataRequest& request);
        void setScale(const decimal& x, const decimal& y, const decimal& z);
        void setScale(const glm_vec3& scale);

        const glm_vec3 getScale(EntityDataRequest& request);
        const glm_vec3 getScale();

        const glm_vec3 position(EntityDataRequest& request);
        const glm_vec3 position();

        const glm_quat rotation(EntityDataRequest& request);
        const glm_quat rotation();

        const bool& isActive() const;

        void activate();
        void deactivate();

        void setProperties(ParticleEmissionProperties& properties);

        void update(const size_t& index, const double& dt, Engine::epriv::ParticleSystem& particleSystem);
        void update_multithreaded(const size_t& index, const double& dt, Engine::epriv::ParticleSystem& particleSystem);
};

#endif