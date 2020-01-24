#pragma once
#ifndef GAME_WEAPON_DISRUPTOR_CANNON_H
#define GAME_WEAPON_DISRUPTOR_CANNON_H

#include "../../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>

class  DisruptorCannon;
struct DisruptorCannonCollisionFunctor;
struct DisruptorCannonInstanceBindFunctor;
struct DisruptorCannonInstanceUnbindFunctor;
struct DisruptorCannonTailInstanceBindFunctor;
struct DisruptorCannonTailInstanceUnbindFunctor;
struct DisruptorCannonOutlineInstanceBindFunctor;
struct DisruptorCannonOutlineInstanceUnbindFunctor;

struct DisruptorCannonProjectile final : public PrimaryWeaponCannonProjectile {
    DisruptorCannonProjectile(EntityWrapper* target, DisruptorCannon&, Map& map, const glm_vec3& final_world_position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos);
    ~DisruptorCannonProjectile();
};

class DisruptorCannon final : public PrimaryWeaponCannon {
    friend struct DisruptorCannonCollisionFunctor;
    friend struct DisruptorCannonInstanceBindFunctor;
    friend struct DisruptorCannonInstanceUnbindFunctor;
    friend struct DisruptorCannonTailInstanceBindFunctor;
    friend struct DisruptorCannonTailInstanceUnbindFunctor;
    friend struct DisruptorCannonOutlineInstanceBindFunctor;
    friend struct DisruptorCannonOutlineInstanceUnbindFunctor;
    public:
        DisruptorCannon(
            Ship&,
            Map&,
            const glm_vec3& position,
            const glm_vec3& forward,
            const float& arc,
            const uint& maxCharges = 5,
            const float& damage = 250.0f,
            const float& _rechargePerRound = 1.0f,
            const float& _impactRadius = 2.5f,
            const float& _impactTime = 1.8f,
            const float& _travelSpeed = 40.5f,
            const float& _volume = 100.0f,
            const unsigned int& _modelIndex = 0
        );
        ~DisruptorCannon();

        void update(const double& dt);
};

#endif