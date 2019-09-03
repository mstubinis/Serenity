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

struct PhotonTorpedoCollisionFunctor;

struct PhotonTorpedoInstanceCoreBindFunctor;
struct PhotonTorpedoInstanceCoreUnbindFunctor;

struct PhotonTorpedoInstanceGlowBindFunctor;
struct PhotonTorpedoInstanceGlowUnbindFunctor;

struct PhotonTorpedoFlareInstanceBindFunctor;
struct PhotonTorpedoFlareInstanceUnbindFunctor;

struct PhotonTorpedoProjectile final {
    Entity entityCore;
    Entity entityGlow;
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
            const uint & damage = 1000,
            const float& _rechargePerRound = 5.0f,
            const float& _impactRadius = 4.5f,
            const float& _impactTime = 3.1f,
            const float& _travelSpeed = 0.5f,
            const float& _volume = 100.0f,
            const float& _rotAngleSpeed = 0.5f
        );
        ~PhotonTorpedo();

        bool fire();
        void forceFire();
        void update(const double& dt);
};

#endif