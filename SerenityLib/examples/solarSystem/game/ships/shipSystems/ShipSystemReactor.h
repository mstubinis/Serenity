#pragma once
#ifndef GAME_SHIP_SYSTEM_REACTOR_H
#define GAME_SHIP_SYSTEM_REACTOR_H

#include "ShipSystemBaseClass.h"

class Ship;
class ShipSystemReactor final : public ShipSystem {
    friend class Ship;
    private:
        float m_TotalPowerMax;
        float m_TotalPower;
    public:
        ShipSystemReactor(Ship&, const float maxPower, const float currentPower = -1);
        ~ShipSystemReactor();

        ShipSystemReactor(const ShipSystemReactor& other)                = delete;
        ShipSystemReactor& operator=(const ShipSystemReactor& other)     = delete;
        ShipSystemReactor(ShipSystemReactor&& other) noexcept            = delete;
        ShipSystemReactor& operator=(ShipSystemReactor&& other) noexcept = delete;

        void update(const double& dt);
};

#endif