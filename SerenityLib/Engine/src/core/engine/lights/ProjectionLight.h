#pragma once
#ifndef ENGINE_LIGHTS_PROJECTION_H
#define ENGINE_LIGHTS_PROJECTION_H

class  Texture;
namespace Engine::priv {
    class IRenderingPipeline;
}

#include <core/engine/lights/SunLight.h>
#include <core/engine/resources/Handle.h>

constexpr float PROJECTION_LIGHT_DEFAULT_NEAR = 0.01f;
constexpr float PROJECTION_LIGHT_DEFAULT_FAR  = 1000.0f;

class ProjectionLight : public SunLight {
    friend class Engine::priv::RenderModule;
    friend class Engine::priv::IRenderingPipeline;
    protected:
        Handle                         m_Texture    = Handle{};

        std::array<glm::vec3, 8>       m_FrustumPoints;
        std::array<unsigned int, 36>   m_FrustumIndices;

        float                          m_Near       = PROJECTION_LIGHT_DEFAULT_NEAR;
        float                          m_Far        = PROJECTION_LIGHT_DEFAULT_FAR;
        glm::vec2                      m_NearScale  = glm::vec2(1.0f);
        glm::vec2                      m_FarScale   = glm::vec2(1.0f);

        void recalc_frustum_points() noexcept;
        void recalc_frustum_indices() noexcept;

    public:
        ProjectionLight() = delete;
        ProjectionLight(
            Scene*,
            Handle textureHandle,
            const glm::vec3& direction = glm::vec3(0.0f, 0.0f, -1.0f)
        );
        virtual ~ProjectionLight();

        inline void setTexture(Handle textureHandle) noexcept { m_Texture = textureHandle; }

        void setNear(float inNear) noexcept { m_Near = inNear; recalc_frustum_points(); }
        void setFar(float inFar) noexcept { m_Far = inFar; recalc_frustum_points(); }
        void setNearScale(float nearScaleX, float nearScaleY) noexcept { m_NearScale.x = nearScaleX; m_NearScale.y = nearScaleY; recalc_frustum_points(); }
        void setFarScale(float farScaleX, float farScaleY) noexcept { m_FarScale.x = farScaleX; m_FarScale.y = farScaleY; recalc_frustum_points(); }

        inline Handle getTexture() const noexcept { return m_Texture; }

        inline CONSTEXPR const std::array<glm::vec3, 8>& getPoints() const noexcept { return m_FrustumPoints; }
        inline CONSTEXPR const std::array<unsigned int, 36>& getIndices() const noexcept { return m_FrustumIndices; }

        inline CONSTEXPR float getNear() const noexcept { return m_Near; }
        inline CONSTEXPR float getFar() const noexcept { return m_Far; }
        inline CONSTEXPR glm::vec2 getNearScale() const noexcept { return m_NearScale; }
        inline CONSTEXPR glm::vec2 getFarScale() const noexcept { return m_FarScale; }

};

#endif