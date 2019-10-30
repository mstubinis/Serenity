#pragma once
#ifndef GAME_WEAPON_KLINGON_PHOTON_TORPEDO_H
#define GAME_WEAPON_KLINGON_PHOTON_TORPEDO_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>

class  KlingonPhotonTorpedo;
struct KlingonPhotonTorpedoCollisionFunctor;
struct KlingonPhotonTorpedoInstanceCoreBindFunctor;
struct KlingonPhotonTorpedoInstanceCoreUnbindFunctor;
struct KlingonPhotonTorpedoInstanceGlowBindFunctor;
struct KlingonPhotonTorpedoInstanceGlowUnbindFunctor;
struct KlingonPhotonTorpedoFlareInstanceBindFunctor;
struct KlingonPhotonTorpedoFlareInstanceUnbindFunctor;

struct KlingonPhotonTorpedoFlare final {
    glm::vec3 spin;
    glm::quat start;
    KlingonPhotonTorpedoFlare() {
        start = glm::quat();
        start.w = 1.0f;
        spin = glm::vec3(0.0f);
    }
    KlingonPhotonTorpedoFlare(glm::vec3& _spin) {
        start = glm::quat();
        start.w = 1.0f;
        spin = _spin;
    }
};
struct KlingonPhotonTorpedoProjectile final : public SecondaryWeaponTorpedoProjectile {
    KlingonPhotonTorpedo& torpedo;
    std::vector<KlingonPhotonTorpedoFlare>   flares;
    KlingonPhotonTorpedoProjectile(KlingonPhotonTorpedo&, Map& map, const glm_vec3& position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos);
    ~KlingonPhotonTorpedoProjectile();
    void update(const double& dt);
};


class KlingonPhotonTorpedo final : public SecondaryWeaponTorpedo {
    friend struct KlingonPhotonTorpedoCollisionFunctor;
    friend struct KlingonPhotonTorpedoInstanceCoreBindFunctor;
    friend struct KlingonPhotonTorpedoInstanceCoreUnbindFunctor;
    friend struct KlingonPhotonTorpedoInstanceGlowBindFunctor;
    friend struct KlingonPhotonTorpedoInstanceGlowUnbindFunctor;
    friend struct KlingonPhotonTorpedoFlareInstanceBindFunctor;
    friend struct KlingonPhotonTorpedoFlareInstanceUnbindFunctor;
public:
    KlingonPhotonTorpedo(
        Ship&,
        Map&,
        const glm_vec3& position,
        const glm_vec3& forward,
        const float& arc,
        const uint& maxCharges = 1,
        const float& damage = 2000.0f,
        const float& _rechargePerRound = 5.0f,
        const float& _impactRadius = 5.5f,
        const float& _impactTime = 3.1f,
        const float& _travelSpeed = 16.5f,
        const float& _volume = 100.0f,
        const float& _rotAngleSpeed = 0.5f,
        const unsigned int& _modelIndex = 0
    );
    ~KlingonPhotonTorpedo();

    void update(const double& dt);
};

#endif