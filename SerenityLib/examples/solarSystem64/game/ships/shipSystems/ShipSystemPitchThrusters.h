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

        void update(const double& dt);
};

#endif