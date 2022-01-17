#pragma once
#ifndef ENGINE_LIGHTS_DIRECITONAL_H
#define ENGINE_LIGHTS_DIRECITONAL_H

class DirectionalLight;

#include <serenity/lights/SunLight.h>
#include <serenity/scene/Camera.h>
#include <array>

constexpr const uint32_t  DIRECTIONAL_LIGHT_DEFAULT_SHADOW_MAP_SIZE   = 2048;
constexpr const uint8_t   DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS = 5;
constexpr const uint8_t   DIRECTIONAL_LIGHT_NUM_CASCADING_DISTANCES   = DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS + 1;
constexpr const uint8_t   DIRECTIONAL_LIGHT_NUM_FRUSTUM_CORNERS       = 8;

class DirectionalLight : public Entity, public LightBaseData<DirectionalLight> {
    friend class Engine::priv::RenderModule;
    public:
        DirectionalLight() = delete;
        DirectionalLight(Scene*, const glm::vec3& direction = glm::vec3{ 0.0f, 0.0f, -1.0f });
        DirectionalLight(Scene*, float directionX, float directionY, float directionZ);

        //sets wether or not the light should produce shadows. If so, the additional parameters will be used. LightShadowFrustumType will multiply camera near and far by the two factors
        //if LightShadowFrustumType == LightShadowFrustumType::CameraBased, otherwise the factors will be used directly as near and far if LightShadowFrustumType::Fixed is used.
        bool setShadowCaster(
            bool castsShadow,
            uint32_t shadowMapWidth = DIRECTIONAL_LIGHT_DEFAULT_SHADOW_MAP_SIZE,
            uint32_t shadowMapSize = DIRECTIONAL_LIGHT_DEFAULT_SHADOW_MAP_SIZE,
            LightShadowFrustumType = LightShadowFrustumType::CameraBased,
            float nearFactor = 1.0f,
            float farFactor = 1.0f
        ) noexcept;

        void setDirection(const glm::vec3& direction);
        void setDirection(float directionX, float directionY, float directionZ);

        glm::vec3 getDirection() const noexcept;
};

class DirectionalLightShadowData {
    public:
        using CascadeContainerDistances = std::array<float, size_t(DIRECTIONAL_LIGHT_NUM_CASCADING_DISTANCES)>;
        using CascadeContainerMatrices  = std::array<glm::mat4, DIRECTIONAL_LIGHT_NUM_CASCADING_SHADOW_MAPS>;
    public:
        CascadeContainerMatrices   m_LightSpaceMatrices;
        CascadeContainerDistances  m_CascadeDistances;
        uint32_t                   m_ShadowWidth           = 0;
        uint32_t                   m_ShadowHeight          = 0;
        float                      m_NearFactor            = 1.0f;
        float                      m_FarFactor             = 1.0f;
        float                      m_NearCache             = 1.0f;
        float                      m_FarCache              = 1.0f;
        LightShadowFrustumType     m_FrustumType           = LightShadowFrustumType::CameraBased;
        bool                       m_Enabled               = true;
};

#endif