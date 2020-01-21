#pragma once
#ifndef ENGINE_LIGHT_SPOT_INCLUDE_GUARD
#define ENGINE_LIGHT_SPOT_INCLUDE_GUARD

#include <core/engine/lights/PointLight.h>

class SpotLight : public PointLight {
    friend class ::Engine::priv::Renderer;
    private:
        float   m_Cutoff;
        float   m_OuterCutoff;
    public:
        SpotLight(
            const glm::vec3 position  = glm::vec3(0.0f, 0.0f, 0.0f),
            const glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f),
            const float innerCutoff   = 11.0f,
            const float outerCutoff   = 13.0f,
            Scene* scene              = nullptr
        );
        virtual ~SpotLight();

        inline void destroy();

        const float getCutoff() const;
        const float getCutoffOuter() const;

        void setCutoff(const float innerCutoff);
        void setCutoffOuter(const float outerCutoff);
};
#endif