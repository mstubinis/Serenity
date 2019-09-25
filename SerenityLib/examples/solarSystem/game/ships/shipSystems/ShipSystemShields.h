#pragma once
#ifndef GAME_SHIP_SYSTEM_SHIELDS_H
#define GAME_SHIP_SYSTEM_SHIELDS_H

#include "ShipSystemBaseClass.h"
#include <ecs/Entity.h>
#include <core/engine/utils/Utils.h>
#include <glm/vec3.hpp>
#include <vector>

#define MAX_IMPACT_POINTS 64

struct ShieldInstanceBindFunctor;
struct ShieldInstanceUnbindFunctor;
struct ShipSystemShieldsImpactPoint final {
    bool       active;
    glm::vec3  impactLocation;
    float      impactRadius;
    float      currentTime;
    float      maxTime;
    uint       indexInArray;

    ShipSystemShieldsImpactPoint();

    void impact(const glm::vec3& _impactLocation, const float& _impactRadius, const float& _maxTime, std::vector<uint>& freelist);
    void update(const float& dt, std::vector<uint>& freelist);
};


class Map;
class ShipSystemShields final : public ShipSystem {
    friend struct ShieldInstanceBindFunctor;
    friend struct ShieldInstanceUnbindFunctor;
    friend class  Ship;
    private:
        ShipSystemShieldsImpactPoint  m_ImpactPoints[MAX_IMPACT_POINTS];
        std::vector<uint>             m_ImpactPointsFreelist;
        Entity                        m_ShieldEntity;
        float                         m_HealthPointsCurrent;
        float                         m_HealthPointsMax;
        //float                       m_TimeSinceLastHit;
        //float                       m_RechargeActivation;
        float                         m_RechargeAmount;
        float                         m_RechargeRate;
        float                         m_RechargeTimer;
        bool                          m_ShieldsAreUp;
    public:
        ShipSystemShields(Ship&, Map&, const float health);
        ~ShipSystemShields();

        Entity getEntity();

        void destroy();

        void update(const double& dt);

        const float getHealthCurrent() const;
        const float getHealthMax() const;
        const float getHealthPercent() const; //returns percent from 0.0f to 1.0f
        const bool shieldsAreUp() const;

        void turnOffShields();
        void turnOnShields();

        void receiveHit(const glm::vec3& impactNormal, const glm::vec3& impactLocation, const float& impactRadius, const float& maxTime, const float damage, const bool doImpactGraphic = true);
        void addShieldImpact(const glm::vec3& impactLocation, const float& impactRadius, const float& maxTime);
};

#endif