#pragma once
#ifndef GAME_SHIP_SYSTEM_PITCH_THRUSTERS_H
#define GAME_SHIP_SYSTEM_PITCH_THRUSTERS_H

#include "ShipSystemBaseClass.h"

class ShipSystemPitchThrusters final : public ShipSystem {
    friend class Ship;
    private:
        float m_AdditionalStrength;
    public:
        ShipSystemPitchThrusters(Ship&, const float additional_strength = 1.0f);
        ~ShipSystemPitchThrusters();

        ShipSystemPitchThrusters(const ShipSystemPitchThrusters& other)                = delete;
        ShipSystemPitchThrusters& operator=(const ShipSystemPitchThrusters& other)     = delete;
        ShipSystemPitchThrusters(ShipSystemPitchThrusters&& other) noexcept            = delete;
        ShipSystemPitchThrusters& operator=(ShipSystemPitchThrusters&& other) noexcept = delete;

        void update(const double& dt);
};

#endif