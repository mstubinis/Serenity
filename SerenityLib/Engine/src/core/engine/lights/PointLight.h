#pragma once
#ifndef ENGINE_LIGHT_POINT_H
#define ENGINE_LIGHT_POINT_H

#include <core/engine/lights/SunLight.h>

struct PointLightDefaultAttenuationData final {
    float constant;
    float linear;
    float exponent;
};

class PointLight : public SunLight {
    friend class ::Engine::priv::Renderer;
    protected:
        LightAttenuation   m_AttenuationModel = LightAttenuation::Constant_Linear_Exponent;
        float              m_C                = 0.1f;
        float              m_L                = 0.1f;
        float              m_E                = 0.1f;
        float              m_CullingRadius    = 0.0f;

        virtual float calculateCullingRadius();
    public:
        PointLight(
            const glm_vec3& position  = glm_vec3(0.0f, 0.0f, 0.0f),
            Scene* scene              = nullptr
        );
        PointLight(
            LightType type,
            const glm_vec3& position  = glm_vec3(0.0f, 0.0f, 0.0f),
            Scene* scene              = nullptr
        );
        virtual ~PointLight() {}

        void free() noexcept override;

        void setConstant(float constant);
        void setLinear(float linear);
        void setExponent(float exponent);
        void setAttenuation(float constant, float linear, float exponent);
        void setAttenuation(LightRange range);
        void setAttenuationModel(LightAttenuation model);

        inline CONSTEXPR float getCullingRadius() const noexcept { return m_CullingRadius; }
        inline CONSTEXPR float getConstant() const noexcept { return m_C; }
        inline CONSTEXPR float getLinear() const noexcept { return m_L; }
        inline CONSTEXPR float getExponent() const noexcept { return m_E; }
        inline CONSTEXPR LightAttenuation getAttenuationModel() const noexcept { return m_AttenuationModel; }
};
#endif