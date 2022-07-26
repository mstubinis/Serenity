#pragma once
#ifndef ENGINE_RENDERER_PARTICLE_H
#define ENGINE_RENDERER_PARTICLE_H

class  ParticleEmitter;
class  Material;
namespace Engine::priv {
    struct PublicScene;
    class  ParticleSystem;
};

#include <serenity/dependencies/glm.h>
#include <serenity/types/ColorVector.h>

/* 2D Sprite that is optimized to render fast and in very large numbers */
class Particle final {
    friend struct Engine::priv::PublicScene;
    friend class  Engine::priv::ParticleSystem;
    public:
        glm::vec3                 m_Position        = glm::vec3{ 0.0f }; //96 bits  (pass to gpu 3 floats)    
        glm::vec3                 m_Velocity        = glm::vec3{ 0.0f }; //96 bits

        glm::vec2                 m_Scale           = glm::vec2{ 1.0f }; //64 bits (pass to gpu 2 floats)
        ParticleEmitter*          m_EmitterSource   = nullptr;         //64 bits
        Material*                 m_Material        = nullptr;         //64 bits (pass to gpu as index (uint))

        Engine::color_vector_4    m_Color           = Engine::color_vector_4(255_uc); //32 bits (pass to gpu (either as 2 floats or 1 uint))

        float                     m_Angle           = 0.0f;            //  32 bits (pass to gpu as 1 float)
        float                     m_AngularVelocity = 0.0f;            //  32 bits
        float                     m_Timer           = 0.0f;            //  32 bits

        Particle() = default;
        Particle(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmitter&);

        Particle(const Particle&)                = delete;
        Particle& operator=(const Particle&)     = delete;
        Particle(Particle&&) noexcept;
        Particle& operator=(Particle&&) noexcept;

        void init(const glm::vec3& emitterPosition, const glm::quat& emitterRotation, ParticleEmitter& parent) noexcept;

        [[nodiscard]] inline bool isActive() const noexcept { return (m_Timer > 0.0f); }
        [[nodiscard]] inline Material* getMaterial() const noexcept { return m_Material; }
        [[nodiscard]] inline float angle() const noexcept { return m_Angle; }
        [[nodiscard]] inline const glm::vec2& getScale() const noexcept { return m_Scale; }
        [[nodiscard]] inline const glm::vec3& position() const noexcept { return m_Position; }
        [[nodiscard]] inline const Engine::color_vector_4& color() const noexcept { return m_Color; }
        [[nodiscard]] inline const glm::vec3& velocity() const noexcept { return m_Velocity; }
        [[nodiscard]] float lifetime() const noexcept;

        void setScale(const glm::vec2& newScale) noexcept;
        void setScale(float x, float y) noexcept;

        void setPosition(const glm::vec3& newPosition) noexcept;
        void setPosition(float x, float y, float z) noexcept;

        void translate(const glm::vec3& newPosition) noexcept;
        void translate(float x, float y, float z) noexcept;
};
#endif