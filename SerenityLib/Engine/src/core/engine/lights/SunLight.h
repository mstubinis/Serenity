#pragma once
#ifndef ENGINE_LIGHT_SUN_INCLUDE_GUARD
#define ENGINE_LIGHT_SUN_INCLUDE_GUARD

class Scene;
namespace Engine::priv {
    class Renderer;
};

#include <core/engine/lights/LightIncludes.h>
#include <ecs/EntityWrapper.h>

class SunLight : public EntityWrapper {
    friend class ::Engine::priv::Renderer;
    protected:
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

        const glm_vec3 position();
        const glm::vec4& color() const;
        const bool isActive() const;
        const unsigned int type() const;

        const float getAmbientIntensity() const;
        const float getDiffuseIntensity() const;
        const float getSpecularIntensity() const;

        void setAmbientIntensity(const float a);  
        void setDiffuseIntensity(const float d);  
        void setSpecularIntensity(const float s);

        void setColor(const float r, const float g, const float b, const float a = 1.0f);    
        void setColor(const glm::vec4& color);
        void setColor(const glm::vec3& color);
        void setPosition(const decimal& x, const decimal& y, const decimal& z);
        void setPosition(const glm_vec3&);
        void activate(const bool active = true);                          
        void deactivate();
};
#endif