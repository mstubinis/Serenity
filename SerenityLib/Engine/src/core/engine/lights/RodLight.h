#pragma once
#ifndef ENGINE_LIGHT_ROD_INCLUDE_GUARD
#define ENGINE_LIGHT_ROD_INCLUDE_GUARD

#include <core/engine/lights/PointLight.h>

class RodLight : public PointLight {
    friend class ::Engine::priv::Renderer;
    private:
        float m_RodLength;
        float calculateCullingRadius();
    public:
        RodLight(
            const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
            const float rodLength    = 2.0f,
            Scene* scene             = nullptr
        );
        virtual ~RodLight();

        inline void destroy();

        const float rodLength() const;
        void setRodLength(const float rodLength);
};
#endif