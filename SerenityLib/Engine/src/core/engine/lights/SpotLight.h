#pragma once
#ifndef ENGINE_LIGHT_SPOT_H
#define ENGINE_LIGHT_SPOT_H

#include <core/engine/lights/PointLight.h>

class SpotLight : public PointLight {
    friend class ::Engine::priv::Renderer;
    private:
        float   m_Cutoff;
        float   m_OuterCutoff;
    public:
        SpotLight(
            const glm_vec3& position   = glm_vec3(0.0f, 0.0f, 0.0f),
            const glm::vec3& direction = glm::vec3(0.0f, 0.0f, -1.0f),
            float innerCutoff    = 11.0f,
            float outerCutoff    = 13.0f,
            Scene* scene               = nullptr
        );
        virtual ~SpotLight() {}

        void free();

        constexpr float getCutoff() const noexcept { return m_Cutoff; }
        constexpr float getCutoffOuter() const noexcept { return m_OuterCutoff; }

        void setCutoff(float innerCutoff);
        void setCutoffOuter(float outerCutoff);
};
#endif