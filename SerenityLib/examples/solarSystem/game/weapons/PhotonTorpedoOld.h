#pragma once
#ifndef GAME_WEAPON_PHOTON_TORPEDO_OLD_H
#define GAME_WEAPON_PHOTON_TORPEDO_OLD_H


#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>

class  PhotonTorpedoOld;
struct PhotonTorpedoOldCollisionFunctor;
struct PhotonTorpedoOldInstanceCoreBindFunctor;
struct PhotonTorpedoOldInstanceCoreUnbindFunctor;
struct PhotonTorpedoOldInstanceGlowBindFunctor;
struct PhotonTorpedoOldInstanceGlowUnbindFunctor;
struct PhotonTorpedoOldFlareInstanceBindFunctor;
struct PhotonTorpedoOldFlareInstanceUnbindFunctor;

struct PhotonTorpedoOldFlare final {
    glm::vec3 spin;
    glm::quat start;
    PhotonTorpedoOldFlare() {
        start   = glm::quat();
        start.w = 1.0f;
        spin    = glm::vec3(0.0f);
    }
    PhotonTorpedoOldFlare(glm::vec3& _spin) {
        start   = glm::quat();
        start.w = 1.0f;
        spin    = _spin;
    }
};
struct PhotonTorpedoOldProjectile final : public SecondaryWeaponTorpedoProjectile {
    PhotonTorpedoOld&                   torpedo;
    std::vector<PhotonTorpedoOldFlare>  flares;
    PhotonTorpedoOldProjectile(PhotonTorpedoOld&, Map& map, const glm_vec3& final_world_position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos);
    ~PhotonTorpedoOldProjectile();
    void update(const double& dt);
};


class PhotonTorpedoOld final : public SecondaryWeaponTorpedo {
    friend struct PhotonTorpedoOldCollisionFunctor;
    friend struct PhotonTorpedoOldInstanceCoreBindFunctor;
    friend struct PhotonTorpedoOldInstanceCoreUnbindFunctor;
    friend struct PhotonTorpedoOldInstanceGlowBindFunctor;
    friend struct PhotonTorpedoOldInstanceGlowUnbindFunctor;
    friend struct PhotonTorpedoOldFlareInstanceBindFunctor;
    friend struct PhotonTorpedoOldFlareInstanceUnbindFunctor;
    public:
        PhotonTorpedoOld(
            Ship&,
            Map&,
            const glm_vec3& position,
            const glm_vec3& forward,
            const float& arc,
            const uint& maxCharges = 1,
            const float& damage = 1650.0f,
            const float& _rechargePerRound = 4.5f,
            const float& _impactRadius = 5.5f,
            const float& _impactTime = 3.1f,
            const float& _travelSpeed = 16.5f,
            const float& _volume = 100.0f,
            const float& _rotAngleSpeed = 0.5f,
            const unsigned int& _modelIndex = 0
        );
        ~PhotonTorpedoOld();

        void update(const double& dt);
};

#endif