#pragma once
#ifndef GAME_SHIP_H
#define GAME_SHIP_H

#include <unordered_map>

#include <ecs/Components.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/events/Engine_EventObject.h>
#include "networking/client/Client.h"
#include "teams/Team.h"
#include "ai/AIIncludes.h"

#include "ships/shipSystems/ShipSystemWeapons.h"
#include <ecs/EntityWrapper.h>

class  GameCamera;
class  Decal;
struct PacketMessage;
struct PacketPhysicsUpdate;
struct PacketCloakUpdate;
struct PacketHealthUpdate;
struct PacketProjectileImpact;
struct ShipLogicFunctor;
struct GameCameraLogicFunctor;
struct HullCollisionFunctor;
class  ShipSystemReactor;
class  ShipSystemMainThrusters;
class  ShipSystemYawThrusters;
class  ShipSystemPitchThrusters;
class  ShipSystemRollThrusters;
class  ShipSystemCloakingDevice;
class  ShipSystemWarpDrive;
class  ShipSystemSensors;
class  ShipSystemShields;
class  ShipSystemHull;
class  ShipSystem;
class  SensorStatusDisplay;
class  AI;
class  FireAtWill;
struct PrimaryWeaponBeam;
struct PrimaryWeaponCannon;
struct SecondaryWeaponTorpedo;

struct ShipModelInstanceBindFunctor {
    void operator()(EngineResource* r) const;
};
struct ShipModelInstanceUnbindFunctor {
    void operator()(EngineResource* r) const;
};

struct ShipState final { enum State {
    Nominal,
    JustFlaggedForDestruction,
    UndergoingDestruction,
    JustFlaggedAsFullyDestroyed,
    Destroyed,
    JustFlaggedToRespawn,
    UndergoingRespawning,
    WaitingForServerToRespawnMe,
};};
//struct ShipFlags final { enum Flag {
//    None            = 0,
//    Invincible = 1 << 0,
//};};

class Ship: public EntityWrapper, public EventObserver {
    friend struct ShipModelInstanceBindFunctor;
    friend struct ShipModelInstanceUnbindFunctor;
    friend struct GameCameraLogicFunctor;
    friend struct HullCollisionFunctor;
    friend  class GameCamera;
    friend  class Client;
    friend struct ShipLogicFunctor;
    friend  class ShipSystemReactor;
    friend  class ShipSystemMainThrusters;
	friend  class ShipSystemYawThrusters;
	friend  class ShipSystemPitchThrusters;
	friend  class ShipSystemRollThrusters;
    friend  class ShipSystemCloakingDevice;
    friend  class ShipSystemWarpDrive;
    friend  class ShipSystemSensors;
    friend  class ShipSystemShields;
    friend  class ShipSystemWeapons;
    friend  class ShipSystemHull;
	friend  class ShipSystem;
    friend  class SensorStatusDisplay;
    friend  class FireAtWill;
    friend struct PrimaryWeaponBeam;
    friend struct PrimaryWeaponCannon;
    friend struct SecondaryWeaponTorpedo;
    private:
        void internal_update_undergoing_respawning(const double& dt, Map& map);
        void internal_update_just_flagged_for_respawn(const double& dt, Map& map);
        void internal_update_just_flagged_for_destruction(const double& dt, Map& map);
        void internal_update_just_destroyed_fully(const double& dt, Map& map);
        void internal_update_undergoing_destruction(const double& dt, Map& map);
        void internal_update_damage_emitters(const double& dt, Map& map);
        void internal_update_decals(const double& dt, Map& map);
        void internal_update_ai(const double& dt, Map& map);
        void internal_update_player_you_logic(const double& dt, Map& map);
        void internal_calculate_ship_destruction_time_max(ComponentModel&);
    public:
        std::vector<std::tuple<ParticleEmitter*, size_t, glm_vec3, glm_quat>>        m_EmittersDestruction;
        Client&                                                                      m_Client;
    protected:
        void internal_finialize_init(const AIType::Type& type);


        std::vector<std::tuple<Decal*,size_t>>                                       m_DamageDecals;


        //unsigned int                                 m_Flags;
        ShipState::State                               m_State;

        double                                         m_DestructionTimerCurrent;
        double                                         m_DestructionTimerDecalTimer;
        double                                         m_DestructionTimerDecalTimerMax;
        double                                         m_DestructionTimerMax;
        double                                         m_RespawnTimer;
        double                                         m_RespawnTimerMax;
        float                                          m_OfflineGlowFactor;
        float                                          m_OfflineGlowFactorTimer;
        float                                          m_VisualMass; //to differentiate between physics mass, for hull collisions

