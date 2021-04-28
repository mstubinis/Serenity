#pragma once
#ifndef ENGINE_LIGHTS_DIRECITONAL_H
#define ENGINE_LIGHTS_DIRECITONAL_H

class DirectionalLight;

#include <serenity/lights/SunLight.h>

class DirectionalLightShadowData {
    public:
        glm::mat4 m_OrthoProjection;
        glm::mat4 m_ViewMatrix;
        bool m_Enabled = true;

        inline void setOrtho(float orthoSize, float Near, float Far) noexcept {
            m_OrthoProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, Near, Far);
        }
        inline void setLookAt(const glm::vec3& direction) noexcept {
            m_ViewMatrix = glm::lookAt(glm::normalize(direction) * 100.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
};

class DirectionalLight : public SunLight {
    friend class Engine::priv::RenderModule;
    public:
        DirectionalLight() = delete;
        DirectionalLight(Scene*, const glm::vec3& direction = glm::vec3{ 0.0f, 0.0f, -1.0f });
        DirectionalLight(Scene*, float directionX, float directionY, float directionZ);
        virtual ~DirectionalLight();

        bool setShadowCaster(bool castsShadow) noexcept override;

        void setDirection(const glm::vec3& direction);
        void setDirection(float directionX, float directionY, float directionZ);
};

#endif