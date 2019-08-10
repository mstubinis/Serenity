#pragma once
#ifndef GAME_SHIP_SYSTEM_YAW_THRUSTERS_H
#define GAME_SHIP_SYSTEM_YAW_THRUSTERS_H

#include "ShipSystemBaseClass.h"

class ShipSystemYawThrusters final : public ShipSystem {
    friend class Ship;
    public:
        ShipSystemYawThrusters(Ship&);
        ~ShipSystemYawThrusters();

        void update(const double& dt);
};

#endif