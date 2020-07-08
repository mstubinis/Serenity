#pragma once
#ifndef ENGINE_LIGHT_POINT_H
#define ENGINE_LIGHT_POINT_H

#include <core/engine/lights/SunLight.h>

class PointLight : public SunLight {
    friend class ::Engine::priv::Renderer;
    protected:
        LightAttenuation::Model  m_AttenuationModel = LightAttenuation::Constant_Linear_Exponent;
        float                    m_C                = 0.1f;
        float                    m_L                = 0.1f;
        float                    m_E                = 0.1f;
        float                    m_CullingRadius;

        virtual float            calculateCullingRadius();
    public:
        PointLight(
            const glm_vec3& position  = glm_vec3(0.0f, 0.0f, 0.0f),
            Scene* scene              = nullptr
        );
        PointLight(
            LightType::Type type,
            const glm_vec3& position  = glm_vec3(0.0f, 0.0f, 0.0f),
            Scene* scene              = nullptr
        );
        virtual ~PointLight() {}

        void free();

        void setConstant(float constant);
        void setLinear(float linear);
        void setExponent(float exponent);
        void setAttenuation(float constant, float linear, float exponent);
        void setAttenuation(LightRange::Range range);
        void setAttenuationModel(LightAttenuation::Model model);

        constexpr float getCullingRadius() const noexcept { return m_CullingRadius; }
        constexpr float getConstant() const noexcept { return m_C; }
        constexpr float getLinear() const noexcept { return m_L; }
        constexpr float getExponent() const noexcept { return m_E; }
        constexpr LightAttenuation::Model getAttenuationModel() const noexcept { return m_AttenuationModel; }
};
#endif