#pragma once
#ifndef GAME_CAPSULE_SPACE_H_INCLUDE_GUARD
#define GAME_CAPSULE_SPACE_H_INCLUDE_GUARD

#include "SolarSystem.h"
#include "core/Components.h"

class PointLight;
struct Handle;

class CapsuleEnd final: public OLD_Entity{
    public:
        OLD_ComponentBody* m_Body;
        CapsuleEnd(float size,glm::vec3 pos, glm::vec3 color, Scene* = nullptr);
        ~CapsuleEnd();
        void update(const float& dt);
};
class CapsuleStar final: public OLD_Entity{
    private:
        PointLight* m_Light;
        OLD_ComponentBody* m_Body;
    public:
        CapsuleStar(float size,glm::vec3 pos, Scene* = nullptr,bool=true);
        ~CapsuleStar();
        void update(const float& dt);
};
class CapsuleTunnel final: public OLD_Entity{
    private:
        float m_TunnelRadius;
    public:
        OLD_ComponentBody* m_Body;
        CapsuleTunnel(float tunnelRadius, Handle& material, Scene* = nullptr);
        ~CapsuleTunnel();
        float getTunnelRadius(){ return m_TunnelRadius; }
};
class CapsuleRibbon final: public OLD_Entity{
    private:
        float m_TunnelRadius;
    public:
        OLD_ComponentBody* m_Body;
        CapsuleRibbon(float tunnelRadius, Handle& mesh,Handle& material, Scene* = nullptr);
        ~CapsuleRibbon();

        float getTunnelRadius(){ return m_TunnelRadius; }
};
class CapsuleSpace final: public SolarSystem{
    private:
        float m_Timer;
        CapsuleTunnel* m_TunnelA;
        CapsuleTunnel* m_TunnelB;
        CapsuleRibbon* m_RibbonA;
        CapsuleRibbon* m_RibbonB;
        CapsuleEnd* m_FrontEnd;
        CapsuleEnd* m_BackEnd;
        std::vector<CapsuleStar*> m_CapsuleStars;
    public:
        CapsuleSpace();
        ~CapsuleSpace();
        void update(const float& dt);
};
#endif