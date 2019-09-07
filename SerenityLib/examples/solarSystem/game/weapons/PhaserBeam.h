#pragma once
#ifndef GAME_WEAPON_PHASER_BEAM_H
#define GAME_WEAPON_PHASER_BEAM_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <ecs/Entity.h>

class  Map;
class  Ship;
class  PhaserBeam;
class  PointLight;
class PhaserBeam final : public PrimaryWeaponBeam {
    private:
        Map& m_Map;
    public:
        PhaserBeam(
            Ship& ship,
            Map& map,
            const glm::vec3& position,
            const glm::vec3& forward,
            const float& arc,
            std::vector<glm::vec3>& windupPts,
            const uint& damage = 100,
            const float& _impactRadius = 2.5f,
            const float& _impactTime = 1.8f,
            const float& _volume = 100.0f
        );
        ~PhaserBeam();

        const bool fire();
        void forceFire();
        void update(const double& dt);
};
#endif