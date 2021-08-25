#pragma once
#ifndef ENGINE_LIGHT_INCLUDES_H
#define ENGINE_LIGHT_INCLUDES_H

class ComponentTransform;

#include <serenity/system/TypeDefs.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

constexpr float LIGHT_DEFAULT_CONSTANT           = 0.1f;
constexpr float LIGHT_DEFAULT_LINEAR             = 0.1f;
constexpr float LIGHT_DEFAULT_EXPONENT           = 0.1f;
constexpr float LIGHT_DEFAULT_DIFFUSE_INTENSITY  = 2.0f;
constexpr float LIGHT_DEFAULT_SPECULAR_INTENSITY = 1.0f;

struct LightDefaultAttenuationData final {
    float constant = LIGHT_DEFAULT_CONSTANT;
    float linear   = LIGHT_DEFAULT_LINEAR;
    float exponent = LIGHT_DEFAULT_EXPONENT;
};

enum class LightType : uint8_t {
    Sun = 0, 
    Point, 
    Directional, 
    Spot, 
    Rod,
    Projection,
};
enum class LightRange : uint8_t {
    _7 = 0, 
    _13, 
    _20, 
    _32, 
    _50, 
    _65, 
    _100, 
    _160, 
    _200, 
    _325, 
    _600, 
    _3250,
_TOTAL};
enum class LightAttenuation : uint8_t {
    Constant = 0,
    Distance,
    Distance_Squared,
    Constant_Linear_Exponent,
    Distance_Radius_Squared,
};

template<class T>
class LightBaseData {
    public:
        static inline uint32_t TYPE_ID = 0;
    protected:
        glm::vec4          m_Color             = glm::vec4{ 1.0f };
        float              m_DiffuseIntensity  = LIGHT_DEFAULT_DIFFUSE_INTENSITY;
        float              m_SpecularIntensity = LIGHT_DEFAULT_SPECULAR_INTENSITY;
        LightType          m_Type              = LightType::Sun;
        bool               m_IsShadowCaster    = false;
        bool               m_Active            = true;
    public:
        LightBaseData() = delete;
        LightBaseData(LightType type) 
            : m_Type { type }
        {}

        [[nodiscard]] inline bool isShadowCaster() const noexcept { return m_IsShadowCaster; }

        [[nodiscard]] inline const glm::vec4& getColor() const noexcept { return m_Color; }
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
        void setColor(float r, float g, float b, float a = 1.0f) noexcept { m_Color = glm::vec4{ r, g, b, a }; }

        //default color is white (1.0, 1.0, 1.0, 1.0)
        void setColor(const glm::vec4& color) noexcept { m_Color = color; }

        //default color is white (1.0, 1.0, 1.0, 1.0)
        void setColor(const glm::vec3& color) noexcept { m_Color = glm::vec4{ color, m_Color.a }; }
};

class LightAttenuationData {
    protected:
        LightAttenuation   m_AttenuationModel = LightAttenuation::Constant_Linear_Exponent;
        float              m_C                = LIGHT_DEFAULT_CONSTANT;
        float              m_L                = LIGHT_DEFAULT_LINEAR;
        float              m_E                = LIGHT_DEFAULT_EXPONENT;
        float              m_CullingRadius    = 0.0f;

        virtual void calculateCullingRadius(ComponentTransform*, const glm::vec4& lightColor);
    public:
        void setConstant(ComponentTransform*, float constant, const glm::vec4& lightColor);
        void setLinear(ComponentTransform*, float linear, const glm::vec4& lightColor);
        void setExponent(ComponentTransform*, float exponent, const glm::vec4& lightColor);
        void setAttenuation(ComponentTransform*, float constant, float linear, float exponent, const glm::vec4& lightColor);
        void setAttenuation(ComponentTransform*, LightRange, const glm::vec4& lightColor);
        void setAttenuationModel(ComponentTransform*, LightAttenuation, const glm::vec4& lightColor);

        [[nodiscard]] inline constexpr float getCullingRadius() const noexcept { return m_CullingRadius; }
        [[nodiscard]] inline constexpr float getConstant() const noexcept { return m_C; }
        [[nodiscard]] inline constexpr float getLinear() const noexcept { return m_L; }
        [[nodiscard]] inline constexpr float getExponent() const noexcept { return m_E; }
        [[nodiscard]] inline constexpr LightAttenuation getAttenuationModel() const noexcept { return m_AttenuationModel; }
};

#endif