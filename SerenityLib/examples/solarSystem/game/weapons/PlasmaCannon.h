#pragma once
#ifndef GAME_WEAPON_PLASMA_CANNON_H
#define GAME_WEAPON_PLASMA_CANNON_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>
#include <ecs/Entity.h>

class  Map;
class  Ship;
class  PlasmaCannon;
class  PointLight;

struct PlasmaCannonCollisionFunctor;
struct PlasmaCannonInstanceBindFunctor;
struct PlasmaCannonInstanceUnbindFunctor;
struct PlasmaCannonTailInstanceBindFunctor;
struct PlasmaCannonTailInstanceUnbindFunctor;

struct PlasmaCannonProjectile final {
    Entity entity;
    PointLight* light;
    float currentTime;
    float maxTime;
    bool active;
    PlasmaCannonProjectile(PlasmaCannon&, Map& map, const glm::vec3& position, const glm::vec3& forward);
    ~PlasmaCannonProjectile();
    void update(const double& dt);
    void destroy();
};

class PlasmaCannon final : public PrimaryWeaponCannon {
    friend struct PlasmaCannonCollisionFunctor;
    friend struct PlasmaCannonInstanceBindFunctor;
    friend struct PlasmaCannonInstanceUnbindFunctor;
    friend struct PlasmaCannonTailInstanceBindFunctor;
    friend struct PlasmaCannonTailInstanceUnbindFunctor;
    private:
        Map& m_Map;
        std::vector<PlasmaCannonProjectile*> m_ActiveProjectiles;
    public:
        PlasmaCannon(Ship&, Map&, const glm::vec3& position, const glm::vec3& forward, const float& arc, const uint& maxCharges = 5, const uint & damage = 100, const float& _rechargePerRound = 1.0f, const float& _impactRadius = 1.5f, const float& _impactTime = 1.8f, const float& _travelSpeed = 50.5f);
        ~PlasmaCannon();

        bool fire();
        void forceFire();
        void update(const double& dt);
};

#endif