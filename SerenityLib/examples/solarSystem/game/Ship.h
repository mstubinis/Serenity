#pragma once
#ifndef GAME_SHIP_H
#define GAME_SHIP_H

#include <ecs/Components.h>
#include <glm/vec2.hpp>
#include <core/engine/utils/Utils.h>
#include "Client.h"

#include "ships/shipSystems/ShipSystemBaseClass.h"

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
        bool                                 m_SavedOldStateBefore;
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

        const std::string getName();
        const glm::vec3 getWarpSpeedVector3();
        const glm::vec3 getPosition();

        void updatePhysicsFromPacket(const PacketPhysicsUpdate& packet, Map& map, std::vector<std::string>& info);
        void updateCloakFromPacket(const PacketCloakUpdate& packet);

        void setModel(Handle& handle);

        void translateWarp(const double& amount, const double& dt);
        void toggleWarp(){
            m_IsWarping = !m_IsWarping;
            m_WarpFactor = 0;
        }
        bool canSeeCloak();
        bool cloak(bool sendPacket = true);
        bool decloak(bool sendPacket = true);

        const std::string& getClass() const { return m_ShipClass; }
        GameCamera* getPlayerCamera(){ return m_PlayerCamera; }
        bool IsPlayer(){ return m_IsPlayer; }
        bool IsWarping(){ return m_IsWarping; }
        bool isCloaked();
        bool isFullyCloaked();
        ShipSystem* getShipSystem(uint type){ return m_ShipSystems[type]; }
        EntityWrapper* getTarget() { return m_Target; }
        Entity& entity() { return m_Entity; }
        void setTarget(EntityWrapper* entityWrapper);
        void setTarget(const std::string&);

        void savePositionState();
        void restorePositionState();
};
#endif
