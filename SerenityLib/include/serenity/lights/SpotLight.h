#pragma once
#ifndef ENGINE_LIGHT_SPOT_H
#define ENGINE_LIGHT_SPOT_H

class SpotLight;

#include <serenity/lights/PointLight.h>

constexpr float SPOT_LIGHT_DEFAULT_CUTOFF = 11.0f;
constexpr float SPOT_LIGHT_DEFAULT_OUTER_CUTOFF = 13.0f;

class SpotLightShadowData {

};

class SpotLight : public PointLight {
    friend class ::Engine::priv::RenderModule;
    private:
        float   m_Cutoff      = SPOT_LIGHT_DEFAULT_CUTOFF;
        float   m_OuterCutoff = SPOT_LIGHT_DEFAULT_OUTER_CUTOFF;
    public:
        SpotLight() = delete;
        SpotLight(
            Scene* scene,
            const glm_vec3& position = glm_vec3(0.0f, 0.0f, 0.0f),
            const glm_vec3& direction = glm_vec3(0.0f, 0.0f, -1.0f),
            float innerCutoffInDegrees = SPOT_LIGHT_DEFAULT_CUTOFF,
            float outerCutoffInDegrees = SPOT_LIGHT_DEFAULT_OUTER_CUTOFF
        );
        virtual ~SpotLight() {}

        [[nodiscard]] inline constexpr float getCutoff() const noexcept { return m_Cutoff; }
        [[nodiscard]] inline constexpr float getCutoffOuter() const noexcept { return m_OuterCutoff; }

        void setDirection(float xDir, float yDir, float zDir) noexcept;
        void setDirection(const glm::vec3& direction) noexcept;

        void setCutoffRadians(float cutoffInRadians) noexcept { m_Cutoff = glm::cos(cutoffInRadians); }
        void setCutoffOuterRadians(float outerCutoffInRadians) noexcept { m_OuterCutoff = glm::cos(outerCutoffInRadians); }
        void setCutoffDegrees(float cutoffInDegrees) noexcept { m_Cutoff = glm::cos(glm::radians(cutoffInDegrees)); }
        void setCutoffOuterDegrees(float outerCutoffInDegrees) noexcept { m_OuterCutoff = glm::cos(glm::radians(outerCutoffInDegrees)); }
        //sets the inner cut off, expecting the input to be in degrees
        inline void setCutoff(float cutoffInDegrees) noexcept { setCutoffDegrees(cutoffInDegrees); }
        //sets the outer cut off, expecting the input to be in degrees
        inline void setCutoffOuter(float outerCutoffInDegrees) noexcept { setCutoffOuterDegrees(outerCutoffInDegrees); }
};
#endif