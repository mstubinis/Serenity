#pragma once
#ifndef GAME_AI_LOGIC_GUNNERY_BASE_CLASS_H
#define GAME_AI_LOGIC_GUNNERY_BASE_CLASS_H

#include "../AIIncludes.h"
#include <random>

class AI;
class ThreatTable;
class Ship;
class Map;
class ShipSystemSensors;
class ShipSystemWeapons;
class GunneryBaseClass {
    protected:
        double m_BeamTimer;
        double m_CannonTimer;
        double m_TorpedoTimer;

        std::mt19937         m_RandomDevice;

        AIType::Type&        m_AIType;
        AI&                  m_AI;
        ThreatTable&         m_ThreatTable;
        Ship&                m_Ship;
        Map&                 m_Map;
        ShipSystemWeapons&   m_Weapons;
        ShipSystemSensors&   m_Sensors;
    public:
        GunneryBaseClass(AIType::Type&, AI&, ThreatTable&, Ship&, Map&, ShipSystemSensors&, ShipSystemWeapons&);
        virtual ~GunneryBaseClass();

        virtual void update(const double& dt);
};

#endif