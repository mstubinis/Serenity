#pragma once
#ifndef ENGINE_LIGHT_SUN_H
#define ENGINE_LIGHT_SUN_H

class Scene;
class SunLight;
namespace Engine::priv {
    class RenderModule;
};

#include <serenity/lights/LightIncludes.h>
#include <serenity/dependencies/glm.h>
#include <serenity/ecs/entity/Entity.h>

constexpr const uint32_t  SUN_LIGHT_DEFAULT_SHADOW_MAP_SIZE = 2048;

class SunLight : public Entity, public LightBaseData<SunLight> {
    friend class ::Engine::priv::RenderModule;
    public:
        SunLight() = delete;
        SunLight(Scene*, const glm_vec3& position);
        SunLight(Scene*, decimal x, decimal y, decimal z);

        //sets wether or not the light should produce shadows. If so, the additional parameters will be used. LightShadowFrustumType will multiply camera near and far by the two factors
        //if LightShadowFrustumType == LightShadowFrustumType::CameraBased, otherwise the factors will be used directly as near and far if LightShadowFrustumType::Fixed is used.
        bool setShadowCaster(
            bool castsShadow,
            uint32_t shadowMapWidth = SUN_LIGHT_DEFAULT_SHADOW_MAP_SIZE,
            uint32_t shadowMapHeight = SUN_LIGHT_DEFAULT_SHADOW_MAP_SIZE,
            LightShadowFrustumType = LightShadowFrustumType::CameraBased,
            float nearFactor = 1.0f,
            float farFactor = 1.0f
        ) noexcept;

        [[nodiscard]] glm_vec3 getPosition() const noexcept;
        void setPosition(decimal x, decimal y, decimal z) noexcept;
        void setPosition(const glm_vec3& position) noexcept;
};

class SunLightShadowData {

};

#endif