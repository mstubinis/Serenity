#pragma once
#ifndef ENGINE_LIGHT_SUN_H
#define ENGINE_LIGHT_SUN_H

class Scene;
class SunLight;
namespace Engine::priv {
    class RenderModule;
};

#include <serenity/lights/LightIncludes.h>
#include <serenity/ecs/EntityBody.h>
#include <glm/vec4.hpp>

constexpr float LIGHT_DEFAULT_AMBIENT_INTENSITY  = 0.005f;
constexpr float LIGHT_DEFAULT_DIFFUSE_INTENSITY  = 2.0f;
constexpr float LIGHT_DEFAULT_SPECULAR_INTENSITY = 1.0f;

class ISunLightShadowData {

};

class SunLight : public EntityBody {
    friend class ::Engine::priv::RenderModule;
    protected:
        glm::vec4          m_Color             = glm::vec4(1.0f);
        float              m_AmbientIntensity  = LIGHT_DEFAULT_AMBIENT_INTENSITY;
        float              m_DiffuseIntensity  = LIGHT_DEFAULT_DIFFUSE_INTENSITY;
        float              m_SpecularIntensity = LIGHT_DEFAULT_SPECULAR_INTENSITY;
        LightType          m_Type              = LightType::Sun;
        bool               m_IsShadowCaster    = false;
        bool               m_Active            = true;
    public:
        SunLight() = delete;
        SunLight(Scene* scene, const glm_vec3& position = glm_vec3(0.0), LightType type = LightType::Sun);
        virtual ~SunLight();

        [[nodiscard]] glm_vec3 position() const;

        [[nodiscard]] bool isShadowCaster() const noexcept;
        virtual bool setShadowCaster(bool castsShadow) noexcept;

        [[nodiscard]] inline  const glm::vec4& color() const noexcept { return m_Color; }
        [[nodiscard]] inline constexpr bool isActive() const noexcept { return m_Active; }
        [[nodiscard]] inline constexpr LightType type() const noexcept { return m_Type; }
        [[nodiscard]] inline constexpr float getAmbientIntensity() const noexcept { return m_AmbientIntensity; }
        [[nodiscard]] inline constexpr float getDiffuseIntensity() const noexcept { return m_DiffuseIntensity; }
        [[nodiscard]] inline constexpr float getSpecularIntensity() const noexcept { return m_SpecularIntensity; }

        void setAmbientIntensity(float a) noexcept { m_AmbientIntensity = a; }
        void setDiffuseIntensity(float d) noexcept { m_DiffuseIntensity = d; }
        void setSpecularIntensity(float s) noexcept { m_SpecularIntensity = s; }
        void activate(bool b = true) noexcept { m_Active = b; }
        void deactivate() noexcept { m_Active = false; }

        void setColor(float r, float g, float b, float a = 1.0f) noexcept { m_Color = glm::vec4(r, g, b, a); }
        void setColor(const glm::vec4& color) noexcept { m_Color = color; }
        void setColor(const glm::vec3& color) noexcept { m_Color.r = color.r; m_Color.g = color.g; m_Color.b = color.b; }

        void setPosition(decimal x, decimal y, decimal z);
        void setPosition(decimal position);
        void setPosition(const glm_vec3& position);
};
#endif