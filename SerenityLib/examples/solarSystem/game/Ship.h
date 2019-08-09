#pragma once
#ifndef GAME_SHIP_H
#define GAME_SHIP_H

#include <ecs/Components.h>
#include <glm/vec2.hpp>
#include <core/engine/utils/Utils.h>
#include "Client.h"

class GameCamera;
class Ship;
class Map;

struct PacketPhysicsUpdate;
struct PacketCloakUpdate;
struct ShipLogicFunctor;

struct ShipSystemType {enum Type {
    Reactor,
    ThrustersPitch,
    ThrustersYaw,
    ThrustersRoll,
    CloakingDevice,
    Shields,
    ThrustersMain,
    WarpDrive,
    Sensors,
_TOTAL}; };

class ShipSystem{
	friend class ::Ship;
    protected:
        Ship& m_Ship;
        uint  m_Type;
        float m_Health;
        float m_Power;
    public:
        ShipSystem(const uint& type,Ship&);
        virtual ~ShipSystem();

        const bool isOnline() const { return (m_Health > 0 && m_Power > 0) ? true : false; }

        virtual void update(const double& dt);
};
class ShipSystemReactor final: public ShipSystem{
	friend class ::Ship;
    private:
        float m_TotalPowerMax;
        float m_TotalPower;
    public:
        ShipSystemReactor(Ship&, const float maxPower, const float currentPower = -1);
        ~ShipSystemReactor();

        void update(const double& dt);
};
class ShipSystemMainThrusters final: public ShipSystem{
	friend class ::Ship;
    public:
        ShipSystemMainThrusters(Ship&);
        ~ShipSystemMainThrusters();

        void update(const double& dt);
};
class ShipSystemPitchThrusters final: public ShipSystem{
	friend class ::Ship;
    public:
        ShipSystemPitchThrusters(Ship&);
        ~ShipSystemPitchThrusters();

        void update(const double& dt);
};
class ShipSystemYawThrusters final: public ShipSystem{
	friend class ::Ship;
    public:
        ShipSystemYawThrusters(Ship&);
        ~ShipSystemYawThrusters();

        void update(const double& dt);
};
class ShipSystemRollThrusters final: public ShipSystem{
	friend class ::Ship;
    public:
        ShipSystemRollThrusters(Ship&);
        ~ShipSystemRollThrusters();

        void update(const double& dt);
};
class ShipSystemCloakingDevice final : public ShipSystem {
    friend class ::Ship;
    private:
        bool  m_Active;
        float m_CloakTimer;
    public:
        ShipSystemCloakingDevice(Ship&);
        ~ShipSystemCloakingDevice();

        const bool  isCloakActive() const;
        const float getCloakTimer() const;

        void update(const double& dt);
        bool cloak(ComponentModel&, bool sendPacket = true);
        bool decloak(ComponentModel&, bool sendPacket = true);
};
class ShipSystemShields final: public ShipSystem{
	friend class ::Ship;
    private:
        EntityWrapper m_ShieldEntity;
    public:
        ShipSystemShields(Ship&, Map*);
        ~ShipSystemShields();

        void update(const double& dt);
};
class ShipSystemWarpDrive final: public ShipSystem{
	friend class ::Ship;
    public:
        ShipSystemWarpDrive(Ship&);
        ~ShipSystemWarpDrive();

        void update(const double& dt);
};
class ShipSystemSensors final: public ShipSystem{
	friend class ::Ship;
    public:
        ShipSystemSensors(Ship&);
        ~ShipSystemSensors();

        void update(const double& dt);
};

class Ship: public EntityWrapper {
    friend struct ::ShipLogicFunctor;
    friend  class ::ShipSystemReactor;
    friend  class ::ShipSystemMainThrusters;
	friend  class ::ShipSystemYawThrusters;
	friend  class ::ShipSystemPitchThrusters;
	friend  class ::ShipSystemRollThrusters;
    friend  class ::ShipSystemCloakingDevice;
    friend  class ::ShipSystemWarpDrive;
    friend  class ::ShipSystemSensors;
    friend  class ::ShipSystemShields;
	friend  class ::ShipSystem;
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
