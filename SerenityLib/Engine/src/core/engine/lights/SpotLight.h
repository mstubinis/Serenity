#pragma once
#ifndef ENGINE_LIGHT_SPOT_H
#define ENGINE_LIGHT_SPOT_H

#include <core/engine/lights/PointLight.h>

class SpotLight : public PointLight {
    friend class ::Engine::priv::RenderModule;
    private:
        float   m_Cutoff;
        float   m_OuterCutoff;
    public:
        SpotLight(
            const glm_vec3& position   = glm_vec3(0.0f, 0.0f, 0.0f),
            const glm_vec3& direction  = glm_vec3(0.0f, 0.0f, -1.0f),
            float innerCutoffInDegrees = 11.0f,
            float outerCutoffInDegrees = 13.0f,
            Scene* scene               = nullptr
        );
        virtual ~SpotLight();

        void free() noexcept override;

        inline CONSTEXPR float getCutoff() const noexcept { return m_Cutoff; }
        inline CONSTEXPR float getCutoffOuter() const noexcept { return m_OuterCutoff; }

        void setDirection(decimal xDir, decimal yDir, decimal zDir) noexcept;
        void setDirection(const glm_vec3& direction) noexcept;

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