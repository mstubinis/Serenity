#pragma once
#ifndef ENGINE_LIGHT_ROD_INCLUDE_GUARD
#define ENGINE_LIGHT_ROD_INCLUDE_GUARD

#include "core/engine/lights/PointLight.h"

class RodLight : public PointLight {
    private:
        float m_RodLength;
        float calculateCullingRadius();
    public:
        RodLight(glm::vec3 = glm::vec3(0.0f), float = 2.0f, Scene* = nullptr);
        virtual ~RodLight();

        void lighten();

        float rodLength();
        void setRodLength(float);
};

#endif