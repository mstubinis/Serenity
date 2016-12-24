#pragma once
#ifndef CAPSULE_SPACE_H
#define CAPSULE_SPACE_H

#include "SolarSystem.h"
#include "ObjectDisplay.h"

class PointLight;

class CapsuleEnd final: public ObjectDisplay{
    public:
        CapsuleEnd(float size,glm::v3 pos, glm::vec3 color, std::string name, Scene* = nullptr);
        ~CapsuleEnd();
        void update(float);
};
class CapsuleStar final: public ObjectDisplay{
        PointLight* m_Light;
    public:
        CapsuleStar(float size,glm::v3 pos, std::string name, Scene* = nullptr,bool=true);
        ~CapsuleStar();
        void update(float);
};
class CapsuleTunnel final: public ObjectDisplay{
    private:
        float m_TunnelRadius;
    public:
        CapsuleTunnel(float tunnelRadius, std::string name, std::string material, Scene* = nullptr);
        ~CapsuleTunnel();
        float getTunnelRadius(){ return m_TunnelRadius; }
};
class CapsuleRibbon final: public ObjectDisplay{
    private:
        float m_TunnelRadius;
    public:
        CapsuleRibbon(float tunnelRadius, std::string name, std::string material, Scene* = nullptr);
        ~CapsuleRibbon();

        float getTunnelRadius(){ return m_TunnelRadius; }
        void bind();
        void unbind();
};
class CapsuleSpace final: public SolarSystem{
    private:
        float m_Timer;
        CapsuleTunnel* m_TunnelA;
        CapsuleTunnel* m_TunnelB;
        CapsuleRibbon* m_Ribbon;
        CapsuleEnd* m_FrontEnd;
        CapsuleEnd* m_BackEnd;
        std::vector<CapsuleStar*> m_CapsuleStars;
    public:
        CapsuleSpace();
        ~CapsuleSpace();
        void update(float);
};
#endif