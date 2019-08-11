#pragma once
#ifndef GAME_WEAPON_PULSE_PHASER_H
#define GAME_WEAPON_PULSE_PHASER_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>
#include <ecs/Entity.h>

class  Map;
class  Ship;
class PulsePhaser;
struct PulsePhaserProjectile final {
    Entity entity;
    float currentTime;
    float maxTime;
    bool active;
    PulsePhaserProjectile(PulsePhaser&, Map& map, const glm::vec3& position, const glm::vec3& forward);
    ~PulsePhaserProjectile();
    void update(const double& dt);
};

class PulsePhaser final: public PrimaryWeaponCannon{
    private:
        Map& m_Map;
        std::vector<PulsePhaserProjectile*> m_ActiveProjectiles;
    public:
        PulsePhaser(Ship&, Map&, const glm::vec3& position, const glm::vec3& forward, const float& arc);
        ~PulsePhaser();

        bool fire();
        void update(const double& dt);
};

#endif