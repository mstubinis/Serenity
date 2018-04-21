#pragma once
#ifndef CAPSULE_SPACE_H
#define CAPSULE_SPACE_H

#include "SolarSystem.h"
#include "Components.h"

class PointLight;
struct Handle;

class CapsuleEnd final: public Entity{
    public:
        ComponentBasicBody* m_Body;
        CapsuleEnd(float size,glm::vec3 pos, glm::vec3 color, Scene* = nullptr);
        ~CapsuleEnd();
        void update(const float& dt);
};
class CapsuleStar final: public Entity{
    private:
        PointLight* m_Light;
        ComponentBasicBody* m_Body;
    public:
        CapsuleStar(float size,glm::vec3 pos, Scene* = nullptr,bool=true);
        ~CapsuleStar();
        void update(const float& dt);
};
class CapsuleTunnel final: public Entity{
    private:
        float m_TunnelRadius;
    public:
        ComponentBasicBody* m_Body;
        CapsuleTunnel(float tunnelRadius, Handle& material, Scene* = nullptr);
        ~CapsuleTunnel();
        float getTunnelRadius(){ return m_TunnelRadius; }
};
class CapsuleRibbon final: public Entity{
    private:
        float m_TunnelRadius;
    public:
        ComponentBasicBody* m_Body;
        CapsuleRibbon(float tunnelRadius, Handle& material, Scene* = nullptr);
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
        void update(const float& dt);
};
#endif