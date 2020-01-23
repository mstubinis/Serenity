#pragma once
#ifndef GAME_SHIP_SYSTEM_WARP_DRIVE_H
#define GAME_SHIP_SYSTEM_WARP_DRIVE_H

#include "ShipSystemBaseClass.h"

class ShipSystemWarpDrive final : public ShipSystem {
    friend class Ship;
    public:
        ShipSystemWarpDrive(Ship&);
        ~ShipSystemWarpDrive();

        ShipSystemWarpDrive(const ShipSystemWarpDrive& other)                = delete;
        ShipSystemWarpDrive& operator=(const ShipSystemWarpDrive& other)     = delete;
        ShipSystemWarpDrive(ShipSystemWarpDrive&& other) noexcept            = delete;
        ShipSystemWarpDrive& operator=(ShipSystemWarpDrive&& other) noexcept = delete;

        void update(const double& dt);
};

#endif