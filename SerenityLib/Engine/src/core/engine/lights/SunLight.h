#pragma once
#ifndef ENGINE_LIGHT_SUN_INCLUDE_GUARD
#define ENGINE_LIGHT_SUN_INCLUDE_GUARD

#include <core/Camera.h>
#include <core/engine/lights/LightIncludes.h>

namespace Engine {
namespace epriv {
    class RenderManager;
};
};

class SunLight : public EntityWrapper {
    friend class ::Engine::epriv::RenderManager;
    protected:
        bool               m_Active;
        glm::vec4          m_Color;
        LightType::Type    m_Type;
        float              m_AmbientIntensity;
        float              m_DiffuseIntensity;
        float              m_SpecularIntensity;
    public:
        SunLight(
            const glm::vec3 position   = glm::vec3(0.0f, 0.0f, 0.0f),
            const LightType::Type type = LightType::Sun,
            Scene* scene               = nullptr
        );
        virtual ~SunLight();

        const glm::vec3 position();
        const bool isActive() const;
        const uint type() const;

        const float getAmbientIntensity() const;
        const float getDiffuseIntensity() const;
        const float getSpecularIntensity() const;

        void setAmbientIntensity(const float a);  
        void setDiffuseIntensity(const float d);  
        void setSpecularIntensity(const float s);

        void setColor(const float r, const float g, const float b, const float a = 1.0f);    
        void setColor(const glm::vec4& color);
        void setColor(const glm::vec3& color);
        void setPosition(const float x, const float y, const float z);                       
        void setPosition(const glm::vec3&);
        void activate(const bool active = true);                          
        void deactivate();
};
#endif