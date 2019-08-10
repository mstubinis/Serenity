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
    friend class  Ship;
    private:
        ShipSystemShieldsImpactPoint  m_ImpactPoints[MAX_IMPACT_POINTS];
        std::vector<uint>             m_ImpactPointsFreelist;
        EntityWrapper                 m_ShieldEntity;
    public:
        ShipSystemShields(Ship&, Map*);
        ~ShipSystemShields();

        void update(const double& dt);

        void receiveHit(const glm::vec3& impactLocation, const float& impactRadius, const float& maxTime, const float& damage);
};

#endif