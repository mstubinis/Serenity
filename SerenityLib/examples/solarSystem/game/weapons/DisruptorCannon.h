#pragma once
#ifndef GAME_WEAPON_DISRUPTOR_CANNON_H
#define GAME_WEAPON_DISRUPTOR_CANNON_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>
#include <ecs/Entity.h>

class  Map;
class  Ship;
class  DisruptorCannon;
class  PointLight;

struct DisruptorCannonCollisionFunctor;
struct DisruptorCannonInstanceBindFunctor;
struct DisruptorCannonInstanceUnbindFunctor;
struct DisruptorCannonTailInstanceBindFunctor;
struct DisruptorCannonTailInstanceUnbindFunctor;
struct DisruptorCannonOutlineInstanceBindFunctor;
struct DisruptorCannonOutlineInstanceUnbindFunctor;

struct DisruptorCannonProjectile final {
    Entity entity;
    PointLight* light;
    float currentTime;
    float maxTime;
    bool active;
    DisruptorCannonProjectile(DisruptorCannon&, Map& map, const glm::vec3& position, const glm::vec3& forward);
    ~DisruptorCannonProjectile();
    void update(const double& dt);
    void destroy();
};

class DisruptorCannon final : public PrimaryWeaponCannon {
    friend struct DisruptorCannonCollisionFunctor;
    friend struct DisruptorCannonInstanceBindFunctor;
    friend struct DisruptorCannonInstanceUnbindFunctor;
    friend struct DisruptorCannonTailInstanceBindFunctor;
    friend struct DisruptorCannonTailInstanceUnbindFunctor;
    friend struct DisruptorCannonOutlineInstanceBindFunctor;
    friend struct DisruptorCannonOutlineInstanceUnbindFunctor;
private:
    Map& m_Map;
    std::vector<DisruptorCannonProjectile*> m_ActiveProjectiles;
public:
    DisruptorCannon(
        Ship&,
        Map&,
        const glm::vec3& position,
        const glm::vec3& forward,
        const float& arc,
        const uint& maxCharges = 5,
        const uint & damage = 100,
        const float& _rechargePerRound = 1.0f,
        const float& _impactRadius = 2.5f,
        const float& _impactTime = 1.8f,
        const float& _travelSpeed = 50.5f,
        const float& _volume = 100.0f
    );
    ~DisruptorCannon();

    bool fire();
    void forceFire();
    void update(const double& dt);
};

#endif