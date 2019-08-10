#pragma once
#ifndef GAME_SHIP_SYSTEM_ROLL_THRUSTERS_H
#define GAME_SHIP_SYSTEM_ROLL_THRUSTERS_H

#include "ShipSystemBaseClass.h"

class ShipSystemRollThrusters final : public ShipSystem {
    friend class Ship;
    public:
        ShipSystemRollThrusters(Ship&);
        ~ShipSystemRollThrusters();

        void update(const double& dt);
};

#endif