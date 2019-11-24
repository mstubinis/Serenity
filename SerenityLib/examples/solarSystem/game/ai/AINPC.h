#pragma once
#ifndef GAME_AI_NPC_H
#define GAME_AI_NPC_H

#include "AIIncludes.h"
#include "AI.h"

class GunneryBaseClass;
class AINPC: public AI {
    protected:
        GunneryBaseClass* m_Gunnery;
    public:
        AINPC(const AIType::Type type);
        virtual ~AINPC();


        virtual void installGunneryLogic(AIType::Type&, ThreatTable&, Ship&, Map&, ShipSystemSensors&, ShipSystemWeapons&);

        virtual void update(const double& dt);

        GunneryBaseClass* getGunneryLogic();
};

#endif