#pragma once
#ifndef GAME_SHIP_SYSTEM_SENSORS_H
#define GAME_SHIP_SYSTEM_SENSORS_H

#include "ShipSystemBaseClass.h"
class Map;
class ShipSystemSensors final : public ShipSystem {
    friend class Ship;
    private:
        Map& m_Map;
    public:
        ShipSystemSensors(Ship&, Map&);
        ~ShipSystemSensors();

        void update(const double& dt);
        void render();
};

#endif