#pragma once
#ifndef GAME_SHIP_SYSTEM_HULL_H
#define GAME_SHIP_SYSTEM_HULL_H

#include "ShipSystemBaseClass.h"
#include <glm/vec3.hpp>

class ShipSystemHull final : public ShipSystem {
    friend class Ship;
    private:
        uint    m_HealthPointsCurrent;
        uint    m_HealthPointsMax;
        uint    m_RechargeAmount;
        float   m_RechargeRate;
        float   m_RechargeTimer;
    public:
        ShipSystemHull(Ship&, const uint health);
        ~ShipSystemHull();

        void receiveHit(const glm::vec3& impactLocation, const float& impactRadius, const float& maxTime, const uint damage);

        const uint getHealthCurrent() const;
        const uint getHealthMax() const;
        const float getHealthPercent() const; //returns percent from 0.0f to 1.0f

        void update(const double& dt);
};

#endif