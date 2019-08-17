#pragma once
#ifndef GAME_SHIP_SYSTEM_SHIELDS_H
#define GAME_SHIP_SYSTEM_SHIELDS_H

#include "ShipSystemBaseClass.h"
#include <ecs/Entity.h>
#include <core/engine/utils/Utils.h>
#include <glm/vec3.hpp>
#include <vector>

#define MAX_IMPACT_POINTS 60

struct ShieldInstanceBindFunctor;
struct ShieldInstanceUnbindFunctor;
struct ShipSystemShieldsImpactPoint final {
    bool       active;
    glm::vec3  impactLocation;
    float      impactRadius;
    float      currentTime;
    float      maxTime;
    uint       indexInArray;
    ShipSystemShieldsImpactPoint() {
        active = false;
        impactLocation = glm::vec3(0.0f);
        impactRadius = maxTime = currentTime = 0.0f;
        indexInArray = 0;
    }
    void impact(const glm::vec3& _impactLocation, const float& _impactRadius, const float& _maxTime, std::vector<uint>& freelist) {
        active = true;
        impactLocation = _impactLocation;
        impactRadius = _impactRadius;
        currentTime = 0.0f;
        maxTime = _maxTime;

        removeFromVector(freelist, indexInArray);
    }

    void update(const float& dt, std::vector<uint>& freelist) {
        if (active) {
            currentTime += dt;
            if (currentTime >= maxTime) {
                active = false;
                currentTime = 0.0f;
                freelist.push_back(indexInArray);
            }
        }
    }
};


class Map;
class ShipSystemShields final : public ShipSystem {
    friend struct ShieldInstanceBindFunctor;
    friend struct ShieldInstanceUnbindFunctor;
    friend class  Ship;
    private:
        ShipSystemShieldsImpactPoint  m_ImpactPoints[MAX_IMPACT_POINTS];
        std::vector<uint>             m_ImpactPointsFreelist;
        EntityWrapper                 m_ShieldEntity;
        uint                          m_HealthPointsCurrent;
        uint                          m_HealthPointsMax;
        //float                       m_TimeSinceLastHit;
        //float                       m_RechargeActivation;
        uint                          m_RechargeAmount;
        float                         m_RechargeRate;
        float                         m_RechargeTimer;
        bool                          m_ShieldsAreUp;
    public:
        ShipSystemShields(Ship&, Map*, const uint health);
        ~ShipSystemShields();

        void destroy();

        void update(const double& dt);

        const uint getHealthCurrent() const;
        const uint getHealthMax() const;
        const float getHealthPercent() const; //returns percent from 0.0f to 1.0f
        const bool shieldsAreUp() const;

        void turnOffShields();
        void turnOnShields();

        void receiveHit(const glm::vec3& impactLocation, const float& impactRadius, const float& maxTime, const uint damage);
};

#endif