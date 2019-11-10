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
        Entity          m_HullEntity;
        Map&            m_Map;
        float           m_HealthPointsCurrent;
        float           m_HealthPointsMax;
        float           m_RechargeAmount;
        float           m_RechargeRate;
        float           m_RechargeTimer;

        void applyDamageDecal(const glm::vec3& impactNormal, const glm::vec3& impactLocation, const float& impactRadius, const bool forceHullFire = false);
    public:
        ShipSystemHull(Ship&, Map&, const float health, const float recharge_amount = 50.0f, const float recharge_rate = 5.0f);
        ~ShipSystemHull();

        Entity getEntity();

        void destroy();

        void receiveHit(const glm::vec3& impactNormal, const glm::vec3& impactLocation, const float& impactRadius, const float& maxTime, const float damage, const bool forceHullFire = false, const bool paint = true);
        void receiveCollisionDamage(const float damage);
        void receiveCollisionVisual(const glm::vec3& impactNormal, const glm::vec3& impactLocation, const float& impactRadius);

        const float getHealthCurrent() const;
        const float getHealthMax() const;
        const float getHealthPercent() const; //returns percent from 0.0f to 1.0f

        void restoreToFull();

        void update(const double& dt);
};

#endif