#pragma once
#ifndef ENGINE_LIGHT_SUN_H
#define ENGINE_LIGHT_SUN_H

class Scene;
class SunLight;
namespace Engine::priv {
    class RenderModule;
};

#include <serenity/lights/LightIncludes.h>
#include <serenity/ecs/entity/EntityBody.h>
#include <glm/vec4.hpp>

constexpr float LIGHT_DEFAULT_DIFFUSE_INTENSITY  = 2.0f;
constexpr float LIGHT_DEFAULT_SPECULAR_INTENSITY = 1.0f;

class SunLightShadowData {

};

class SunLight : public EntityBody {
    friend class ::Engine::priv::RenderModule;
    protected:
        glm::vec4          m_Color             = glm::vec4{ 1.0f };
        float              m_DiffuseIntensity  = LIGHT_DEFAULT_DIFFUSE_INTENSITY;
        float              m_SpecularIntensity = LIGHT_DEFAULT_SPECULAR_INTENSITY;
        LightType          m_Type              = LightType::Sun;
        bool               m_IsShadowCaster    = false;
        bool               m_Active            = true;
    public:
        SunLight() = delete;
        SunLight(Scene* scene, const glm_vec3& position, LightType type = LightType::Sun);
        SunLight(Scene* scene, decimal x, decimal y, decimal z, LightType type = LightType::Sun);
        virtual ~SunLight() = default;

        [[nodiscard]] bool isShadowCaster() const noexcept;
        virtual bool setShadowCaster(bool castsShadow) noexcept;

        [[nodiscard]] glm_vec3 getPosition() const;
        [[nodiscard]] inline  const glm::vec4& getColor() const noexcept { return m_Color; }
        [[nodiscard]] inline constexpr bool isActive() const noexcept { return m_Active; }
        [[nodiscard]] inline constexpr LightType getType() const noexcept { return m_Type; }
        [[nodiscard]] inline constexpr float getDiffuseIntensity() const noexcept { return m_DiffuseIntensity; }
        [[nodiscard]] inline constexpr float getSpecularIntensity() const noexcept { return m_SpecularIntensity; }

        //default diffuse intensity is 2.0
        void setDiffuseIntensity(float d) noexcept { m_DiffuseIntensity = d; }

        //default specular intensity is 1.0
        void setSpecularIntensity(float s) noexcept { m_SpecularIntensity = s; }

        void activate(bool b = true) noexcept { m_Active = b; }
        void deactivate() noexcept { m_Active = false; }

        //default color is white (1.0, 1.0, 1.0, 1.0)
        void setColor(float r, float g, float b, float a = 1.0f) noexcept { m_Color = glm::vec4(r, g, b, a); }

        //default color is white (1.0, 1.0, 1.0, 1.0)
        void setColor(const glm::vec4& color) noexcept { m_Color = color; }

        //default color is white (1.0, 1.0, 1.0, 1.0)
        void setColor(const glm::vec3& color) noexcept { m_Color.r = color.r; m_Color.g = color.g; m_Color.b = color.b; }

        void setPosition(decimal x, decimal y, decimal z);
        void setPosition(decimal position);
        void setPosition(const glm_vec3& position);
};
#endif