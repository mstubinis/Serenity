#pragma once
#ifndef GAME_SHIP_SYSTEM_SENSORS_H
#define GAME_SHIP_SYSTEM_SENSORS_H

#include "ShipSystemBaseClass.h"

class ShipSystemSensors final : public ShipSystem {
    friend class Ship;
    public:
        ShipSystemSensors(Ship&);
        ~ShipSystemSensors();

        void update(const double& dt);
};

#endif