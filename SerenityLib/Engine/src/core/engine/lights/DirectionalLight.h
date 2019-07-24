#pragma once
#ifndef ENGINE_LIGHTS_DIRECITONAL_INCLUDE_GUARD
#define ENGINE_LIGHTS_DIRECITONAL_INCLUDE_GUARD

#include <core/engine/lights/SunLight.h>

class DirectionalLight : public SunLight {
    friend class Engine::epriv::RenderManager;
    public:
        DirectionalLight(
            const glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f),
            Scene*                    = nullptr
        );
        virtual ~DirectionalLight();
};

#endif