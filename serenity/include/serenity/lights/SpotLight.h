#pragma once
#ifndef ENGINE_LIGHT_SPOT_H
#define ENGINE_LIGHT_SPOT_H

class SpotLight;

#include <serenity/lights/PointLight.h>

constexpr float SPOT_LIGHT_DEFAULT_CUTOFF_DEGREES = 12.5f;
constexpr float SPOT_LIGHT_DEFAULT_OUTER_CUTOFF_DEGREES = 17.5f;

class SpotLightShadowData {

};

class SpotLight : public Entity, public LightAttenuationData, public LightBaseData<SpotLight> {
    friend class ::Engine::priv::RenderModule;
    private:
        float   m_CutoffDegrees      = SPOT_LIGHT_DEFAULT_CUTOFF_DEGREES;
        float   m_OuterCutoffDegrees = SPOT_LIGHT_DEFAULT_OUTER_CUTOFF_DEGREES;

        void calculateCullingRadius();
    public:
        SpotLight() = delete;
        SpotLight(
            Scene*,
            const glm_vec3& position = glm_vec3{ 0.0f, 0.0f, 0.0f },
            const glm_vec3& direction = glm_vec3{ 0.0f, 0.0f, -1.0f },
            float innerCutoffInDegrees = SPOT_LIGHT_DEFAULT_CUTOFF_DEGREES,
            float outerCutoffInDegrees = SPOT_LIGHT_DEFAULT_OUTER_CUTOFF_DEGREES
        );
        SpotLight(
            Scene*,
            decimal x = 0.0, decimal y = 0.0, decimal z = 0.0,
            const glm_vec3& direction = glm_vec3{ 0.0f, 0.0f, -1.0f },
            float innerCutoffInDegrees = SPOT_LIGHT_DEFAULT_CUTOFF_DEGREES,
            float outerCutoffInDegrees = SPOT_LIGHT_DEFAULT_OUTER_CUTOFF_DEGREES
        );

        void setConstant(float constant);
        void setLinear(float linear);
        void setExponent(float exponent);
        void setAttenuation(float constant, float linear, float exponent);
        void setAttenuation(LightRange);
        void setAttenuationModel(LightAttenuation);

        bool setShadowCaster(bool castsShadow) noexcept { return false; }

        //returns the inner cutoff in degrees
        [[nodiscard]] inline constexpr float getCutoff() const noexcept { return m_CutoffDegrees; }

        //returns the outer cutoff in degrees
        [[nodiscard]] inline constexpr float getCutoffOuter() const noexcept { return m_OuterCutoffDegrees; }

        void setDirection(float xDir, float yDir, float zDir) noexcept;
        void setDirection(const glm::vec3& direction) noexcept;

        inline void setCutoffRadians(float cutoffInRadians) noexcept { m_CutoffDegrees = glm::degrees(cutoffInRadians); }
        inline void setCutoffOuterRadians(float outerCutoffInRadians) noexcept { m_OuterCutoffDegrees = glm::degrees(outerCutoffInRadians); }
        inline void setCutoffDegrees(float cutoffInDegrees) noexcept { m_CutoffDegrees = cutoffInDegrees; }
        inline void setCutoffOuterDegrees(float outerCutoffInDegrees) noexcept { m_OuterCutoffDegrees = outerCutoffInDegrees; }
};
#endif