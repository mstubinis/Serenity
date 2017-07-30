#pragma once
#ifndef ENGINE_LIGHT_H
#define ENGINE_LIGHT_H

#include "ObjectDisplay.h"

#include <boost/tuple/tuple.hpp>
#include <unordered_map>

enum LightType {
    Sun,
    Point,
    Directional,
    Spot
};
enum LightRange{
    _7,
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
    _3250
};

class Scene;
class SunLight: public ObjectDisplay{
    protected:
        unsigned int m_Type;
        bool m_Active;
        float m_AmbientIntensity, m_DiffuseIntensity, m_SpecularIntensity;
        void sendGenericAttributesToShader();
    public:
        SunLight(glm::vec3 = glm::vec3(0.0f),std::string = "Sun Light",uint=LightType::Sun,Scene* = nullptr);
        virtual ~SunLight();

        void update(float);

        virtual void lighten();
        float getAmbientIntensity(){ return m_AmbientIntensity; }
        void setAmbientIntensity(float a){ m_AmbientIntensity = a; }
        float getDiffuseIntensity(){ return m_DiffuseIntensity; }
        void setDiffuseIntensity(float d){ m_DiffuseIntensity = d; }
        float getSpecularIntensity(){ return m_SpecularIntensity; }
        void setSpecularIntensity(float s){ m_SpecularIntensity = s; }

        void activate(){ m_Active = true; }
        void deactivate(){ m_Active = false; }
        bool isActive(){ return m_Active; }
};
class DirectionalLight: public SunLight{
    public:
        DirectionalLight(std::string = "Directional Light",glm::vec3 = glm::vec3(0.0f,0.0f,-1.0f), Scene* = nullptr);
        virtual ~DirectionalLight();

        virtual void lighten();
};
class PointLight: public SunLight{
    protected:
        float m_Constant, m_Linear, m_Exp;
        float m_PointLightRadius;
        float calculatePointLightRadius();
    public:
        PointLight(std::string = "Point Light",glm::vec3 = glm::vec3(0.0f), Scene* = nullptr);
        virtual ~PointLight();

        void setConstant(float c);
        void setLinear(float l);
        void setExponent(float e);
        void setAttenuation(float c,float l, float e);
        void setAttenuation(LightRange);

        float& getLightRadius(){ return m_PointLightRadius; }

        float& getConstant(){ return m_Constant; }
        float& getLinear(){ return m_Linear; }
        float& getExponent(){ return m_Exp; }
        glm::vec3 getAttributes(){ return glm::vec3(m_Constant,m_Linear,m_Exp); }

        virtual void lighten();
};
class SpotLight: public PointLight{
    private:
        float m_Cutoff;
        float m_OuterCutoff;
    public:
        SpotLight(std::string = "Spot Light",glm::vec3 = glm::vec3(0.0f), glm::vec3 = glm::vec3(0.0f,0.0f,-1.0f), float = 11.0f, float = 13.0f,Scene* = nullptr);
        virtual ~SpotLight();

        void lighten();
        void setCutoff(float);
        void setCutoffOuter(float);
};
#endif
