#pragma once
#ifndef ENGINE_LIGHTS_DIRECITONAL_H
#define ENGINE_LIGHTS_DIRECITONAL_H

#include <core/engine/lights/SunLight.h>

class DirectionalLight : public SunLight {
    friend class Engine::priv::Renderer;
    public:
        DirectionalLight(
            const glm::vec3& direction = glm::vec3(0.0f, 0.0f, -1.0f),
            Scene*                     = nullptr
        );
        virtual ~DirectionalLight() {}

        void free() noexcept override;
};

#endif