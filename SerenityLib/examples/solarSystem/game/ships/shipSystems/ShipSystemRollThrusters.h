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

        ShipSystemRollThrusters(const ShipSystemRollThrusters& other)                = delete;
        ShipSystemRollThrusters& operator=(const ShipSystemRollThrusters& other)     = delete;
        ShipSystemRollThrusters(ShipSystemRollThrusters&& other) noexcept            = delete;
        ShipSystemRollThrusters& operator=(ShipSystemRollThrusters&& other) noexcept = delete;

        void update(const double& dt);
};

#endif