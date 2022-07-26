#pragma once
#ifndef ENGINE_LIGHT_POINT_H
#define ENGINE_LIGHT_POINT_H

class ComponentTransform;
class PointLight;

#include <serenity/lights/SunLight.h>

class PointLightShadowData {

};

class PointLight : public Entity, public LightAttenuationData, public LightBaseData<PointLight> {
    friend class ::Engine::priv::RenderModule;
    protected:
        void calculateCullingRadius();
    public:
        PointLight() = delete;
        PointLight(Scene*, const glm_vec3& position);
        PointLight(Scene*, decimal x, decimal y, decimal z);

        void setConstant(float constant);
        void setLinear(float linear);
        void setExponent(float exponent);
        void setAttenuation(float constant, float linear, float exponent);
        void setAttenuation(LightRange);
        void setAttenuationModel(LightAttenuation);

        bool setShadowCaster(bool castsShadow) noexcept { return false; }
};
#endif