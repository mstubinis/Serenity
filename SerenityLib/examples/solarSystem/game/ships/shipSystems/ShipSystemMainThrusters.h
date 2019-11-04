#pragma once
#ifndef GAME_SHIP_SYSTEM_MAIN_THRUSTERS_H
#define GAME_SHIP_SYSTEM_MAIN_THRUSTERS_H

#include "ShipSystemBaseClass.h"

class ShipSystemMainThrusters final : public ShipSystem {
    friend class Ship;
    private:
        float m_AdditionalForceStrength;
    public:
        ShipSystemMainThrusters(Ship&, const float additional_force_strength = 1.0f);
        ~ShipSystemMainThrusters();

        void update(const double& dt);
};

#endif