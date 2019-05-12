#pragma once
#ifndef ENGINE_LIGHT_POINT_INCLUDE_GUARD
#define ENGINE_LIGHT_POINT_INCLUDE_GUARD

#include <core/engine/lights/SunLight.h>

class PointLight : public SunLight {
    friend class ::Engine::epriv::RenderManager;
    protected:
        float                    m_C;
        float                    m_L;
        float                    m_E;
        float                    m_CullingRadius;
        LightAttenuation::Model  m_AttenuationModel;
        virtual float            calculateCullingRadius();
    public:
        PointLight(
            glm::vec3 position = glm::vec3(0.0f),
            Scene* = nullptr
        );
        PointLight(
            LightType::Type,
            glm::vec3 position = glm::vec3(0.0f),
            Scene* = nullptr
        );
        virtual ~PointLight();

        void setConstant(float constant);
        void setLinear(float linear);
        void setExponent(float exponent);
        void setAttenuation(float constant, float linear, float exponent);
        void setAttenuation(LightRange::Range);
        void setAttenuationModel(LightAttenuation::Model);

        float getCullingRadius();
        float getConstant();
        float getLinear();
        float getExponent();
};

#endif