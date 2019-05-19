#pragma once
#ifndef ENGINE_LIGHT_SPOT_INCLUDE_GUARD
#define ENGINE_LIGHT_SPOT_INCLUDE_GUARD

#include <core/engine/lights/PointLight.h>

class SpotLight : public PointLight {
    friend class ::Engine::epriv::RenderManager;
    private:
        float   m_Cutoff;
        float   m_OuterCutoff;
    public:
        SpotLight(
            glm::vec3 position = glm::vec3(0.0f),
            glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f),
            float innerCutoff = 11.0f,
            float outerCutoff = 13.0f,
            Scene* = nullptr
        );
        virtual ~SpotLight();

        void setCutoff(float);
        void setCutoffOuter(float);
};
#endif