#pragma once
#ifndef GAME_SHIP_H
#define GAME_SHIP_H

#include <ecs/Components.h>
#include <glm/vec2.hpp>
#include <core/engine/utils/Utils.h>
#include "Client.h"

#include "ships/shipSystems/ShipSystemBaseClass.h"

#define WARP_PHYSICS_MODIFIER 1.333333333f

class  GameCamera;
class  Ship;
class  Map;
struct PacketPhysicsUpdate;
struct PacketCloakUpdate;
struct ShipLogicFunctor;
class  ShipSystemReactor;
class  ShipSystemMainThrusters;
class  ShipSystemYawThrusters;
class  ShipSystemPitchThrusters;
class  ShipSystemRollThrusters;
class  ShipSystemCloakingDevice;
class  ShipSystemWarpDrive;
class  ShipSystemSensors;
class  ShipSystemShields;
class  ShipSystemWeapons;
class  ShipSystem;

struct PrimaryWeaponBeam;
struct PrimaryWeaponCannon;
struct SecondaryWeaponTorpedo;

class Ship: public EntityWrapper {
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
	friend  class ShipSystem;
    protected:
        Client&                              m_Client;
        std::unordered_map<uint,ShipSystem*> m_ShipSystems;
        bool                                 m_IsPlayer;
        GameCamera*                          m_PlayerCamera;
		glm::dvec2                           m_MouseFactor;
        bool                                 m_IsWarping;
        float                                m_WarpFactor;
        EntityWrapper*                       m_Target;
        std::string                          m_ShipClass;
    public:
        Ship(
            Client& client,
            Handle& meshHandle,                   //Mesh
            Handle& materialHandle,               //Material
            const std::string& shipClass,
            bool player = false,                  //Player Ship?
            const std::string& name = "Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull,
            Map* = nullptr
        );
        virtual ~Ship();

        void onEvent(const Event&);

        void destroy();

        const std::string getName();
        const glm::vec3 getWarpSpeedVector3();
        const glm::vec3 getPosition();
        const glm::quat getRotation();
        const glm::vec3 getPosition(const EntityDataRequest& dataRequest);
        const glm::quat getRotation(const EntityDataRequest& dataRequest);

        void updatePhysicsFromPacket(const PacketPhysicsUpdate& packet, Map& map, std::vector<std::string>& info);
        void updateCloakFromPacket(const PacketCloakUpdate& packet);

        void setModel(Handle& handle);

        const glm::vec3 getLinearVelocity();

        void translateWarp(const double& amount, const double& dt);
        void toggleWarp();
        bool canSeeCloak();
        bool cloak(const bool sendPacket = true);
        bool decloak(const bool sendPacket = true);

        const std::string& getClass() const;
        GameCamera* getPlayerCamera();
        const bool IsPlayer() const;
        const bool IsWarping() const;
        const bool isCloaked();
        const bool isFullyCloaked();
        ShipSystem* getShipSystem(const uint type);
        EntityWrapper* getTarget();
        Entity& entity();
        void setTarget(EntityWrapper* entityWrapper, const bool sendPacket);
        void setTarget(const std::string&, const bool sendPacket);


        PrimaryWeaponBeam& getPrimaryWeaponBeam(const uint index);
        PrimaryWeaponCannon& getPrimaryWeaponCannon(const uint index);
        SecondaryWeaponTorpedo& getSecondaryWeaponTorpedo(const uint index);
};
#endif
