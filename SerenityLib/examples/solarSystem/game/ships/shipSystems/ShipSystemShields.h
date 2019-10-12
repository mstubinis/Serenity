#pragma once
#ifndef GAME_SHIP_SYSTEM_SHIELDS_H
#define GAME_SHIP_SYSTEM_SHIELDS_H

#include "ShipSystemBaseClass.h"
#include <ecs/Entity.h>
#include <core/engine/utils/Utils.h>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

#define MAX_IMPACT_POINTS 64
#define SHIELD_SCALE_FACTOR 1.37f

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
    struct ShieldSide final { enum Side {
        Forward = 0,
        Aft = 1,
        Port = 2,
        Starboard = 3,
        Dorsal = 4,
        Ventral = 5,
        Err,
    };};
    struct Plane final {
        glm::vec3 a, b, c;
        Plane();
        Plane(const glm::vec3& _a, const glm::vec3& _b, const glm::vec3& _c);
        const float CalcSide(const glm::vec3& point, const glm::quat& rot);
    };
    struct Pyramid final {
        std::vector<Plane> planes;
        Pyramid();
        Pyramid(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec3& p5);
        const bool isInside(const glm::vec3& point, const glm::quat& rot);
    };
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
        std::vector<Pyramid>          m_Pyramids;
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

        ShieldSide::Side getImpactSide(const glm::vec3& impactLocation);
        const std::string getImpactSideString(const ShieldSide::Side);
};

#endif