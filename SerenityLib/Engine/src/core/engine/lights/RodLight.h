#pragma once
#ifndef ENGINE_LIGHT_ROD_H
#define ENGINE_LIGHT_ROD_H

#include <core/engine/lights/PointLight.h>

constexpr float ROD_LIGHT_DEFAULT_ROD_LENGTH = 2.0f;

class RodLight : public PointLight {
    friend class ::Engine::priv::RenderModule;
    private:
        float m_RodLength = ROD_LIGHT_DEFAULT_ROD_LENGTH;
        float calculateCullingRadius() override;

    public:
        RodLight() = delete;
        RodLight(
            Scene* scene,
            const glm_vec3& position = glm_vec3(0.0f, 0.0f, 0.0f),
            float rodLength = ROD_LIGHT_DEFAULT_ROD_LENGTH
        );
        virtual ~RodLight();

        inline CONSTEXPR float rodLength() const noexcept { return m_RodLength; }

        void setRodLength(float rodLength);
};
#endif