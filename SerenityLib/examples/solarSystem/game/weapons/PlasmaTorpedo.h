#pragma once
#ifndef GAME_WEAPON_PLASMA_TORPEDO_H
#define GAME_WEAPON_PLASMA_TORPEDO_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>

class  PlasmaTorpedo;
struct PlasmaTorpedoCollisionFunctor;
struct PlasmaTorpedoInstanceCoreBindFunctor;
struct PlasmaTorpedoInstanceCoreUnbindFunctor;
struct PlasmaTorpedoInstanceGlowBindFunctor;
struct PlasmaTorpedoInstanceGlowUnbindFunctor;
struct PlasmaTorpedoFlareInstanceBindFunctor;
struct PlasmaTorpedoFlareInstanceUnbindFunctor;

struct PlasmaTorpedoFlare final {
    glm::vec3 spin;
    glm::quat start;
    PlasmaTorpedoFlare() {
        start   = glm::quat();
        start.w = 1.0f;
        spin    = glm::vec3(0.0f);
    }
    PlasmaTorpedoFlare(glm::vec3& _spin) {
        start   = glm::quat();
        start.w = 1.0f;
        spin    = _spin;
    }
};
struct PlasmaTorpedoProjectile final : public SecondaryWeaponTorpedoProjectile {
    PlasmaTorpedo&                   torpedo;
    std::vector<PlasmaTorpedoFlare>  flares;

    PlasmaTorpedoProjectile(EntityWrapper* target, PlasmaTorpedo&, Map& map, const glm_vec3& final_world_position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos);
    ~PlasmaTorpedoProjectile();
    void update(const double& dt);
};


class PlasmaTorpedo final : public SecondaryWeaponTorpedo {
    friend struct PlasmaTorpedoCollisionFunctor;
    friend struct PlasmaTorpedoInstanceCoreBindFunctor;
    friend struct PlasmaTorpedoInstanceCoreUnbindFunctor;
    friend struct PlasmaTorpedoInstanceGlowBindFunctor;
    friend struct PlasmaTorpedoInstanceGlowUnbindFunctor;
    friend struct PlasmaTorpedoFlareInstanceBindFunctor;
    friend struct PlasmaTorpedoFlareInstanceUnbindFunctor;
    public:
        PlasmaTorpedo(
            Ship&,
            Map&,
            const glm_vec3& position,
            const glm_vec3& forward,
            const float& arc,
            const uint& maxCharges = 1,
            const float& damage = 6650.0f,
            const float& _rechargePerRound = 15.0f,
            const float& _impactRadius = 6.8f,
            const float& _impactTime = 4.0f,
            const float& _travelSpeed = 16.5f,
            const float& _volume = 100.0f,
            const float& _rotAngleSpeed = 0.5f,
            const unsigned int& _modelIndex = 0
        );
        ~PlasmaTorpedo();

        void update(const double& dt);
};

#endif