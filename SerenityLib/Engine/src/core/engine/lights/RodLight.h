#pragma once
#ifndef ENGINE_LIGHT_ROD_INCLUDE_GUARD
#define ENGINE_LIGHT_ROD_INCLUDE_GUARD

#include <core/engine/lights/PointLight.h>

class RodLight : public PointLight {
    friend class ::Engine::epriv::RenderManager;
    private:
        float m_RodLength;
        float calculateCullingRadius();
    public:
        RodLight(
            glm::vec3 position = glm::vec3(0.0f),
            float rodLength = 2.0f,
            Scene* = nullptr
        );
        virtual ~RodLight();

        float rodLength();
        void setRodLength(float);
};
#endif