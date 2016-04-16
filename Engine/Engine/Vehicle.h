#pragma once
#ifndef ENGINE_VEHICLE_H
#define ENGINE_VEHICLE_H

#include "ObjectDynamic.h"
#include "ObjectDisplay.h"
#include <bullet/BulletDynamics/Vehicle/btRaycastVehicle.h>
class btDefaultVehicleRaycaster;

class Wheel: public ObjectDynamic{
    protected:
    public:
        Wheel(std::string mesh,
              std::string mat,
              glm::v3 pos = glm::v3(0),
              glm::vec3 scl = glm::vec3(1),
              std::string name = "Wheel",
              Collision* = nullptr,
              Scene* = nullptr);
        virtual ~Wheel();

        virtual void on_applyForce(float,float,float,bool local=true);
        virtual void on_setLinearVelocity(float,float,float,bool local=true);
        virtual void on_setAngularVelocity(float,float,float,bool local=true); 
};

class Vehicle: public ObjectDynamic{
    private:
    protected:
        std::vector<Wheel*> m_Wheels;
        btRaycastVehicle* m_Vehicle;
        btRaycastVehicle::btVehicleTuning* m_Tuning;
        btDefaultVehicleRaycaster* m_Raycaster;
    public:
        Vehicle(std::string mesh,
                std::string mat,
                glm::v3 pos = glm::v3(0),
                glm::vec3 scl = glm::vec3(1),
                std::string name = "Vehicle",
                Collision* = nullptr,
                Scene* = nullptr);
        virtual ~Vehicle();

        virtual void update(float dt);

        virtual void applyEngineForce(float force, unsigned int wheelIndex);
        virtual void setBrake(float force, unsigned int wheelIndex);
        virtual void setSteeringValue(float value, unsigned int wheelIndex);

        virtual void setFrictionSlip(float);
        virtual void setMaxSuspensionForce(float);
        virtual void setMaxSuspensionTravelCm(float);
        virtual void setSuspensionCompression(float);
        virtual void setSuspensionDamping(float);
        virtual void setSuspensionStiffness(float);
        virtual void resetSuspension();
        virtual float getCurrentSpeedKmHour();

        virtual glm::v3 getForward(){ return m_Forward; }
        virtual glm::v3 getRight(){ return m_Right; }
        virtual glm::v3 getUp(){ return m_Up; }

        virtual void addWheel(Wheel*,glm::v3 pos,float suspensionRestLength = 0,bool isFront = true);

        virtual void applyForce(float,float,float,bool local=true);
        virtual void applyForce(glm::vec3,glm::vec3 = glm::vec3(0),bool local=true);
        virtual void applyForceX(float,bool local=true);
        virtual void applyForceY(float,bool local=true);
        virtual void applyForceZ(float,bool local=true);

        virtual void setLinearVelocity(float,float,float,bool local=true); 
        virtual void setLinearVelocity(glm::vec3,bool local=true);
        virtual void setLinearVelocityX(float,bool local=true);
        virtual void setLinearVelocityY(float,bool local=true);
        virtual void setLinearVelocityZ(float,bool local=true);

        virtual void setAngularVelocity(float,float,float,bool local=true); 
        virtual void setAngularVelocity(glm::vec3,bool local=true);
        virtual void setAngularVelocityX(float,bool local=true);
        virtual void setAngularVelocityY(float,bool local=true);
        virtual void setAngularVelocityZ(float,bool local=true);
};

#endif