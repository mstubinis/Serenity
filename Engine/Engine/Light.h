#pragma once
#ifndef ENGINE_LIGHT_H
#define ENGINE_LIGHT_H

#include "ObjectDisplay.h"
#include "Camera.h"
#include <unordered_map>

class Scene;
class ShaderP;
class LightType{public: enum Type{
    Sun,Point,Directional,Spot,Rod
};};
class LightRange{public:enum Range{
    _7,_13,_20,_32,_50,_65,_100,_160,_200,_325,_600,_3250
};};
class LightAttenuation{public:enum Model{
    Constant,
    Distance,
    Distance_Squared,
    Constant_Linear_Exponent,
    Distance_Radius_Squared,
};};
class SunLight: public Entity{
    protected:
		ComponentBasicBody* m_Body;
		glm::vec4 m_Color;
        uint m_Type;
        bool m_Active;
        float m_AmbientIntensity, m_DiffuseIntensity, m_SpecularIntensity;
        void sendGenericAttributesToShader();
    public:
        SunLight(glm::vec3 = glm::vec3(0.0f),uint=LightType::Sun,Scene* = nullptr);
        virtual ~SunLight();

        virtual void lighten();
        float getAmbientIntensity();
        void setAmbientIntensity(float a);
        float getDiffuseIntensity();
        void setDiffuseIntensity(float d);
        float getSpecularIntensity();
        void setSpecularIntensity(float s);

		glm::vec3 position();
		void setColor(float,float,float,float);
		void setColor(glm::vec4);
		void setPosition(float,float,float);
		void setPosition(glm::vec3);
        void activate(bool=true);
        void deactivate();
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
        float m_Constant, m_Linear, m_Exp;
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
        float m_Cutoff;
        float m_OuterCutoff;
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

        void update(float);
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
