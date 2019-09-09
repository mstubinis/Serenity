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
        EntityWrapper*  m_HullEntity;
        Map&            m_Map;
        uint            m_HealthPointsCurrent;
        uint            m_HealthPointsMax;
        uint            m_RechargeAmount;
        float           m_RechargeRate;
        float           m_RechargeTimer;
        float           m_CollisionTimer;

        void applyDamageDecal(const glm::vec3& impactNormal, const glm::vec3& impactLocation, const float& impactRadius, const bool forceHullFire = false);
    public:
        ShipSystemHull(Ship&, Map&, const uint health);
        ~ShipSystemHull();

        EntityWrapper* getEntity();

        void destroy();

        void receiveHit(const glm::vec3& impactNormal, const glm::vec3& impactLocation, const float& impactRadius, const float& maxTime, const uint damage, const bool forceHullFire = false);
        void receiveCollision(const glm::vec3& impactNormal, const glm::vec3& impactLocation, const float& impactRadius, const float damage);

        const uint getHealthCurrent() const;
        const uint getHealthMax() const;
        const float getHealthPercent() const; //returns percent from 0.0f to 1.0f

        void update(const double& dt);
};

#endif