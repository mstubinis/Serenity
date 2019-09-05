#pragma once
#ifndef GAME_SHIP_SYSTEM_WEAPONS_H
#define GAME_SHIP_SYSTEM_WEAPONS_H

#include "ShipSystemBaseClass.h"
#include <vector>
#include <glm/vec3.hpp>

class EntityWrapper;
class ComponentBody;
struct ShipWeapon {
    Ship&           ship;
    float           arc;
    glm::vec3       position; //relative to the ship's model center
    glm::vec3       forward;

    ShipWeapon(Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc);

    const bool isInArc(EntityWrapper* target, const float _arc);
};

struct PrimaryWeaponCannonPrediction final {
    glm::vec3 pedictedPosition;
    glm::vec3 pedictedVector;
    float finalProjectileSpeed;
    PrimaryWeaponCannonPrediction() {
        finalProjectileSpeed = 0.0f;
        pedictedPosition = pedictedVector = glm::vec3(0.0f);
    }
};

struct PrimaryWeaponCannon : public ShipWeapon {
    uint        damage;
    float       impactRadius;
    float       impactTime;
    uint        numRounds;
    uint        numRoundsMax;
    float       rechargeTimePerRound;
    float       rechargeTimer;
    float       travelSpeed;
    float       volume;

    PrimaryWeaponCannon(
        Ship& _ship,
        const glm::vec3& _position,
        const glm::vec3& _forward,
        const float& _arc,
        const uint& _maxCharges,
        const uint& _damage,
        const float& _rechargePerRound,
        const float& _impactRadius,
        const float& _impactTime,
        const float& _travelSpeed,
        const float& volume
    );
    virtual const bool fire();
    virtual void forceFire();
    virtual const PrimaryWeaponCannonPrediction calculatePredictedVector(ComponentBody& projectileBody);
    virtual void update(const double& dt);
};

struct PrimaryWeaponBeam : public ShipWeapon {

    PrimaryWeaponBeam(Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc);
    virtual const bool fire();
    virtual void forceFire();
    virtual const glm::vec3 calculatePredictedVector();
    virtual void update(const double& dt);
};

struct SecondaryWeaponTorpedoPrediction final {
    glm::vec3 pedictedPosition;
    glm::vec3 pedictedVector;
    float finalProjectileSpeed;
    EntityWrapper* target;
    bool hasLock;
    SecondaryWeaponTorpedoPrediction() {
        hasLock = false;
        target = nullptr;
        finalProjectileSpeed = 0.0f;
        pedictedPosition = pedictedVector = glm::vec3(0.0f);
    }
};

struct SecondaryWeaponTorpedo : public ShipWeapon {
    uint            damage;
    float           impactRadius;
    float           impactTime;
    uint            numRounds;
    uint            numRoundsMax;
    float           rechargeTimePerRound;
    float           rechargeTimer;
    float           travelSpeed;
    float           volume;
    float           rotationAngleSpeed;

    SecondaryWeaponTorpedo(
        Ship& _ship,
        const glm::vec3& _position,
        const glm::vec3& _forward,
        const float& _arc,
        const uint& _maxCharges,
        const uint& _damage,
        const float& _rechargePerRound,
        const float& _impactRadius,
        const float& _impactTime,
        const float& _travelSpeed,
        const float& _volume,
        const float& _rotAngleSpeed
    );

    virtual const bool isInControlledArc(EntityWrapper* target);

    virtual const bool canFire();
    virtual const bool fire();
    virtual void forceFire();
    virtual const SecondaryWeaponTorpedoPrediction calculatePredictedVector(ComponentBody& projectileBody);
    virtual void update(const double& dt);
};


class ShipSystemWeapons final : public ShipSystem {
    friend class Ship;
    private:
        std::vector<PrimaryWeaponCannon*>      m_PrimaryWeaponsCannons;
        std::vector<PrimaryWeaponBeam*>        m_PrimaryWeaponsBeams;
        std::vector<SecondaryWeaponTorpedo*>   m_SecondaryWeaponsTorpedos;
    public:
        ShipSystemWeapons(Ship&);
        ~ShipSystemWeapons();

        static float calculate_quadratic_time_till_hit(const glm::vec3& pos, const glm::vec3& vel, const float& s);

        void addPrimaryWeaponBeam(PrimaryWeaponBeam&);
        void addPrimaryWeaponCannon(PrimaryWeaponCannon&);
        void addSecondaryWeaponTorpedo(SecondaryWeaponTorpedo&);

        void update(const double& dt);
};

#endif