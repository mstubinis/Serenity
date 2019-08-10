#pragma once
#ifndef GAME_SHIP_SYSTEM_MAIN_THRUSTERS_H
#define GAME_SHIP_SYSTEM_MAIN_THRUSTERS_H

#include "ShipSystemBaseClass.h"

class ShipSystemMainThrusters final : public ShipSystem {
    friend class Ship;
    public:
        ShipSystemMainThrusters(Ship&);
        ~ShipSystemMainThrusters();

        void update(const double& dt);
};

#endif