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
        Texture*                       m_Texture    = nullptr;

        std::array<glm::vec3, 8>       m_FrustumPoints;
        std::array<unsigned int, 36>   m_FrustumIndices;

        float                          m_Near       = 0.01f;
        float                          m_Far        = 1000.0f;
        glm::vec2                      m_NearScale  = glm::vec2(1.0f);
        glm::vec2                      m_FarScale   = glm::vec2(1.0f);

        void recalc_frustum_points() noexcept;
        void recalc_frustum_indices() noexcept;
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

        void setNear(float inNear) noexcept { m_Near = inNear; recalc_frustum_points(); }
        void setFar(float inFar) noexcept { m_Far = inFar; recalc_frustum_points(); }
        void setNearScale(float nearScaleX, float nearScaleY) noexcept { m_NearScale.x = nearScaleX; m_NearScale.y = nearScaleY; recalc_frustum_points(); }
        void setFarScale(float farScaleX, float farScaleY) noexcept { m_FarScale.x = farScaleX; m_FarScale.y = farScaleY; recalc_frustum_points(); }

        Texture* getTexture() const noexcept { return m_Texture; }

        inline CONSTEXPR const std::array<glm::vec3, 8>& getPoints() const noexcept { return m_FrustumPoints; }
        inline CONSTEXPR const std::array<unsigned int, 36>& getIndices() const noexcept { return m_FrustumIndices; }

        inline CONSTEXPR float getNear() const noexcept { return m_Near; }
        inline CONSTEXPR float getFar() const noexcept { return m_Far; }
        inline CONSTEXPR glm::vec2 getNearScale() const noexcept { return m_NearScale; }
        inline CONSTEXPR glm::vec2 getFarScale() const noexcept { return m_FarScale; }

        void free() noexcept override;
};

#endif