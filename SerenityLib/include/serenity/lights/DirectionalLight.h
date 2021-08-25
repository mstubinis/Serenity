#pragma once
#ifndef ENGINE_LIGHTS_DIRECITONAL_H
#define ENGINE_LIGHTS_DIRECITONAL_H

class DirectionalLight;

#include <serenity/lights/SunLight.h>
#include <serenity/scene/Camera.h>
#include <array>

constexpr const float   DIRECTIONAL_LIGHT_DEFAULT_SHADOW_MAP_SIZE   = 1024.0f;
constexpr const uint8_t DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS = 4;
constexpr const uint8_t DIRECTIONAL_LIGHT_NUM_FRUSTUM_CORNERS       = 8;

class DirectionalLight : public EntityBody, public LightBaseData<DirectionalLight> {
    friend class Engine::priv::RenderModule;
    public:
        DirectionalLight() = delete;
        DirectionalLight(Scene*, const glm::vec3& direction = glm::vec3{ 0.0f, 0.0f, -1.0f });
        DirectionalLight(Scene*, float directionX, float directionY, float directionZ);

        bool setShadowCaster(bool castsShadow) noexcept;

        void setDirection(const glm::vec3& direction);
        void setDirection(float directionX, float directionY, float directionZ);

        glm::vec3 getDirection() const noexcept;
};

class DirectionalLightShadowData {
    using CascadeContainerDistances = std::array<float, (size_t)DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS + 1>;
    using CascadeContainerMatrices  = std::array<glm::mat4, DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS>;
    public:
        CascadeContainerMatrices   m_LightOrthoProjection;
        glm::mat4                  m_LightViewMatrix;
        CascadeContainerDistances  m_CascadeDistances;
        uint32_t                   m_ShadowWidth           = static_cast<uint32_t>(DIRECTIONAL_LIGHT_DEFAULT_SHADOW_MAP_SIZE);
        uint32_t                   m_ShadowHeight          = static_cast<uint32_t>(DIRECTIONAL_LIGHT_DEFAULT_SHADOW_MAP_SIZE);
        bool                       m_Enabled               = true;

        inline void setOrtho(float orthoSize, float Near, float Far) noexcept {
            m_LightOrthoProjection[0] = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, Near, Far);
        }
        inline void setLookAt(const glm::vec3& direction) noexcept {
            const auto dir = glm::normalize(direction) * 1.0f;
            m_LightViewMatrix = glm::lookAt(dir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        inline void setLookAt(const glm::vec3& direction, const glm::vec3& center) noexcept {
            const auto dir = glm::normalize(direction) * 1.0f;
            m_LightViewMatrix = glm::lookAt(center + dir, center, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        inline void setLookAt(const glm::vec3& direction, const glm::vec3& center, const glm::vec3& up) noexcept {
            const auto dir = glm::normalize(direction) * 1.0f;
            m_LightViewMatrix = glm::lookAt(center + dir, center, up);
        }
        inline void update(DirectionalLight& light, Camera& camera) noexcept {
            setLookAt(light.getComponent<ComponentTransform>()->getForward(), glm::vec3{ camera.getPosition() });
        }
};

#endif