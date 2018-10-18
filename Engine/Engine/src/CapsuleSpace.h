#pragma once
#ifndef GAME_CAPSULE_SPACE_H
#define GAME_CAPSULE_SPACE_H

#include "SolarSystem.h"
#include "ecs/Components.h"

class PointLight;
struct Handle;

struct CapsuleEndLogicFunctor;
struct CapsuleStarLogicFunctor;

class CapsuleSpace;

class CapsuleEnd final{
    friend class  ::CapsuleSpace;
    friend struct ::CapsuleEndLogicFunctor;
    private:
        Entity m_Entity;
    public:
        CapsuleEnd(float size,glm::vec3 pos, glm::vec3 color, Scene* = nullptr);
        ~CapsuleEnd();
};
class CapsuleStar final{
    friend class  ::CapsuleSpace;
    friend struct ::CapsuleStarLogicFunctor;
    private:
        Entity m_Entity;
        PointLight* m_Light;
    public:
        CapsuleStar(float size,glm::vec3 pos, Scene* = nullptr,bool=true);
        ~CapsuleStar();
};
class CapsuleTunnel final{
    friend class  ::CapsuleSpace;
    private:
        Entity m_Entity;
        float m_TunnelRadius;
    public:
        CapsuleTunnel(float tunnelRadius, Handle& material, Scene* = nullptr);
        ~CapsuleTunnel();
        float getTunnelRadius(){ return m_TunnelRadius; }
};
class CapsuleRibbon final{
    friend class  ::CapsuleSpace;
    private:
        Entity m_Entity;
        float m_TunnelRadius;
    public:
        CapsuleRibbon(float tunnelRadius, Handle& mesh,Handle& material, Scene* = nullptr);
        ~CapsuleRibbon();

        float getTunnelRadius(){ return m_TunnelRadius; }
};
class CapsuleSpace final: public SolarSystem{
    friend class ::CapsuleEnd;
    friend class ::CapsuleStar;
    friend class ::CapsuleTunnel;
    friend class ::CapsuleRibbon;
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