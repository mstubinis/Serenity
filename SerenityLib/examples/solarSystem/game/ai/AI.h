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
class AI {
    private:
        AIType::Type  m_Type;
        FireAtWill*   m_FireAtWill;
    public:
        AI(const AIType::Type type);
        ~AI();

        const AIType::Type& getType() const;

        virtual void update(const double& dt);

        void installFireAtWill(Ship&, Map&, ShipSystemSensors&, ShipSystemWeapons&);

        FireAtWill* getFireAtWill();
        /*
        const std::string serialize() const;
        void deserialize(const std::string& input);
        */
};

#endif