#pragma once
#ifndef GAME_SHIP_SYSTEM_WEAPONS_H
#define GAME_SHIP_SYSTEM_WEAPONS_H

#include "ShipSystemBaseClass.h"
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <ecs/Entity.h>

class  EntityWrapper;
class  ComponentBody;
class  ComponentModel;
class  SoundEffect;
class  PointLight;
class  RodLight;
class  Map;
class  Ship;

struct ShipWeapon {
    Ship&           ship;
    float           arc;
    glm::vec3       position; //relative to the ship's model center
    glm::vec3       forward;
    float           volume;
    uint            damage;
    float           impactRadius;
    float           impactTime;
    SoundEffect*    soundEffect;

    uint        numRounds;
    uint        numRoundsMax;
    float       rechargeTimePerRound;
    float       rechargeTimer;

    ShipWeapon(
        Ship& _ship,
        const glm::vec3& _position,
        const glm::vec3& _forward,
        const float& _arc,
        const uint& _dmg,
        const float& _impactRad,
        const float& _impactTime,
        const float& _volume,
        const uint& _numRounds,
        const float& _rechargeTimerPerRound
    );

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

struct PrimaryWeaponCannonProjectile {
    Entity        entity;
    PointLight*   light;
    float         currentTime;
    float         maxTime;
    bool          active;
    PrimaryWeaponCannonProjectile(Map& map, const glm::vec3& position, const glm::vec3& forward);
    ~PrimaryWeaponCannonProjectile();
    virtual void update(const double& dt);
    virtual void destroy();
};

struct PrimaryWeaponCannon : public ShipWeapon {
    float       travelSpeed;

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
    std::vector<glm::vec3>   windupPoints;
    float                    chargeTimer;
    float                    chargeTimerSpeed;
    bool                     isFiring;
    bool                     isFiringWeapon;
    float                    firingTime;
    float                    firingTimeMax;

    EntityWrapper*           beamGraphic;
    EntityWrapper*           beamEndPointGraphic;
    RodLight*                beamLight;

    std::vector<glm::vec3>   modPts;
    std::vector<glm::vec2>   modUvs;


    PrimaryWeaponBeam(
        Ship& _ship,
        Map& map,
        const glm::vec3& _position,
        const glm::vec3& _forward,
        const float& _arc, 
        const uint& _dmg,
        const float& _impactRad,
        const float& _impactTime,
        const float& _volume,
        std::vector<glm::vec3>& windupPts,
        const uint& _maxCharges,
        const float& _rechargeTimePerRound,
        const float& chargeTimerSpeed,
        const float& _firingTime
    );
    ~PrimaryWeaponBeam();
    virtual const bool fire(const double& dt);
    virtual void forceFire(const double& dt);
    virtual const glm::vec3 calculatePredictedVector();
    virtual void update(const double& dt);

    void modifyBeamMesh(ComponentModel& beamModel, const float length);
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
    float           travelSpeed;
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