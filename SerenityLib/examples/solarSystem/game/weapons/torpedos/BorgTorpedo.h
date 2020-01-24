#pragma once
#ifndef GAME_WEAPON_BORG_TORPEDO_H
#define GAME_WEAPON_BORG_TORPEDO_H

#include "../../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>

class  BorgTorpedo;
struct BorgTorpedoCollisionFunctor;
struct BorgTorpedoInstanceCoreBindFunctor;
struct BorgTorpedoInstanceCoreUnbindFunctor;
struct BorgTorpedoInstanceGlowBindFunctor;
struct BorgTorpedoInstanceGlowUnbindFunctor;
struct BorgTorpedoFlareInstanceBindFunctor;
struct BorgTorpedoFlareInstanceUnbindFunctor;

struct BorgTorpedoFlare final {
    glm::vec3 spin;
    glm::quat start;
    BorgTorpedoFlare() {
        start = glm::quat();
        start.w = 1.0f;
        spin = glm::vec3(0.0f);
    }
    BorgTorpedoFlare(glm::vec3& _spin) {
        start = glm::quat();
        start.w = 1.0f;
        spin = _spin;
    }
};
struct BorgTorpedoProjectile final : public SecondaryWeaponTorpedoProjectile {
    BorgTorpedo& torpedo;
    std::vector<BorgTorpedoFlare>   flares;
    BorgTorpedoProjectile(EntityWrapper* target, BorgTorpedo&, Map& map, const glm_vec3& final_world_position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos);
    ~BorgTorpedoProjectile();
    void update(const double& dt);
};


class BorgTorpedo final : public SecondaryWeaponTorpedo {
    friend struct BorgTorpedoCollisionFunctor;
    friend struct BorgTorpedoInstanceCoreBindFunctor;
    friend struct BorgTorpedoInstanceCoreUnbindFunctor;
    friend struct BorgTorpedoInstanceGlowBindFunctor;
    friend struct BorgTorpedoInstanceGlowUnbindFunctor;
    friend struct BorgTorpedoFlareInstanceBindFunctor;
    friend struct BorgTorpedoFlareInstanceUnbindFunctor;
    public:
        BorgTorpedo(
            Ship&,
            Map&,
            const glm_vec3& position,
            const glm_vec3& forward,
            const float& arc,
            const uint& maxCharges = 1,
            const float& damage = 10000.0f,
            const float& _rechargePerRound = 15.0f,
            const float& _impactRadius = 5.5f,
            const float& _impactTime = 3.1f,
            const float& _travelSpeed = 16.5f,
            const float& _volume = 100.0f,
            const float& _rotAngleSpeed = 0.5f,
            const unsigned int& _modelIndex = 0
        );
        ~BorgTorpedo();

    void update(const double& dt);
};

#endif