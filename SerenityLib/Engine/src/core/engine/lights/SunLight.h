#pragma once
#ifndef ENGINE_LIGHT_SUN_INCLUDE_GUARD
#define ENGINE_LIGHT_SUN_INCLUDE_GUARD

class Scene;
namespace Engine::priv {
    class Renderer;
};

#include <core/engine/lights/LightIncludes.h>
#include <ecs/Entity.h>

class SunLight : public Entity {
    friend class ::Engine::priv::Renderer;
    protected:
        bool               m_IsShadowCaster    = false;
        bool               m_Active            = true;
        glm::vec4          m_Color             = glm::vec4(1.0f);
        LightType::Type    m_Type;
        float              m_AmbientIntensity  = 0.005f;
        float              m_DiffuseIntensity  = 2.0f;
        float              m_SpecularIntensity = 1.0f;
    public:
        SunLight(
            const glm_vec3& position    = glm_vec3(0.0),
            const LightType::Type type = LightType::Sun,
            Scene* scene               = nullptr
        );
        virtual ~SunLight();

        inline void destroy();

        glm_vec3 position() const;
        const glm::vec4& color() const;
        bool isActive() const;
        LightType::Type type() const;
        bool isShadowCaster() const;
        void setShadowCaster(bool castsShadow);

        float getAmbientIntensity() const;
        float getDiffuseIntensity() const;
        float getSpecularIntensity() const;

        void setAmbientIntensity(float a);
        void setDiffuseIntensity(float d);
        void setSpecularIntensity(float s);

        void setColor(float r, float g, float b, float a = 1.0f);    
        void setColor(const glm::vec4& color);
        void setColor(const glm::vec3& color);

        void setPosition(const decimal& x, const decimal& y, const decimal& z);
        void setPosition(const decimal& position);
        void setPosition(const glm_vec3& position);

        void activate(bool active = true);                          
        void deactivate();
};
#endif