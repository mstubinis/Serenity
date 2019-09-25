#pragma once
#ifndef GAME_WEAPON_PLASMA_CANNON_H
#define GAME_WEAPON_PLASMA_CANNON_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>

class  PlasmaCannon;
struct PlasmaCannonCollisionFunctor;
struct PlasmaCannonInstanceBindFunctor;
struct PlasmaCannonInstanceUnbindFunctor;
struct PlasmaCannonTailInstanceBindFunctor;
struct PlasmaCannonTailInstanceUnbindFunctor;
struct PlasmaCannonOutlineInstanceBindFunctor;
struct PlasmaCannonOutlineInstanceUnbindFunctor;

struct PlasmaCannonProjectile final : public PrimaryWeaponCannonProjectile {
    PlasmaCannonProjectile(PlasmaCannon&, Map& map, const glm::vec3& position, const glm::vec3& forward, const int index);
    ~PlasmaCannonProjectile();
};

class PlasmaCannon final : public PrimaryWeaponCannon {
    friend struct PlasmaCannonCollisionFunctor;
    friend struct PlasmaCannonInstanceBindFunctor;
    friend struct PlasmaCannonInstanceUnbindFunctor;
    friend struct PlasmaCannonTailInstanceBindFunctor;
    friend struct PlasmaCannonTailInstanceUnbindFunctor;
    friend struct PlasmaCannonOutlineInstanceBindFunctor;
    friend struct PlasmaCannonOutlineInstanceUnbindFunctor;
    public:
        PlasmaCannon(
            Ship&,
            Map&,
            const glm::vec3& position,
            const glm::vec3& forward,
            const float& arc,
            const uint& maxCharges = 5,
            const float& damage = 100.0f,
            const float& _rechargePerRound = 1.0f,
            const float& _impactRadius = 2.5f,
            const float& _impactTime = 1.8f,
            const float& _travelSpeed = 50.5f,
            const float& _volume = 100.0f
        );
        ~PlasmaCannon();

        void update(const double& dt);
};

#endif