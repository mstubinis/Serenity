#pragma once
#ifndef ENGINE_LIGHTS_PROJECTION_H
#define ENGINE_LIGHTS_PROJECTION_H

class  Texture;
namespace Engine::priv {
    class IRenderingPipeline;
}

#include <core/engine/lights/SunLight.h>
#include <core/engine/resources/Handle.h>

class ProjectionLight : public SunLight {
    friend class Engine::priv::Renderer;
    friend class Engine::priv::IRenderingPipeline;
    protected:
        Texture*   m_Texture          = nullptr;
        float      m_FOV              = 60.0f;
        float      m_Near             = 0.01f;
        float      m_Far              = 1000.0f;
        float      m_AspectRatio      = 1.0f;
        glm::mat4  m_ViewMatrix       = glm::mat4(1.0f);
        glm::mat4  m_ProjectionMatrix = glm::mat4(1.0f);

        void internal_recalculate_projection_matrix() noexcept;

    public:
        ProjectionLight(
            Texture* texture = nullptr,
            const glm::vec3& direction = glm::vec3(0.0f, 0.0f, -1.0f),
            Scene* = nullptr
        );
        ProjectionLight(
            Handle textureHandle,
            const glm::vec3& direction = glm::vec3(0.0f, 0.0f, -1.0f),
            Scene* = nullptr
        );
        virtual ~ProjectionLight() {}

        void setTexture(Texture* texture) noexcept { m_Texture = texture; }
        void setTexture(Handle textureHandle) noexcept;

        void recalculateViewMatrix() noexcept;

        void setFOV(float fov) noexcept { m_FOV = fov; internal_recalculate_projection_matrix(); }
        void setViewMatrix(glm::mat4& viewMatrix) noexcept { m_ViewMatrix = viewMatrix; }
        void setProjectionMatrix(glm::mat4& projectionMatrix) noexcept { m_ProjectionMatrix = projectionMatrix; }
        void setAspectRatio(float aspectRatio) noexcept { m_AspectRatio = aspectRatio; internal_recalculate_projection_matrix(); }
        void setNear(float inNear) noexcept { m_Near = inNear; internal_recalculate_projection_matrix(); }
        void setFar(float inFar) noexcept { m_Far = inFar; internal_recalculate_projection_matrix(); }

        Texture* getTexture() const noexcept { return m_Texture; }
        constexpr float getFOV() const noexcept { return m_FOV; }
        constexpr const glm::mat4& getViewMatrix() const noexcept { return m_ViewMatrix; }
        constexpr const glm::mat4& getProjectionMatrix() const noexcept { return m_ProjectionMatrix; }
        constexpr float getAspectRatio() const noexcept { return m_AspectRatio; }
        constexpr float getNear() const noexcept { return m_Near; }
        constexpr float getFar() const noexcept { return m_Far; }
        void free() noexcept override;
};

#endif