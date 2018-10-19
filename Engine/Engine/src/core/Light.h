#pragma once
#ifndef ENGINE_LIGHT_H
#define ENGINE_LIGHT_H

#include "core/Camera.h"

class Scene;
class ShaderP;
class SunLight;
class DirectionalLight;
class PointLight;
class SpotLight;
class RodLight;
class EntityWrapper;

struct LightType{enum Type{
    Sun,Point,Directional,Spot,Rod,
_TOTAL};};
struct LightRange{enum Range{
    _7,_13,_20,_32,_50,_65,_100,_160,_200,_325,_600,_3250,
_TOTAL};};
struct LightAttenuation{enum Model{
    Constant,
    Distance,
    Distance_Squared,
    Constant_Linear_Exponent,
    Distance_Radius_Squared,
_TOTAL};};
class SunLight: public EntityWrapper{
    private:
        class impl;
    protected:
        std::unique_ptr<impl> m_i;
    public:
        SunLight(glm::vec3 = glm::vec3(0.0f),LightType::Type = LightType::Sun,Scene* = nullptr);
        virtual ~SunLight();

        virtual void lighten();
        float getAmbientIntensity();     void setAmbientIntensity(float a);
        float getDiffuseIntensity();     void setDiffuseIntensity(float d);
        float getSpecularIntensity();    void setSpecularIntensity(float s);

        glm::vec3 position();
        void setColor(float,float,float,float);  void setColor(glm::vec4);
        void setPosition(float,float,float);     void setPosition(glm::vec3);
        void activate(bool=true);                void deactivate();
        bool isActive();
        uint type();
};
class DirectionalLight: public SunLight{
    public:
        DirectionalLight(glm::vec3 = glm::vec3(0.0f,0.0f,-1.0f), Scene* = nullptr);
        virtual ~DirectionalLight();

        virtual void lighten();
};
class PointLight: public SunLight{
    protected:
        float m_C, m_L, m_E;
        float m_CullingRadius;
        virtual float calculateCullingRadius();
        LightAttenuation::Model m_AttenuationModel;
    public:
        PointLight(glm::vec3 = glm::vec3(0.0f), Scene* = nullptr);
        virtual ~PointLight();

        void setConstant(float c);
        void setLinear(float l);
        void setExponent(float e);
        void setAttenuation(float c,float l, float e);
        void setAttenuation(LightRange::Range);
        void setAttenuationModel(LightAttenuation::Model);

        float getCullingRadius();
        float getConstant();
        float getLinear();
        float getExponent();

        virtual void lighten();
};
class SpotLight: public PointLight{
    private:
        float m_Cutoff,  m_OuterCutoff;
    public:
        SpotLight(glm::vec3 = glm::vec3(0.0f), glm::vec3 = glm::vec3(0.0f,0.0f,-1.0f), float = 11.0f, float = 13.0f,Scene* = nullptr);
        virtual ~SpotLight();

        void lighten();
        void setCutoff(float);
        void setCutoffOuter(float);
};
class RodLight: public PointLight{
    private:
        float m_RodLength;
        float calculateCullingRadius();
    public:
        RodLight(glm::vec3 = glm::vec3(0.0f), float = 2.0f,Scene* = nullptr);
        virtual ~RodLight();

        void lighten();

        float rodLength();
        void setRodLength(float);
};
/*
class LightProbe: public Camera{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        LightProbe(uint envMapWidth = 128,glm::vec3 = glm::vec3(0),bool onlyOnce = false,Scene* = nullptr,uint sidesPerFrame = 6);
        ~LightProbe();

        void renderCubemap(ShaderP* convolude,ShaderP* prefilter);

        GLuint getEnvMap();
        GLuint getIrriadianceMap();
        GLuint getPrefilterMap();

        const uint getEnvMapSize() const;
};
*/
#endif
