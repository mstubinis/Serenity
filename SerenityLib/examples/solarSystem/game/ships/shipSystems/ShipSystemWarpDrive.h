#pragma once
#ifndef GAME_SHIP_SYSTEM_WARP_DRIVE_H
#define GAME_SHIP_SYSTEM_WARP_DRIVE_H

#include "ShipSystemBaseClass.h"

class ShipSystemWarpDrive final : public ShipSystem {
    friend class Ship;
    public:
        ShipSystemWarpDrive(Ship&);
        ~ShipSystemWarpDrive();

        void update(const double& dt);
};

#endif