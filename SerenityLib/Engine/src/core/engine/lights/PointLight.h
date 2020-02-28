#pragma once
#ifndef ENGINE_LIGHT_POINT_INCLUDE_GUARD
#define ENGINE_LIGHT_POINT_INCLUDE_GUARD

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
            const LightType::Type type,
            const glm_vec3& position  = glm_vec3(0.0f, 0.0f, 0.0f),
            Scene* scene              = nullptr
        );
        virtual ~PointLight();

        inline void destroy();

        void setConstant(const float constant);
        void setLinear(const float linear);
        void setExponent(const float exponent);
        void setAttenuation(const float constant, const float linear, const float exponent);
        void setAttenuation(const LightRange::Range range);
        void setAttenuationModel(const LightAttenuation::Model model);

        const float getCullingRadius() const;
        const float getConstant() const;
        const float getLinear() const;
        const float getExponent() const;
        const LightAttenuation::Model getAttenuationModel() const;
};
#endif