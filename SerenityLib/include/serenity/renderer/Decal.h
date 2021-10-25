#pragma once
#ifndef ENGINE_RENDERER_DECAL_H
#define ENGINE_RENDERER_DECAL_H

class Material;

#include <serenity/ecs/entity/Entity.h>
#include <serenity/renderer/RendererIncludes.h>
#include <serenity/resources/Handle.h>

class Decal final: public Entity {
    private:
        glm_quat   m_InitialRotation  = glm_quat{ 1.0, 0.0, 0.0, 0.0 };
        glm_vec3   m_InitialPosition  = glm_vec3{ 0.0 };
        float      m_LifetimeCurrent  = 0.0f;
        float      m_LifetimeMax      = 0.0f;
        bool       m_Active           = true;
    public:
        Decal(
            Handle materialHandle,
            const glm_vec3& position,
            const glm::vec3& hitNormal,
            float size,
            Scene&,
            float lifetimeMax,
            RenderStage = RenderStage::Decals
        );
        ~Decal();

        Decal(const Decal&)                  = delete;
        Decal& operator=(const Decal&)       = delete;
        Decal(Decal&&) noexcept;
        Decal& operator=(Decal&&) noexcept;

        void update(const float dt);

        [[nodiscard]] inline constexpr bool active() const noexcept { return m_Active; }
        [[nodiscard]] inline constexpr const glm_vec3& initialPosition() const noexcept { return m_InitialPosition; }
        [[nodiscard]] inline constexpr const glm_quat& initialRotation() const noexcept { return m_InitialRotation; }
};
#endif