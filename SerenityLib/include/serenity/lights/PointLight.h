#pragma once
#ifndef ENGINE_LIGHT_POINT_H
#define ENGINE_LIGHT_POINT_H

class PointLight;

#include <serenity/lights/SunLight.h>

constexpr float POINT_LIGHT_DEFAULT_CONSTANT = 0.1f;
constexpr float POINT_LIGHT_DEFAULT_LINEAR   = 0.1f;
constexpr float POINT_LIGHT_DEFAULT_EXPONENT = 0.1f;

struct PointLightDefaultAttenuationData final {
    float constant = POINT_LIGHT_DEFAULT_CONSTANT;
    float linear   = POINT_LIGHT_DEFAULT_LINEAR;
    float exponent = POINT_LIGHT_DEFAULT_EXPONENT;
};

class PointLightShadowData {

};

class PointLight : public SunLight {
    friend class ::Engine::priv::RenderModule;
    protected:
        LightAttenuation   m_AttenuationModel = LightAttenuation::Constant_Linear_Exponent;
        float              m_C                = POINT_LIGHT_DEFAULT_CONSTANT;
        float              m_L                = POINT_LIGHT_DEFAULT_LINEAR;
        float              m_E                = POINT_LIGHT_DEFAULT_EXPONENT;
        float              m_CullingRadius    = 0.0f;

        virtual float calculateCullingRadius();

    public:
        PointLight() = delete;
        PointLight(Scene*, const glm_vec3& position);
        PointLight(Scene*, decimal x, decimal y, decimal z);
        PointLight(Scene*, LightType, const glm_vec3& position);
        PointLight(Scene*, LightType, decimal x, decimal y, decimal z);
        virtual ~PointLight() = default;

        void setConstant(float constant);
        void setLinear(float linear);
        void setExponent(float exponent);
        void setAttenuation(float constant, float linear, float exponent);
        void setAttenuation(LightRange range);
        void setAttenuationModel(LightAttenuation model);

        [[nodiscard]] inline constexpr float getCullingRadius() const noexcept { return m_CullingRadius; }
        [[nodiscard]] inline constexpr float getConstant() const noexcept { return m_C; }
        [[nodiscard]] inline constexpr float getLinear() const noexcept { return m_L; }
        [[nodiscard]] inline constexpr float getExponent() const noexcept { return m_E; }
        [[nodiscard]] inline constexpr LightAttenuation getAttenuationModel() const noexcept { return m_AttenuationModel; }
};
#endif