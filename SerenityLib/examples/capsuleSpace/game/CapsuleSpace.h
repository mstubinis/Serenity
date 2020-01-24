#pragma once
#ifndef GAME_CAPSULE_SPACE_H
#define GAME_CAPSULE_SPACE_H

#include <core/engine/scene/Scene.h>
#include <ecs/Components.h>

class PointLight;
struct Handle;

struct CapsuleEndLogicFunctor;
struct CapsuleStarLogicFunctor;

class CapsuleSpace;
class Ship;

class CapsuleEnd final: public EntityWrapper {
    friend class  ::CapsuleSpace;
    friend struct ::CapsuleEndLogicFunctor;
    public:
        CapsuleEnd(float size,glm::vec3 pos, glm::vec3 color, CapsuleSpace* = nullptr);
        ~CapsuleEnd();
};
class CapsuleStar final : public EntityWrapper {
    friend class  ::CapsuleSpace;
    friend struct ::CapsuleStarLogicFunctor;
    private:
        PointLight* m_Light;
    public:
        CapsuleStar(float size,glm::vec3 pos, CapsuleSpace* = nullptr,bool=true);
        ~CapsuleStar();
};
class CapsuleTunnel final : public EntityWrapper {
    friend class  ::CapsuleSpace;
    private:
        float m_TunnelRadius;
    public:
        CapsuleTunnel(float tunnelRadius, Handle& material, CapsuleSpace* = nullptr);
        ~CapsuleTunnel();
        float getTunnelRadius(){ return m_TunnelRadius; }
};
class CapsuleRibbon final : public EntityWrapper {
    friend class  ::CapsuleSpace;
    private:
        float m_TunnelRadius;
    public:
        CapsuleRibbon(float tunnelRadius, Handle& mesh,Handle& material, CapsuleSpace* = nullptr);
        ~CapsuleRibbon();

        float getTunnelRadius(){ return m_TunnelRadius; }
};
class CapsuleSpace final: public Scene {
    friend class ::CapsuleEnd;
    friend class ::CapsuleStar;
    friend class ::CapsuleTunnel;
    friend class ::CapsuleRibbon;
    private:
        float m_Timer;
        Ship* player;
        CapsuleTunnel* m_TunnelA;
        CapsuleTunnel* m_TunnelB;
        CapsuleRibbon* m_RibbonA;
        CapsuleRibbon* m_RibbonB;
        CapsuleEnd* m_FrontEnd;
        CapsuleEnd* m_BackEnd;
        std::vector<CapsuleStar*> m_CapsuleStars;
    public:
        std::vector<EntityWrapper*> m_Objects;

        CapsuleSpace();
        virtual ~CapsuleSpace();
        void update(const double& dt);

        Ship* getPlayer() { return player; }
        void setPlayer(Ship* p) { player = p; }
};
#endif