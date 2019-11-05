#pragma once
#ifndef GAME_WEAPON_QUANTUM_TORPEDO_H
#define GAME_WEAPON_QUANTUM_TORPEDO_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>

class  QuantumTorpedo;
struct QuantumTorpedoCollisionFunctor;
struct QuantumTorpedoInstanceCoreBindFunctor;
struct QuantumTorpedoInstanceCoreUnbindFunctor;
struct QuantumTorpedoInstanceGlowBindFunctor;
struct QuantumTorpedoInstanceGlowUnbindFunctor;
struct QuantumTorpedoFlareInstanceBindFunctor;
struct QuantumTorpedoFlareInstanceUnbindFunctor;

struct QuantumTorpedoFlare final {
    glm::vec3 spin;
    glm::quat start;
    QuantumTorpedoFlare() {
        start = glm::quat();
        start.w = 1.0f;
        spin = glm::vec3(0.0f);
    }
    QuantumTorpedoFlare(glm::vec3& _spin) {
        start = glm::quat();
        start.w = 1.0f;
        spin = _spin;
    }
};
struct QuantumTorpedoProjectile final : public SecondaryWeaponTorpedoProjectile {
    QuantumTorpedo&                    torpedo;
    std::vector<QuantumTorpedoFlare>   flares;
    QuantumTorpedoProjectile(EntityWrapper* target, QuantumTorpedo&, Map& map, const glm_vec3& final_world_position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos);
    ~QuantumTorpedoProjectile();
    void update(const double& dt);
};


class QuantumTorpedo final : public SecondaryWeaponTorpedo {
    friend struct QuantumTorpedoCollisionFunctor;
    friend struct QuantumTorpedoInstanceCoreBindFunctor;
    friend struct QuantumTorpedoInstanceCoreUnbindFunctor;
    friend struct QuantumTorpedoInstanceGlowBindFunctor;
    friend struct QuantumTorpedoInstanceGlowUnbindFunctor;
    friend struct QuantumTorpedoFlareInstanceBindFunctor;
    friend struct QuantumTorpedoFlareInstanceUnbindFunctor;
    public:
        QuantumTorpedo(
            Ship&,
            Map&,
            const glm_vec3& position,
            const glm_vec3& forward,
            const float& arc,
            const uint& maxCharges = 1,
            const float& damage = 3000.0f,
            const float& _rechargePerRound = 5.0f,
            const float& _impactRadius = 6.5f,
            const float& _impactTime = 4.2f,
            const float& _travelSpeed = 16.5f,
            const float& _volume = 100.0f,
            const float& _rotAngleSpeed = 0.5f,
            const unsigned int& _modelIndex = 0
        );
        ~QuantumTorpedo();

        void update(const double& dt);
};

#endif