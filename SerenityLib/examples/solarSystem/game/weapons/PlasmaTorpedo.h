#pragma once
#ifndef GAME_WEAPON_PLASMA_TORPEDO_H
#define GAME_WEAPON_PLASMA_TORPEDO_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>
#include <ecs/Entity.h>

class  Map;
class  Ship;
class  PointLight;
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
        start = glm::quat();
        start.w = 1.0f;
        spin = glm::vec3(0.0f);
    }
    PlasmaTorpedoFlare(glm::vec3& _spin) {
        start = glm::quat();
        start.w = 1.0f;
        spin = _spin;
    }
};
struct PlasmaTorpedoProjectile final {
    PlasmaTorpedo& torpedo;
    EntityWrapper* entity;
    EntityWrapper* target;
    std::vector<PlasmaTorpedoFlare> flares;
    bool hasLock;
    float rotationAngleSpeed;
    PointLight* light;
    float currentTime;
    float maxTime;
    bool active;
    PlasmaTorpedoProjectile(PlasmaTorpedo&, Map& map, const glm::vec3& position, const glm::vec3& forward);
    ~PlasmaTorpedoProjectile();
    void update(const double& dt);
    void destroy();
};


class PlasmaTorpedo final : public SecondaryWeaponTorpedo {
    friend struct PlasmaTorpedoCollisionFunctor;
    friend struct PlasmaTorpedoInstanceCoreBindFunctor;
    friend struct PlasmaTorpedoInstanceCoreUnbindFunctor;
    friend struct PlasmaTorpedoInstanceGlowBindFunctor;
    friend struct PlasmaTorpedoInstanceGlowUnbindFunctor;
    friend struct PlasmaTorpedoFlareInstanceBindFunctor;
    friend struct PlasmaTorpedoFlareInstanceUnbindFunctor;
    private:
        Map& m_Map;
        std::vector<PlasmaTorpedoProjectile*> m_ActiveProjectiles;
    public:
        PlasmaTorpedo(
            Ship&,
            Map&,
            const glm::vec3& position,
            const glm::vec3& forward,
            const float& arc,
            const uint& maxCharges = 1,
            const uint & damage = 2750,
            const float& _rechargePerRound = 5.0f,
            const float& _impactRadius = 6.8f,
            const float& _impactTime = 4.0f,
            const float& _travelSpeed = 16.5f,
            const float& _volume = 100.0f,
            const float& _rotAngleSpeed = 0.5f
        );
        ~PlasmaTorpedo();

        const bool fire();
        void forceFire();
        void update(const double& dt);
};

#endif