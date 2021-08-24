#pragma once
#ifndef ENGINE_LIGHT_ROD_H
#define ENGINE_LIGHT_ROD_H

class RodLight;

#include <serenity/lights/PointLight.h>

constexpr float ROD_LIGHT_DEFAULT_ROD_LENGTH = 2.0f;

class RodLightShadowData {

};

class RodLight : public PointLight {
    friend class ::Engine::priv::RenderModule;
    private:
        float m_RodLength = ROD_LIGHT_DEFAULT_ROD_LENGTH;

        void calculateCullingRadius() override;
        void recalculateTransformScale();
    public:
        RodLight() = delete;
        RodLight(
            Scene* scene,
            const glm_vec3& position = glm_vec3{ 0.0f, 0.0f, 0.0f },
            float rodLength = ROD_LIGHT_DEFAULT_ROD_LENGTH
        );
        virtual ~RodLight();

        [[nodiscard]] inline constexpr float getRodLength() const noexcept { return m_RodLength; }

        void setRodLength(float rodLength);
};
#endif