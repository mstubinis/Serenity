#pragma once
#ifndef GAME_SHIP_SYSTEM_WEAPONS_H
#define GAME_SHIP_SYSTEM_WEAPONS_H

#include "ShipSystemBaseClass.h"
#include "../../weapons/WeaponIncludes.h"
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
class  Client;
struct DetectedShip;

struct ShipWeapon {
    Map&             m_Map;
    size_t           index;
    WeaponType::Type type;
    Ship&            ship;
    float            arc;
    glm_vec3         position; //relative to the ship's model center
    glm_vec3         forward;
    float            volume;
    float            damage;
    float            impactRadius;
    float            impactTime;
    SoundEffect*     soundEffect;

    uint        numRounds;
    uint        numRoundsMax;
    float       rechargeTimePerRound;
    float       rechargeTimer;
    unsigned int modelIndex;

    ShipWeapon(
        Map&,
        WeaponType::Type _type,
        Ship& _ship,
        const glm_vec3& _position,
        const glm_vec3& _forward,
        const float& _arc,
        const float& _dmg,
        const float& _impactRad,
        const float& _impactTime,
        const float& _volume,
        const uint& _numRounds,
        const float& _rechargeTimerPerRound,
        const unsigned int& _modelIndex
    );
    const glm_vec3 getWorldPosition();
    const decimal getDistanceSquared(const glm_vec3&);
    const decimal getDistanceSquared(DetectedShip& ship);
    const decimal getDistanceSquared(EntityWrapper* target);
    const bool isInArc(EntityWrapper* target, const float _arc);
    const bool isInArc(const glm_vec3& world_position, const float _arc);
};

struct PrimaryWeaponCannonPrediction final {
    glm_vec3 pedictedPosition;
    glm_vec3 pedictedVector;
    decimal finalProjectileSpeed;
    PrimaryWeaponCannonPrediction() {
        finalProjectileSpeed = static_cast<decimal>(0.0);
        pedictedPosition = pedictedVector = glm_vec3(static_cast<decimal>(0.0));
    }
};

struct PrimaryWeaponCannonProjectile {
    Map& map;
    uint          projectile_index;
    Entity        entity;
    PointLight*   light;
    float         currentTime;
    float         maxTime;
    bool          active;
    bool          destroyed;
    PrimaryWeaponCannonProjectile(Map& map, const glm_vec3& position, const glm_vec3& forward, const int index);
    virtual ~PrimaryWeaponCannonProjectile();
    virtual void update(const double& dt);
    virtual void destroy();

    void clientToServerImpact(Client& client, Ship& shipHit, const glm::vec3& impactLocalPosition, const glm::vec3& impactNormal, const float& impactRadius, const float& damage, const float& time, const bool& shields);
};

struct PrimaryWeaponCannon : public ShipWeapon {
    decimal       travelSpeed;
    PrimaryWeaponCannon(
        Map&,
        WeaponType::Type _type,
        Ship& _ship,
        const glm_vec3& _position,
        const glm_vec3& _forward,
        const float& _arc,
        const uint& _maxCharges,
        const float& _damage,
        const float& _rechargePerRound,
        const float& _impactRadius,
        const float& _impactTime,
        const float& _travelSpeed,
        const float& volume,
        const unsigned int& _modelIndex = 0
    );
    virtual const bool isInControlledArc(EntityWrapper* target);

    virtual const int canFire();
    const bool forceFire(EntityWrapper* mytarget, const int index, const glm_vec3& chosen_target_pos);
    virtual const PrimaryWeaponCannonPrediction calculatePredictedVector(EntityWrapper* mytarget, ComponentBody& projectileBody, const glm_vec3& chosen_target_pos);
    virtual void update(const double& dt);
};


struct BeamWeaponState final {enum State {
    JustStarted,
    WindingUp,
    Firing,
    CoolingDown,
    JustTurnedOff,
    Off,
};};

struct PrimaryWeaponBeam : public ShipWeapon {
    EntityWrapper*           target;
    std::vector<glm::vec3>   windupPoints;
    float                    chargeTimer;
    float                    chargeTimerSpeed;
    BeamWeaponState::State   state;
    float                    firingTime;
    float                    firingTimeShieldGraphicPing;
    float                    firingTimeMax;
    float                    additionalEndPointScale;
    float                    additionalBeamSizeScale;
    float                    rangeInKMSquared;

    Entity                   beamGraphic;
    Entity                   beamEndPointGraphic;
    RodLight*                beamLight;
    glm::vec3                targetCoordinates;

    std::vector<glm::vec3>   modPts;
    std::vector<glm::vec2>   modUvs;
    PrimaryWeaponBeam(
        WeaponType::Type _type,
        Ship& _ship,
        Map& map,
        const glm_vec3& _position,
        const glm_vec3& _forward,
        const float& _arc, 
        const float& _dmg,
        const float& _impactRad,
        const float& _impactTime,
        const float& _volume,
        std::vector<glm::vec3>& windupPts,
        const uint& _maxCharges,
        const float& _rechargeTimePerRound,
        const float& chargeTimerSpeed,
        const float& _firingTime,
        const unsigned int& _modelIndex = 0,
        const float& endpointExtraScale = 1.0f,
        const float& beamSizeExtraScale = 1.0f,
        const float& RangeInKM = 10.0f
    );
    ~PrimaryWeaponBeam();
    const bool canFire();
    void setTarget(EntityWrapper*);
    EntityWrapper* getTarget();
    virtual const bool fire(const double& dt, const glm_vec3& chosen_target_pt);
    virtual const bool forceFire(const double& dt);
    virtual const glm_vec3 calculatePredictedVector();
    virtual void update(const double& dt);

