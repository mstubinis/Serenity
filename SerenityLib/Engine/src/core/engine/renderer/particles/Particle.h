#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_H
#define ENGINE_RENDERER_PARTICLE_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>

class ParticleEmitter;
class ParticleEmissionProperties;
class Scene;
class Material;
namespace Engine {
    namespace epriv {
        struct InternalScenePublicInterface;
        class  ParticleSystem;
        class  GBuffer;
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
    friend class  Engine::epriv::ParticleSystem;
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
        Material* getMaterial();
        Scene& scene() const;
        const float& angle() const;
        const glm::vec2& getScale() const;
        const glm::vec3& position() const;
        const glm::vec4& color() const;
        const glm::vec3& velocity() const;
        const double lifetime() const;
        void update(const size_t& index, const double& dt, Engine::epriv::ParticleSystem& particleSystem);
        void update_multithreaded(const size_t& index, const double& dt, Engine::epriv::ParticleSystem& particleSystem);

        void render(Engine::epriv::GBuffer&);

};

#endif