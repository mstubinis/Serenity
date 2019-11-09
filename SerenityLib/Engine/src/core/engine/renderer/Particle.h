#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_H
#define ENGINE_RENDERER_PARTICLE_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <mutex>
#include <stack>

class ParticleEmitter;
class ParticleEmissionProperties;
class Scene;
class Material;
namespace Engine {
    namespace epriv {
        struct InternalScenePublicInterface;
    };
};
struct ParticleData final {
    ParticleEmissionProperties*  m_Properties;
    glm::vec2                    m_Scale;
    glm::vec3                    m_Velocity;
    glm::vec4                    m_Color;
    float                        m_Depth;
    float                        m_Angle;
    float                        m_AngularVelocity;
    double                       m_Timer;
    bool                         m_Active;

    ParticleData();
    ParticleData(ParticleEmissionProperties& properties);

    ParticleData(const ParticleData& other);
    ParticleData& operator=(const ParticleData& other);
    ParticleData(ParticleData&& other) noexcept;
    ParticleData& operator=(ParticleData&& other) noexcept;
};

class Particle {
    friend struct Engine::epriv::InternalScenePublicInterface;
    private:
        ParticleData   m_Data;
        Material*      m_Material;
        Scene*         m_Scene;
        bool           m_Hidden;
        bool           m_PassedRenderCheck;
        glm::vec3      m_Position;
    public:
        Particle();
        Particle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmissionProperties& properties, Scene& scene);
        ~Particle();

        Particle(const Particle& other);
        Particle& operator=(const Particle& other);
        Particle(Particle&& other) noexcept;
        Particle& operator=(Particle&& other) noexcept;

        void init(ParticleData& data, const glm::vec3& emitterPosition, const glm::quat& emitterRotation);

        const bool& isActive() const;
        void setPosition(const glm::vec3& newPosition);
        const glm::vec3& position() const;
        const glm::vec4& color() const;
        const glm::vec3& velocity() const;
        const double lifetime() const;
        void update(const size_t& index, std::stack<unsigned int>& freelist, const double& dt);
        void update_multithreaded(const size_t& index, std::stack<unsigned int>& freelist, const double& dt, std::mutex& mutex_);

        void render();

};

#endif