    void modifyBeamMesh(ComponentModel& beamModel, const float length);
};

struct SecondaryWeaponTorpedoPrediction final {
    glm_vec3 pedictedPosition;
    glm_vec3 pedictedVector;
    decimal finalProjectileSpeed;
    EntityWrapper* target;
    bool hasLock;
    SecondaryWeaponTorpedoPrediction() {
        hasLock = false;
        target = nullptr;
        finalProjectileSpeed = static_cast<decimal>(0.0);
        pedictedPosition = pedictedVector = glm_vec3(static_cast<decimal>(0.0));
    }
};

struct SecondaryWeaponTorpedoProjectile {
    Map& map;
    uint             projectile_index;
    Entity           entity;
    bool             hasLock;
    EntityWrapper*   target;
    float            rotationAngleSpeed;
    PointLight*      light;
    float            currentTime;
    float            maxTime;
    bool             active;
    bool             destroyed;
    SecondaryWeaponTorpedoProjectile(Map& map, const glm_vec3& position, const glm_vec3& forward, const int index);
    virtual ~SecondaryWeaponTorpedoProjectile();
    virtual void update(const double& dt);
    virtual void destroy();

    void clientToServerImpact(Client& client, Ship& shipHit, const glm::vec3& impactLocalPosition, const glm::vec3& impactNormal, const float& impactRadius, const float& damage, const float& time, const bool& shields);
};

struct SecondaryWeaponTorpedo : public ShipWeapon {
    decimal         travelSpeed;
    float           rotationAngleSpeed;
    SecondaryWeaponTorpedo(
        Map&,
        WeaponType::Type _type,
        Ship& _ship,
        const glm_vec3& _position,
        const glm_vec3& _forward,
        const float& _arc,
        const uint& _maxCharges,
        const float& _damage,
        const float& _rechargePerRound,
        const float& _impactRadius,
        const float& _impactTime,
        const float& _travelSpeed,
        const float& _volume,
        const float& _rotAngleSpeed,
        const unsigned int& _modelIndex = 0
    );
    virtual const bool isInControlledArc(EntityWrapper* target);

    virtual const int canFire();
    const bool forceFire(EntityWrapper* mytarget, const int index, const glm_vec3& chosen_target_pos);
    virtual const SecondaryWeaponTorpedoPrediction calculatePredictedVector(EntityWrapper* mytarget, ComponentBody& projectileBody, const glm_vec3& chosen_target_pos);
    virtual void update(const double& dt);
};

class ShipSystemWeapons final : public ShipSystem {
    friend class Ship;
    private:

        struct WeaponBeam final {
            PrimaryWeaponBeam* beam;
            size_t main_container_index;
            WeaponBeam() {
                beam = nullptr;
                main_container_index = 0;
            }
        };
        struct WeaponCannon final {
            PrimaryWeaponCannon* cannon;
            size_t main_container_index;
            WeaponCannon() {
                cannon = nullptr;
                main_container_index = 0;
            }
        };
        struct WeaponTorpedo final {
            SecondaryWeaponTorpedo* torpedo;
            size_t main_container_index;
            WeaponTorpedo() {
                torpedo = nullptr;
                main_container_index = 0;
            }
        };

        std::vector<PrimaryWeaponCannon*>      m_PrimaryWeaponsCannons;
        std::vector<PrimaryWeaponBeam*>        m_PrimaryWeaponsBeams;
        std::vector<SecondaryWeaponTorpedo*>   m_SecondaryWeaponsTorpedos;


        std::vector<WeaponCannon>              m_PrimaryWeaponsCannonsFwd;
        std::vector<WeaponBeam>                m_PrimaryWeaponsBeamsFwd;
        std::vector<WeaponTorpedo>             m_SecondaryWeaponsTorpedosFwd;

        std::vector<WeaponCannon>              m_PrimaryWeaponsCannonsNonFwd;
        std::vector<WeaponBeam>                m_PrimaryWeaponsBeamsNonFwd;
        std::vector<WeaponTorpedo>             m_SecondaryWeaponsTorpedosNonFwd;
    public:
        ShipSystemWeapons(Ship&);
        ~ShipSystemWeapons();

        static const decimal calculate_quadratic_time_till_hit(const glm_vec3& pos, const glm_vec3& vel, const float& s);

        glm::vec3 cannonTargetPoint; //for random target spots on the hull / random subsystem targets
        glm::vec3 torpedoTargetPoint;//for random target spots on the hull / random subsystem targets


        void addPrimaryWeaponBeam(PrimaryWeaponBeam&, const bool isForwardWeapon = false);
        void addPrimaryWeaponCannon(PrimaryWeaponCannon&, const bool isForwardWeapon = false);
        void addSecondaryWeaponTorpedo(SecondaryWeaponTorpedo&, const bool isForwardWeapon = false);

        PrimaryWeaponBeam& getPrimaryWeaponBeam(const uint index);
        PrimaryWeaponCannon& getPrimaryWeaponCannon(const uint index);
        SecondaryWeaponTorpedo& getSecondaryWeaponTorpedo(const uint index);

        std::vector<PrimaryWeaponCannon*>& getCannons();
        std::vector<PrimaryWeaponBeam*>& getBeams();
        std::vector<SecondaryWeaponTorpedo*>& getTorpedos();


        std::vector<WeaponCannon>& getForwardCannons();
        std::vector<WeaponBeam>& getForwardBeams();
        std::vector<WeaponTorpedo>& getForwardTorpedos();

        std::vector<WeaponCannon>& getNonForwardCannons();
        std::vector<WeaponBeam>& getNonForwardBeams();
        std::vector<WeaponTorpedo>& getNonForwardTorpedos();


        void update(const double& dt);
};

#endif