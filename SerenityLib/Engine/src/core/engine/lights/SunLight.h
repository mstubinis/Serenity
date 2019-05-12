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
            glm::vec3 position = glm::vec3(0.0f),
            LightType::Type = LightType::Sun,
            Scene* = nullptr
        );
        virtual ~SunLight();

        float getAmbientIntensity();     void setAmbientIntensity(float a);
        float getDiffuseIntensity();     void setDiffuseIntensity(float d);
        float getSpecularIntensity();    void setSpecularIntensity(float s);

        glm::vec3 position();
        void setColor(float, float, float, float = 1.0f);    void setColor(glm::vec4);
        void setColor(glm::vec3);
        void setPosition(float, float, float);               void setPosition(glm::vec3);
        void activate(bool = true);                          void deactivate();
        bool isActive();
        uint type();
};

#endif