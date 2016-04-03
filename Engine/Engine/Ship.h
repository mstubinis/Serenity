#ifndef SHIP_H
#define SHIP_H

#include "ObjectDynamic.h"

class GameCamera;
class Ship;

enum SHIP_SYSTEM_TYPE { 
    SHIP_SYSTEM_REACTOR,
    SHIP_SYSTEM_PITCH_THRUSTERS,
    SHIP_SYSTEM_YAW_THRUSTERS,
    SHIP_SYSTEM_ROLL_THRUSTERS,
    SHIP_SYSTEM_SHIELDS,
    SHIP_SYSTEM_MAIN_THRUSTERS,
    SHIP_SYSTEM_WARP_DRIVE,
    SHIP_SYSTEM_SENSORS,
SHIP_SYSTEM_NUMBER};

class ShipSystem{
    protected:
        Ship* m_Ship;
        unsigned int m_Type;
        float m_Health;
        float m_Power;
    public:
        ShipSystem(unsigned int type,Ship*);
        virtual ~ShipSystem();

        const bool isOnline() const { if(m_Health > 0 && m_Power > 0) return true; return false; }

        virtual void update(float);
};
class ShipSystemReactor final: public ShipSystem{
    private:
        float m_TotalPowerMax;
        float m_TotalPower;
    public:
        ShipSystemReactor(Ship*, float maxPower, float currentPower = -1);
        ~ShipSystemReactor();

        void update(float);
};
class ShipSystemMainThrusters final: public ShipSystem{
    public:
        ShipSystemMainThrusters(Ship*);
        ~ShipSystemMainThrusters();

        void update(float);
};
class ShipSystemPitchThrusters final: public ShipSystem{
    public:
        ShipSystemPitchThrusters(Ship*);
        ~ShipSystemPitchThrusters();

        void update(float);
};
class ShipSystemYawThrusters final: public ShipSystem{
    public:
        ShipSystemYawThrusters(Ship*);
        ~ShipSystemYawThrusters();

        void update(float);
};
class ShipSystemRollThrusters final: public ShipSystem{
    public:
        ShipSystemRollThrusters(Ship*);
        ~ShipSystemRollThrusters();

        void update(float);
};
class ShipSystemShields final: public ShipSystem{
    public:
        ShipSystemShields(Ship*);
        ~ShipSystemShields();

        void update(float);
};
class ShipSystemWarpDrive final: public ShipSystem{
    public:
        ShipSystemWarpDrive(Ship*);
        ~ShipSystemWarpDrive();

        void update(float);
};
class ShipSystemSensors final: public ShipSystem{
    public:
        ShipSystemSensors(Ship*);
        ~ShipSystemSensors();

        void update(float);
};

class Ship: public ObjectDynamic{
    protected:
        std::unordered_map<unsigned int,ShipSystem*> m_ShipSystems;

        bool m_IsPlayer;
        GameCamera* m_PlayerCamera;

        bool m_IsWarping;
        float m_WarpFactor;
        Object* m_Target;
    public:
        Ship(
            std::string,              //Mesh
            std::string,              //Material
            bool = false,             //Player Ship?
            std::string = "Ship",     //Name
            glm::v3 = glm::v3(0),     //Position
            glm::vec3 = glm::vec3(1), //Scale
            Collision* = nullptr,     //Bullet Collision Shape
            Scene* = nullptr
            );
        virtual ~Ship();

        void translateWarp(float);

        void toggleWarp(){
            m_IsWarping = !m_IsWarping;
            m_WarpFactor = 0;
        }

        GameCamera* getPlayerCamera(){ return m_PlayerCamera; }
        bool IsPlayer(){ return m_IsPlayer; }
        bool IsWarping(){ return m_IsWarping; }
        ShipSystem* getShipSystem(unsigned int type){ return m_ShipSystems[type]; }

        Object* getTarget() { return m_Target; }
        virtual void setTarget(Object*);
        virtual void setTarget(std::string);
        virtual void update(float);
};
#endif