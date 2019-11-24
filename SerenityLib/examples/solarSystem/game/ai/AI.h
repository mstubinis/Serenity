#pragma once
#ifndef GAME_AI_H
#define GAME_AI_H

#include "AIIncludes.h"
#include <string>

class FireAtWill;
class Ship;
class Map;
class ShipSystemSensors;
class ShipSystemWeapons;
class ThreatTable;
class AI {
    protected:
        AIType::Type   m_Type;
        FireAtWill*    m_FireAtWill;
        ThreatTable*   m_Threat;
    public:
        AI(const AIType::Type type);
        virtual ~AI();

        const AIType::Type& getType() const;

        virtual void update(const double& dt);

        void installFireAtWill(AIType::Type&, Ship&, Map&, ShipSystemSensors&, ShipSystemWeapons&);
        void installThreatTable(Map& map);

        FireAtWill* getFireAtWill();
        ThreatTable* getThreatTable();
        /*
        const std::string serialize() const;
        void deserialize(const std::string& input);
        */
};

#endif