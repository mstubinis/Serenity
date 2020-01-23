#pragma once
#ifndef GAME_SHIP_SYSTEM_YAW_THRUSTERS_H
#define GAME_SHIP_SYSTEM_YAW_THRUSTERS_H

#include "ShipSystemBaseClass.h"

class ShipSystemYawThrusters final : public ShipSystem {
    friend class Ship;
    private:
        float m_AdditionalStrength;
    public:
        ShipSystemYawThrusters(Ship&, const float additional_strength = 1.0f);
        ~ShipSystemYawThrusters();

        ShipSystemYawThrusters(const ShipSystemYawThrusters& other)                = delete;
        ShipSystemYawThrusters& operator=(const ShipSystemYawThrusters& other)     = delete;
        ShipSystemYawThrusters(ShipSystemYawThrusters&& other) noexcept            = delete;
        ShipSystemYawThrusters& operator=(ShipSystemYawThrusters&& other) noexcept = delete;

        void update(const double& dt);
};

#endif