#pragma once
#ifndef GAME_WEAPON_BORG_BEAM_H
#define GAME_WEAPON_BORG_BEAM_H

#include "../../ships/shipSystems/ShipSystemWeapons.h"
#include <ecs/Entity.h>

class  BorgBeam;
struct BorgBeamInstanceBindFunctor;
struct BorgBeamInstanceUnbindFunctor;
struct BorgBeamCollisionFunctor;
class BorgBeam final : public PrimaryWeaponBeam {
    friend struct BorgBeamInstanceBindFunctor;
    friend struct BorgBeamInstanceUnbindFunctor;
    friend struct BorgBeamCollisionFunctor;
    public:
        BorgBeam(
            Ship& ship,
            Map& map,
            const glm_vec3& position,
            const glm_vec3& forward,
            const float& arc,
            std::vector<glm::vec3>& windupPts,
            const float& damage = 1000.0f,
            const float& _chargeTimerSpeed = 4.0f,
            const float& _firingTime = 1.05f,
            const float& _impactRadius = 2.5f,
            const float& _impactTime = 1.8f,
            const float& _volume = 100.0f,
            const uint & _maxCharges = 1,
            const float& _rechargeTimePerRound = 6.0f,
            const unsigned int& _modelIndex = 0,
            const float& endpointExtraScale = 1.0f,
            const float& beamSizeExtraScale = 1.0f,
            const float& RangeInKM = 10.0f,
            const float& BeamLaunchSpeed = 235.0f
        );
        ~BorgBeam();

        const bool fire(const double& dt, const glm_vec3& chosen_target_pt);
        const bool forceFire(const double& dt);
        void update(const double& dt);
};
#endif