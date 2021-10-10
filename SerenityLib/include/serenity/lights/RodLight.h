#pragma once
#ifndef ENGINE_LIGHT_ROD_H
#define ENGINE_LIGHT_ROD_H

class RodLight;

#include <serenity/lights/PointLight.h>

constexpr float ROD_LIGHT_DEFAULT_ROD_LENGTH = 2.0f;

class RodLightShadowData {

};

class RodLight : public Entity, public LightAttenuationData, public LightBaseData<RodLight> {
    friend class ::Engine::priv::RenderModule;
    private:
        float m_RodLength = ROD_LIGHT_DEFAULT_ROD_LENGTH;

        void calculateCullingRadius();
        void recalculateTransformScale();
    public:
        RodLight() = delete;
        RodLight(
            Scene*,
            const glm_vec3& position = glm_vec3{ 0.0f, 0.0f, 0.0f },
            float rodLength = ROD_LIGHT_DEFAULT_ROD_LENGTH
        );
        RodLight(
            Scene*,
            decimal x = 0.0, decimal y = 0.0, decimal z = 0.0,
            float rodLength = ROD_LIGHT_DEFAULT_ROD_LENGTH
        );

        void setConstant(float constant);
        void setLinear(float linear);
        void setExponent(float exponent);
        void setAttenuation(float constant, float linear, float exponent);
        void setAttenuation(LightRange);
        void setAttenuationModel(LightAttenuation);

        bool setShadowCaster(bool castsShadow) noexcept { return false; }

        [[nodiscard]] inline constexpr float getRodLength() const noexcept { return m_RodLength; }

        void setRodLength(float rodLength);
};
#endif