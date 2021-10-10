#pragma once
#ifndef ENGINE_RENDERER_DECAL_H
#define ENGINE_RENDERER_DECAL_H

class Material;
namespace Engine::priv {
    struct DefaultDecalBindFunctor;
    struct DefaultDecalUnbindFunctor;
};

#include <serenity/ecs/entity/Entity.h>
#include <serenity/renderer/RendererIncludes.h>
#include <serenity/resources/Handle.h>

class Decal final: public Entity {
    friend struct Engine::priv::DefaultDecalBindFunctor;
    friend struct Engine::priv::DefaultDecalUnbindFunctor;
    private:
        float      m_LifetimeCurrent  = 0.0f;
        float      m_LifetimeMax      = 0.0f;
        bool       m_Active           = true;
        glm_vec3   m_InitialPosition  = glm_vec3(0.0, 0.0, 0.0);
        glm_quat   m_InitialRotation  = glm_quat(1.0, 0.0, 0.0, 0.0);
    public:
        Decal(
            Handle materialHandle,
            const glm_vec3& position,
            const glm::vec3& hitNormal,
            float size,
            Scene& scene,
            float lifetimeMax,
            RenderStage = RenderStage::Decals
        );
        ~Decal();

        Decal(const Decal& other) = delete;
        Decal& operator=(const Decal& other) = delete;
        Decal(Decal&& other) noexcept;
        Decal& operator=(Decal&& other) noexcept;

        void update(const float dt);

        inline constexpr bool active() const noexcept { return m_Active; }
        inline constexpr const glm_vec3& initialPosition() const noexcept { return m_InitialPosition; }
        inline constexpr const glm_quat& initialRotation() const noexcept { return m_InitialRotation; }
};
#endif