#pragma once
#ifndef GAME_WEAPON_PLASMA_BEAM_H
#define GAME_WEAPON_PLASMA_BEAM_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <ecs/Entity.h>

class  PlasmaBeam;
struct PlasmaBeamInstanceBindFunctor;
struct PlasmaBeamInstanceUnbindFunctor;
struct PlasmaBeamCollisionFunctor;
class PlasmaBeam final : public PrimaryWeaponBeam {
    friend struct PlasmaBeamInstanceBindFunctor;
    friend struct PlasmaBeamInstanceUnbindFunctor;
    friend struct PlasmaBeamCollisionFunctor;
    private:
        Map& m_Map;

        EntityWrapper* firstWindupGraphic;
        EntityWrapper* secondWindupGraphic;

        PointLight* firstWindupLight;
        PointLight* secondWindupLight;
    public:
        PlasmaBeam(
            Ship& ship,
            Map& map,
            const glm::vec3& position,
            const glm::vec3& forward,
            const float& arc,
            std::vector<glm::vec3>& windupPts,
            const float& damage = 240.0f,
            const float& _chargeTimerSpeed = 4.0f,
            const float& _firingTime = 1.05f,
            const float& _impactRadius = 2.5f,
            const float& _impactTime = 1.8f,
            const float& _volume = 100.0f,
            const uint & _maxCharges = 1,
            const float& _rechargeTimePerRound = 6.0f
        );
        ~PlasmaBeam();

        const bool fire(const double& dt);
        void forceFire(const double& dt);
        void update(const double& dt);
};
#endif