#pragma once
#ifndef ENGINE_LIGHT_ROD_H
#define ENGINE_LIGHT_ROD_H

#include <core/engine/lights/PointLight.h>

class RodLight : public PointLight {
    friend class ::Engine::priv::Renderer;
    private:
        float m_RodLength;
        float calculateCullingRadius();
    public:
        RodLight(
            const glm_vec3& position = glm_vec3(0.0f, 0.0f, 0.0f),
            float rodLength          = 2.0f,
            Scene* scene             = nullptr
        );
        virtual ~RodLight() {}

        void free();

        constexpr float rodLength() const noexcept { return m_RodLength; }

        void setRodLength(float rodLength);
};
#endif