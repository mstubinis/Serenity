#pragma once
#ifndef GAME_WEAPON_PHOTON_TORPEDO_H
#define GAME_WEAPON_PHOTON_TORPEDO_H

#include "../../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>

class  PhotonTorpedo;
struct PhotonTorpedoCollisionFunctor;
struct PhotonTorpedoInstanceCoreBindFunctor;
struct PhotonTorpedoInstanceCoreUnbindFunctor;
struct PhotonTorpedoInstanceGlowBindFunctor;
struct PhotonTorpedoInstanceGlowUnbindFunctor;
struct PhotonTorpedoFlareInstanceBindFunctor;
struct PhotonTorpedoFlareInstanceUnbindFunctor;

struct PhotonTorpedoFlare final {
    glm::vec3 spin;
    glm::quat start;
    PhotonTorpedoFlare() {
        start   = glm::quat();
        start.w = 1.0f;
        spin    = glm::vec3(0.0f);
    }
    PhotonTorpedoFlare(glm::vec3& _spin) {
        start   = glm::quat();
        start.w = 1.0f;
        spin    = _spin;
    }
};
struct PhotonTorpedoProjectile final : public SecondaryWeaponTorpedoProjectile {
    PhotonTorpedo&                    torpedo;
    std::vector<PhotonTorpedoFlare>   flares;
    PhotonTorpedoProjectile(EntityWrapper* target, PhotonTorpedo&, Map& map, const glm_vec3& final_world_position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos);
    ~PhotonTorpedoProjectile();
    void update(const double& dt);
};


class PhotonTorpedo final : public SecondaryWeaponTorpedo {
    friend struct PhotonTorpedoCollisionFunctor;
    friend struct PhotonTorpedoInstanceCoreBindFunctor;
    friend struct PhotonTorpedoInstanceCoreUnbindFunctor;
    friend struct PhotonTorpedoInstanceGlowBindFunctor;
    friend struct PhotonTorpedoInstanceGlowUnbindFunctor;
    friend struct PhotonTorpedoFlareInstanceBindFunctor;
    friend struct PhotonTorpedoFlareInstanceUnbindFunctor;
    public:
        PhotonTorpedo(
            Ship&,
            Map&,
            const glm_vec3& position,
            const glm_vec3& forward,
            const float& arc,
            const uint& maxCharges = 1,
            const float& damage = 6500.0f,
            const float& _rechargePerRound = 15.0f,
            const float& _impactRadius = 5.5f,
            const float& _impactTime = 3.1f,
            const float& _travelSpeed = 16.5f,
            const float& _volume = 100.0f,
            const float& _rotAngleSpeed = 0.5f,
            const unsigned int& _modelIndex = 0
        );
        ~PhotonTorpedo();

        void update(const double& dt);
};

#endif