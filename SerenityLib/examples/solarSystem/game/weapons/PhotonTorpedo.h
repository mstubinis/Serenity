#pragma once
#ifndef GAME_WEAPON_PHOTON_TORPEDO_H
#define GAME_WEAPON_PHOTON_TORPEDO_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>
#include <ecs/Entity.h>

class  Map;
class  Ship;
class  PhotonTorpedo;
class  PointLight;
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
        start = glm::quat();
        start.w = 1.0f;
        spin = glm::vec3(0.0f);
    }
    PhotonTorpedoFlare(glm::vec3& _spin) {
        start = glm::quat();
        start.w = 1.0f;
        spin = _spin;
    }
};
struct PhotonTorpedoProjectile final {
    PhotonTorpedo& torpedo;
    Entity entityCore;
    Entity entityGlow;
    bool hasLock;
    EntityWrapper* target;
    float rotationAngleSpeed;
    std::vector<PhotonTorpedoFlare> flares;
    PointLight* light;
    float currentTime;
    float maxTime;
    bool active;
    PhotonTorpedoProjectile(PhotonTorpedo&, Map& map, const glm::vec3& position, const glm::vec3& forward);
    ~PhotonTorpedoProjectile();
    void update(const double& dt);
    void destroy();
};


class PhotonTorpedo final : public SecondaryWeaponTorpedo {
    friend struct PhotonTorpedoCollisionFunctor;
    friend struct PhotonTorpedoInstanceCoreBindFunctor;
    friend struct PhotonTorpedoInstanceCoreUnbindFunctor;
    friend struct PhotonTorpedoInstanceGlowBindFunctor;
    friend struct PhotonTorpedoInstanceGlowUnbindFunctor;
    friend struct PhotonTorpedoFlareInstanceBindFunctor;
    friend struct PhotonTorpedoFlareInstanceUnbindFunctor;
    private:
        Map& m_Map;
        std::vector<PhotonTorpedoProjectile*> m_ActiveProjectiles;
    public:
        PhotonTorpedo(
            Ship&,
            Map&,
            const glm::vec3& position,
            const glm::vec3& forward,
            const float& arc,
            const uint& maxCharges = 1,
            const uint & damage = 2000,
            const float& _rechargePerRound = 5.0f,
            const float& _impactRadius = 5.5f,
            const float& _impactTime = 3.1f,
            const float& _travelSpeed = 16.5f,
            const float& _volume = 100.0f,
            const float& _rotAngleSpeed = 0.5f
        );
        ~PhotonTorpedo();

        const bool fire();
        void forceFire();
        void update(const double& dt);
};

#endif