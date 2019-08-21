#pragma once
#ifndef GAME_SHIP_SYSTEM_HULL_H
#define GAME_SHIP_SYSTEM_HULL_H

#include "ShipSystemBaseClass.h"
#include <glm/vec3.hpp>
#include <ecs/Entity.h>

#define HULL_TO_HULL_COLLISION_DELAY 1.5

class Map;
class ShipSystemHull final : public ShipSystem {
    friend class Ship;
    private:
        EntityWrapper   m_HullEntity;
        uint            m_HealthPointsCurrent;
        uint            m_HealthPointsMax;
        uint            m_RechargeAmount;
        float           m_RechargeRate;
        float           m_RechargeTimer;
        float           m_CollisionTimer;
    public:
        ShipSystemHull(Ship&, Map&, const uint health);
        ~ShipSystemHull();

        void receiveHit(const glm::vec3& impactLocation, const float& impactRadius, const float& maxTime, const uint damage);
        void receiveCollision(const glm::vec3& impactLocation, const float damage);

        const uint getHealthCurrent() const;
        const uint getHealthMax() const;
        const float getHealthPercent() const; //returns percent from 0.0f to 1.0f

        void update(const double& dt);
};

#endif