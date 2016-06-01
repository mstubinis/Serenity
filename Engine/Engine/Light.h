#pragma once
#ifndef ENGINE_LIGHT_H
#define ENGINE_LIGHT_H
#include "ObjectDisplay.h"

enum LIGHT_TYPE {
    LIGHT_TYPE_SUN,
    LIGHT_TYPE_POINT,
    LIGHT_TYPE_DIRECTIONAL,
    LIGHT_TYPE_SPOT
};
class Scene;
class SunLight: public ObjectDisplay{
    protected:
        unsigned int m_Type;
        float m_AmbientIntensity, m_DiffuseIntensity, m_SpecularPower;
        void sendGenericAttributesToShader(GLuint);
    public:
        SunLight(glm::v3 = glm::v3(0),std::string = "Sun Light",unsigned int=LIGHT_TYPE_SUN,Scene* = nullptr);
        virtual ~SunLight();

        void update(float);

        virtual void render(GLuint=0,bool=false);
        virtual void draw(GLuint shader,bool=false,bool=false);

        virtual void lighten(GLuint);
        float getSpecularPower(){ return m_SpecularPower; }
        void setSpecularPower(float s){ m_SpecularPower = s; }

        void setName(std::string);
};
class DirectionalLight: public SunLight{
    private:
        glm::vec3 m_Direction;
    public:
        DirectionalLight(std::string = "Directional Light",glm::vec3 = glm::vec3(0,0,-1), Scene* = nullptr);
        virtual ~DirectionalLight();

        virtual void lighten(GLuint);
};
class PointLight: public SunLight{
    private:
        float m_Constant, m_Linear, m_Exp;
        float m_PointLightRadius;
    protected:
        float calculatePointLightRadius();
    public:
        PointLight(std::string = "Point Light",glm::v3 = glm::v3(0), Scene* = nullptr);
        virtual ~PointLight();

        virtual void setConstant(float c);
        virtual void setLinear(float l);
        virtual void setExponent(float e);

        float& getConstant(){ return m_Constant; }
        float& getLinear(){ return m_Linear; }
        float& getExponent(){ return m_Exp; }
        glm::vec3 getAttributes(){ return glm::vec3(m_Constant,m_Linear,m_Exp); }

        virtual void lighten(GLuint);
};
class SpotLight: public SunLight{
    private:
        glm::vec3 m_Direction;
        float m_Cutoff;

    public:
        SpotLight(std::string = "Spot Light",glm::v3 = glm::v3(0), Scene* = nullptr);
        virtual ~SpotLight();

        virtual void lighten(GLuint);
};
#endif