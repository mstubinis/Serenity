#pragma once
#ifndef GAME_SHIP_SYSTEM_ROLL_THRUSTERS_H
#define GAME_SHIP_SYSTEM_ROLL_THRUSTERS_H

#include "ShipSystemBaseClass.h"

class ShipSystemRollThrusters final : public ShipSystem {
    friend class Ship;
    private:
        float m_AdditionalStrength;
    public:
        ShipSystemRollThrusters(Ship&, const float additional_strength = 1.0f);
        ~ShipSystemRollThrusters();

        void update(const double& dt);
};

#endif