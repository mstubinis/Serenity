#pragma once
#ifndef GAME_WEAPON_PULSE_PHASER_H
#define GAME_WEAPON_PULSE_PHASER_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>

class  PulsePhaser;

struct PulsePhaserCollisionFunctor;
struct PulsePhaserInstanceBindFunctor;
struct PulsePhaserInstanceUnbindFunctor;
struct PulsePhaserTailInstanceBindFunctor;
struct PulsePhaserTailInstanceUnbindFunctor;
struct PulsePhaserOutlineInstanceBindFunctor;
struct PulsePhaserOutlineInstanceUnbindFunctor;

struct PulsePhaserProjectile final: public PrimaryWeaponCannonProjectile {
    PulsePhaserProjectile(PulsePhaser&, Map& map, const glm::vec3& position, const glm::vec3& forward, const int index);
    ~PulsePhaserProjectile();
};

class PulsePhaser final: public PrimaryWeaponCannon{
    friend struct PulsePhaserCollisionFunctor;
    friend struct PulsePhaserInstanceBindFunctor;
    friend struct PulsePhaserInstanceUnbindFunctor;
    friend struct PulsePhaserTailInstanceBindFunctor;
    friend struct PulsePhaserTailInstanceUnbindFunctor;
    friend struct PulsePhaserOutlineInstanceBindFunctor;
    friend struct PulsePhaserOutlineInstanceUnbindFunctor;
    public:
        PulsePhaser(
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
        ~PulsePhaser();

        const bool fire();
        void update(const double& dt);
};

#endif