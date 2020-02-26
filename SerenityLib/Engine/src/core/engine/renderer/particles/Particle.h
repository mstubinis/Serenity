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
};

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>

struct ParticleData final {
    ParticleEmissionProperties*  m_Properties      = nullptr;
    glm::vec4                    m_UserData        = glm::vec4(0.0f);
    glm::vec2                    m_Scale           = glm::vec2(1.0f);
    glm::vec3                    m_Velocity        = glm::vec3(0.0f);
    float                        m_Depth           = 0.0f;
    Engine::color_vector_4       m_Color           = Engine::color_vector_4(255_uc);
    float                        m_Angle           = 0.0f;
    float                        m_AngularVelocity = 0.0f;
    float                        m_Timer           = 0.0f;
    bool                         m_Active          = true;

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
        ParticleEmitter*   m_EmitterSource     = nullptr;
        Material*          m_Material          = nullptr;
        Scene*             m_Scene             = nullptr;

        bool               m_Hidden            = false;
        bool               m_PassedRenderCheck = false;
        glm::vec3          m_Position          = glm::vec3(0.0f);
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

        const bool isActive() const;
        void setPosition(const glm::vec3& newPosition);
        Material* getMaterial() const;
        Scene& scene() const;
        const float angle() const;
        const glm::vec2& getScale() const;
        const glm::vec3& position() const;
        const Engine::color_vector_4& color() const;
        const glm::vec3& velocity() const;
        const double lifetime() const;
        void update(const size_t index, const float dt, Engine::priv::ParticleSystem& particleSystem, const glm::vec3& cameraPosition, const bool multi_threaded);


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
};

#endif