#pragma once
#ifndef GAME_SHIP_SYSTEM_PITCH_THRUSTERS_H
#define GAME_SHIP_SYSTEM_PITCH_THRUSTERS_H

#include "ShipSystemBaseClass.h"

class ShipSystemPitchThrusters final : public ShipSystem {
    friend class Ship;
    public:
        ShipSystemPitchThrusters(Ship&);
        ~ShipSystemPitchThrusters();

        void update(const double& dt);
};

#endif