        Team&                                          m_Team;
        std::unordered_map<unsigned int,ShipSystem*>   m_ShipSystems;
        AI*                                            m_AI;
        GameCamera*                                    m_PlayerCamera;
		glm::dvec2                                     m_MouseFactor;
        bool                                           m_IsWarping;
        float                                          m_WarpFactor;
        std::string                                    m_ShipClass;
        std::string                                    m_MapKey;
        glm::vec3                                      m_CameraOffsetDefault;
    public:
        Ship(
            Team& team,
            Client& client,
            const std::string& shipClass,
            Map& map,
            const AIType::Type ai_type,
            const std::string& name = "Ship",     //Name
            const glm_vec3 = glm_vec3(0),         //Position
            const glm_vec3 = glm_vec3(1),         //Scale
            CollisionType::Type = CollisionType::ConvexHull,
            const glm::vec3 camOffsetDefault = glm::vec3(0.0f)
        );
        virtual ~Ship();

        virtual void update(const double& dt);

        void onEvent(const Event&);

        void destroy();

        void respawn(const glm_vec3& newPosition, const std::string& nearest_spawn_anchor, Map& map);
        const bool setState(const ShipState::State& state);

        AI* getAI();
        const bool isDestroyed() const;
        const bool isFullyDestroyed() const;
        const AIType::Type getAIType() const;
        const Team& getTeam() const;
        const std::string& getMapKey() const;
        const std::string getName();
        const glm_vec3 getWarpSpeedVector3();
        const glm_vec3 getPosition();
        const glm_vec3 getScale();
        const glm::vec3 getAimPositionDefault();
        const glm::vec3 getAimPositionRandom();
        const glm::vec3 getAimPositionDefaultLocal();
        const glm::vec3 getAimPositionRandomLocal();
        const uint getAimPositionRandomLocalIndex();
        const glm::vec3 getAimPositionLocal(const uint index);
        const glm_quat getRotation();
        const glm_vec3 getPosition(const EntityDataRequest& dataRequest);
        const glm_quat getRotation(const EntityDataRequest& dataRequest);

        void setPosition(const decimal& x, const decimal& y, const decimal& z);
        void setPosition(const glm_vec3& position);
        void setPosition(const decimal& x, const decimal& y, const decimal& z, const EntityDataRequest& dataRequest);
        void setPosition(const glm_vec3& position, const EntityDataRequest& dataRequest);

        void updatePhysicsFromPacket(const PacketPhysicsUpdate& packet, Map& map);
        void updateCloakFromPacket(const PacketCloakUpdate& packet);
        void updateAntiCloakScanFromPacket(const PacketMessage& packet);
        void updateHealthFromPacket(const PacketHealthUpdate& packet);

        void updateCloakVisuals(const float& r, const float& g, const float& b, const float& alpha, ComponentModel& model);
        void updateCloakVisuals(const float& alpha, ComponentModel& model);
        void updateProjectileImpact(const PacketProjectileImpact& packet);

        void setModel(Handle& handle);
        const float updateShipDimensions();

        const glm_vec3 getLinearVelocity();

        void setDamping(const decimal& linear, const decimal& angular);
        void translateWarp(const double& amount, const double& dt);
        void toggleWarp();
        const bool canSeeCloak(Ship* otherShip);
        virtual bool cloak(const bool sendPacket = true);
        virtual bool decloak(const bool sendPacket = true);

        const std::string& getClass() const;
        GameCamera* getPlayerCamera();
        const bool IsPlayer() const;
        const bool IsWarping() const;
        const bool isCloaked();
        const bool isFullyCloaked();
        Entity& entity();
        ShipSystem* getShipSystem(const uint type);
        EntityWrapper* getTarget();      
        void setTarget(EntityWrapper* entityWrapper, const bool sendPacket);
        void setTarget(const std::string&, const bool sendPacket);

        const glm_vec3& forward();
        const glm_vec3& right();
        const glm_vec3& up();

        const bool isAlly(Ship& other);
        const bool isEnemy(Ship& other);
        const bool isNeutral(Ship& other);

        void apply_threat(const std::string& source, const unsigned int threat_amount);

        PrimaryWeaponBeam& getPrimaryWeaponBeam(const uint index);
        PrimaryWeaponCannon& getPrimaryWeaponCannon(const uint index);
        SecondaryWeaponTorpedo& getSecondaryWeaponTorpedo(const uint index);

        virtual void fireBeams(ShipSystemWeapons& weapons, EntityWrapper* target, Ship* target_as_ship);
        virtual void fireCannons(ShipSystemWeapons& weapons, EntityWrapper* target, Ship* target_as_ship);
        virtual void fireTorpedos(ShipSystemWeapons& weapons, EntityWrapper* target, Ship* target_as_ship);
};
#endif
