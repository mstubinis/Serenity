#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_H
#define ENGINE_RENDERER_PARTICLE_H

class  Particle;
class  ParticleEmitter;
class  ParticleEmissionProperties;
class  Scene;
class  Material;
struct Entity;
namespace Engine::priv {
    struct InternalScenePublicInterface;
    class  ParticleSystem;
    class  GBuffer;
};

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>

struct ParticleData final {
    ParticleEmissionProperties*  m_Properties;
    glm::vec4                    m_UserData;
    glm::vec2                    m_Scale;
    glm::vec3                    m_Velocity;
    glm::vec4                    m_Color;
    float                        m_Depth;
    float                        m_Angle;
    float                        m_AngularVelocity;
    double                       m_Timer;
    bool                         m_Active;

    ParticleData();
    ParticleData(ParticleEmissionProperties& properties, ParticleEmitter& emitter, Particle& particle);

    ParticleData(const ParticleData& other) = delete;
    ParticleData& operator=(const ParticleData& other) = delete;
    ParticleData(ParticleData&& other) noexcept;
    ParticleData& operator=(ParticleData&& other) noexcept;
};

class Particle {
    friend struct Engine::priv::InternalScenePublicInterface;
    friend class  Engine::priv::ParticleSystem;
    friend struct ParticleData;
    private:
        ParticleEmitter*   m_EmitterSource;
        Material*          m_Material;
        Scene*             m_Scene;
        bool               m_Hidden;
        bool               m_PassedRenderCheck;
        glm::vec3          m_Position;
    public:
        ParticleData       m_Data;

        Particle();
        Particle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmitter& emitter);
        ~Particle();

        Particle(const Particle& other) = delete;
        Particle& operator=(const Particle& other) = delete;
        Particle(Particle&& other) noexcept;
        Particle& operator=(Particle&& other) noexcept;

        void init(ParticleData&& data, const glm::vec3& emitterPosition, const glm::quat& emitterRotation, Entity& parent);

        const bool& isActive() const;
        void setPosition(const glm::vec3& newPosition);
        Material* getMaterial() const;
        Scene& scene() const;
        const float& angle() const;
        const glm::vec2& getScale() const;
        const glm::vec3& position() const;
        const glm::vec4& color() const;
        const glm::vec3& velocity() const;
        const double lifetime() const;
        void update(const size_t& index, const float& dt, Engine::priv::ParticleSystem& particleSystem);
        void update_multithreaded(const size_t& index, const float& dt, Engine::priv::ParticleSystem& particleSystem);



        void setUserDataX(const float x);
        void setUserDataY(const float y);
        void setUserDataZ(const float z);
        void setUserDataW(const float w);
        void setUserData(const float x, const float y, const float z, const float w);
        void setUserData(const glm::vec4& data);

        const float getUserDataX() const;
        const float getUserDataY() const;
        const float getUserDataZ() const;
        const float getUserDataW() const;
        const glm::vec4& getUserData() const;


        void render(const Engine::priv::GBuffer&);

};

#endif