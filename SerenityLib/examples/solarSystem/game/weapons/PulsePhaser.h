#pragma once
#ifndef GAME_WEAPON_PULSE_PHASER_H
#define GAME_WEAPON_PULSE_PHASER_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>
#include <ecs/Entity.h>

class  Map;
class  Ship;
class  PulsePhaser;
class  PointLight;

struct PulsePhaserCollisionFunctor;
struct PulsePhaserInstanceBindFunctor;
struct PulsePhaserInstanceUnbindFunctor;
struct PulsePhaserTailInstanceBindFunctor;
struct PulsePhaserTailInstanceUnbindFunctor;

struct PulsePhaserProjectile final {
    Entity entity;
    PointLight* light;
    float currentTime;
    float maxTime;
    bool active;
    PulsePhaserProjectile(PulsePhaser&, Map& map, const glm::vec3& position, const glm::vec3& forward);
    ~PulsePhaserProjectile();
    void update(const double& dt);
    void destroy();
};

class PulsePhaser final: public PrimaryWeaponCannon{
    friend struct PulsePhaserCollisionFunctor;
    friend struct PulsePhaserInstanceBindFunctor;
    friend struct PulsePhaserInstanceUnbindFunctor;
    friend struct PulsePhaserTailInstanceBindFunctor;
    friend struct PulsePhaserTailInstanceUnbindFunctor;
    private:
        Map& m_Map;
        std::vector<PulsePhaserProjectile*> m_ActiveProjectiles;
    public:
        PulsePhaser(Ship&, Map&, const glm::vec3& position, const glm::vec3& forward, const float& arc, const uint& maxCharges = 5, const uint & damage = 100, const float& _rechargePerRound = 1.0f, const float& _impactRadius = 1.5f, const float& _impactTime = 1.8f, const float& _travelSpeed = 50.5f);
        ~PulsePhaser();

        bool fire();
        void update(const double& dt);
};

#endif