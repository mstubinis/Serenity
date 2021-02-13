#pragma once
#ifndef ENGINE_LIGHTS_DIRECITONAL_H
#define ENGINE_LIGHTS_DIRECITONAL_H

class DirectionalLight;

#include <serenity/lights/SunLight.h>

class IDirectionalLightShadowData {

};

class DirectionalLight : public SunLight {
    friend class Engine::priv::RenderModule;
    private:

    public:
        DirectionalLight() = delete;
        DirectionalLight(
            Scene* scene,
            const glm::vec3& direction = glm::vec3(0.0f, 0.0f, -1.0f)
        );
        virtual ~DirectionalLight();

        bool setShadowCaster(bool castsShadow) noexcept override;
};

#endif