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
            const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
            Scene* scene             = nullptr
        );
        PointLight(
            const LightType::Type type,
            const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
            Scene* scene             = nullptr
        );
        virtual ~PointLight();

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
        const LightAttenuation::Model& getAttenuationModel() const;
};
#endif