#include "Engine_Resources.h"
#include "Light.h"
#include "Ship.h"
#include "Engine_Events.h"
#include "GameCamera.h"
#include "SolarSystem.h"
#include "Particles.h"
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>

using namespace Engine;
using namespace Engine::Events;

#pragma region ShipSystem
ShipSystem::ShipSystem(unsigned int _type, Ship* _ship){
    m_Ship = _ship;
    m_Health = 1.0f;
    m_Power = 1.0f;
    m_Type = _type;
}
ShipSystem::~ShipSystem(){
    m_Health = 0.0f;
    m_Power = 0.0f;
}
void ShipSystem::update(float dt){
    // handle power transfers...?
}
#pragma endregion

#pragma region ShipSystemReactor
ShipSystemReactor::ShipSystemReactor(Ship* _ship, float maxPower, float currentPower):ShipSystem(SHIP_SYSTEM_REACTOR,_ship){
    if( currentPower == -1){
        m_TotalPower = maxPower;
    }
    else{
        m_TotalPower = currentPower;
    }
    m_TotalPowerMax = maxPower;
}
ShipSystemReactor::~ShipSystemReactor(){

}
void ShipSystemReactor::update(float dt){
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemShields
ShipSystemShields::ShipSystemShields(Ship* _ship):ShipSystem(SHIP_SYSTEM_SHIELDS, _ship){

}
ShipSystemShields::~ShipSystemShields(){

}
void ShipSystemShields::update(float dt){
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemMainThrusters
ShipSystemMainThrusters::ShipSystemMainThrusters(Ship* _ship):ShipSystem(SHIP_SYSTEM_MAIN_THRUSTERS, _ship){

}
ShipSystemMainThrusters::~ShipSystemMainThrusters(){

}
void ShipSystemMainThrusters::update(float dt){
    if(isOnline()){
        btVector3 velocity = m_Ship->getRigidBody()->getLinearVelocity();
        // apply dampening
        m_Ship->getRigidBody()->setLinearVelocity(velocity * 0.9993f);

        if(m_Ship->IsPlayer()){
            if(!m_Ship->IsWarping()){
                if(Keyboard::isKeyDown("w")){
                    m_Ship->applyForceZ(-1*(m_Ship->getMass()*3));
                }
                if(Keyboard::isKeyDown("s")){
                    m_Ship->applyForceZ(1*(m_Ship->getMass()*3));
                }
                if(Keyboard::isKeyDown("a")){
                    m_Ship->applyForceX(-1*(m_Ship->getMass()*3));
                }
                if(Keyboard::isKeyDown("d")){
                    m_Ship->applyForceX(1*(m_Ship->getMass()*3));
                }
                if(Keyboard::isKeyDown("f")){
                    m_Ship->applyForceY(-1*(m_Ship->getMass()*3));
                }
                if(Keyboard::isKeyDown("r")){
                    m_Ship->applyForceY(1*(m_Ship->getMass()*3));
                }
            }
        }
    }
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemPitchThrusters
ShipSystemPitchThrusters::ShipSystemPitchThrusters(Ship* _ship):ShipSystem(SHIP_SYSTEM_PITCH_THRUSTERS, _ship){

}
ShipSystemPitchThrusters::~ShipSystemPitchThrusters(){

}
void ShipSystemPitchThrusters::update(float dt){
    if(isOnline()){
        btVector3 velocity = m_Ship->getRigidBody()->getAngularVelocity();
        velocity.setX(velocity.x() * 0.9985f);
        // apply dampening
        m_Ship->getRigidBody()->setAngularVelocity(velocity);
        if(m_Ship->IsPlayer()){
            if(m_Ship->getPlayerCamera()->getState() != CAMERA_STATE_ORBIT){
                m_Ship->applyTorqueX(-Mouse::getMouseDifference().y*0.002f*(1/(m_Ship->getMass()*3)));
            }
        }
    }


    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemYawThrusters
ShipSystemYawThrusters::ShipSystemYawThrusters(Ship* _ship):ShipSystem(SHIP_SYSTEM_YAW_THRUSTERS, _ship){

}
ShipSystemYawThrusters::~ShipSystemYawThrusters(){

}
void ShipSystemYawThrusters::update(float dt){
    if(isOnline()){
        btVector3 velocity = m_Ship->getRigidBody()->getAngularVelocity();
        velocity.setY(velocity.y() * 0.9985f);
        // apply dampening
        m_Ship->getRigidBody()->setAngularVelocity(velocity);
        if(m_Ship->IsPlayer()){
            if(m_Ship->getPlayerCamera()->getState() != CAMERA_STATE_ORBIT){
                m_Ship->applyTorqueY(-Mouse::getMouseDifference().x*0.002f*(1/(m_Ship->getMass()*3)));
            }
        }
    }

    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemRollThrusters
ShipSystemRollThrusters::ShipSystemRollThrusters(Ship* _ship):ShipSystem(SHIP_SYSTEM_ROLL_THRUSTERS, _ship){

}
ShipSystemRollThrusters::~ShipSystemRollThrusters(){

}
void ShipSystemRollThrusters::update(float dt){
    if(isOnline()){
        btVector3 velocity = m_Ship->getRigidBody()->getAngularVelocity();
        velocity.setZ(velocity.z() * 0.9985f);
        // apply dampening
        m_Ship->getRigidBody()->setAngularVelocity(velocity);
        if(m_Ship->IsPlayer()){
            if(Keyboard::isKeyDown("q")){
                m_Ship->applyTorqueZ(1*1/(m_Ship->getMass()));
            }
            if(Keyboard::isKeyDown("e")){
                m_Ship->applyTorqueZ(-1*1/(m_Ship->getMass()));
            }
        }
    }
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemWarpDrive
ShipSystemWarpDrive::ShipSystemWarpDrive(Ship* _ship):ShipSystem(SHIP_SYSTEM_WARP_DRIVE, _ship){

}
ShipSystemWarpDrive::~ShipSystemWarpDrive(){

}
void ShipSystemWarpDrive::update(float dt){
    if(isOnline()){
        if(m_Ship->IsPlayer()){
            if(Keyboard::isKeyDownOnce("l")){
                m_Ship->toggleWarp();
            }
            if(m_Ship->IsWarping()){
                if(Keyboard::isKeyDown("w")){
                    m_Ship->translateWarp(0.1f);
                }
                else if(Keyboard::isKeyDown("s")){
                    m_Ship->translateWarp(-0.1f);
                }
            }
        }
    }
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemSensors
ShipSystemSensors::ShipSystemSensors(Ship* _ship):ShipSystem(SHIP_SYSTEM_SENSORS, _ship){

}
ShipSystemSensors::~ShipSystemSensors(){

}
void ShipSystemSensors::update(float dt){

    ShipSystem::update(dt);
}
#pragma endregion

Ship::Ship(std::string mesh, std::string mat, bool player, std::string name,glm::v3 pos, glm::vec3 scl, Collision* collision,Scene* scene): ObjectDynamic(mesh,mat,pos,scl,name,collision,scene){
    m_WarpFactor = 0;
    m_IsPlayer = player;
    m_IsWarping = false;
    m_Target = nullptr;
    m_PlayerCamera = nullptr;

    if(player){
        m_PlayerCamera = static_cast<GameCamera*>(Resources::getActiveCamera());
    }

    for(unsigned int i = 0; i < SHIP_SYSTEM_NUMBER; i++){
        ShipSystem* system = nullptr;
        if(i == 0)       system = new ShipSystemReactor(this,1000);
        else if(i == 1)  system = new ShipSystemPitchThrusters(this);
        else if(i == 2)  system = new ShipSystemYawThrusters(this);
        else if(i == 3)  system = new ShipSystemRollThrusters(this);
        else if(i == 4)  system = new ShipSystemShields(this);
        else if(i == 5)  system = new ShipSystemMainThrusters(this);
        else if(i == 6)  system = new ShipSystemWarpDrive(this);
        else if(i == 7)  system = new ShipSystemSensors(this);
        m_ShipSystems[i] = system;
    }
    m_RigidBody->setDamping(0,0);//we dont want default dampening, we want the ship systems to manually control that
}
Ship::~Ship(){
    for(auto shipSystem:m_ShipSystems) SAFE_DELETE(shipSystem.second);
}
void Ship::update(float dt){
    if(m_IsPlayer){

        #pragma region PlayerFlightControls
        if(m_IsWarping && m_WarpFactor > 0){
            glm::nType speed = (m_WarpFactor * static_cast<glm::nType>(1.0L)/static_cast<glm::nType>(0.46L))*static_cast<glm::nType>(2.0L);
            glm::v3 s = (getForward() * glm::pow(speed,static_cast<glm::nType>(15.0L)))/static_cast<glm::nType>(getMass());
            for(auto obj:Resources::getCurrentScene()->getObjects()){
                if((obj.second->getName().find("Skybox") == std::string::npos) && (obj.second->getName().find("Camera") == std::string::npos) && obj.second != this && obj.second->getParent() == nullptr){
                    obj.second->setPosition(obj.second->getPosition() + (s * static_cast<glm::nType>(dt)));
                }
            }
        }

        #pragma endregion

        #pragma region PlayerCameraControls
        if(Keyboard::isKeyDownOnce("f1")){
            if(m_PlayerCamera->getState() != CAMERA_STATE_FOLLOW || (m_PlayerCamera->getState() == CAMERA_STATE_FOLLOW && m_PlayerCamera->getTarget() != this)){
                Resources::getCurrentScene()->centerSceneToObject(this);
                m_PlayerCamera->follow(this);
            }
        }
        else if(Keyboard::isKeyDownOnce("f2")){
            if(m_PlayerCamera->getState() == CAMERA_STATE_FOLLOW || m_Target == nullptr || m_PlayerCamera->getTarget() != this){
                Resources::getCurrentScene()->centerSceneToObject(this);
                m_PlayerCamera->orbit(this);
            }
            else if(m_Target != nullptr){
                Resources::getCurrentScene()->centerSceneToObject(m_Target);
                m_PlayerCamera->orbit(m_Target);
            }
        }
        else if(Keyboard::isKeyDownOnce("f3")){
            if(m_PlayerCamera->getState() == CAMERA_STATE_FOLLOWTARGET || (m_Target == nullptr && m_PlayerCamera->getState() != CAMERA_STATE_FOLLOW) || m_PlayerCamera->getTarget() != this){
                Resources::getCurrentScene()->centerSceneToObject(this);
                m_PlayerCamera->follow(this);
            }
            else if(m_Target != nullptr){
                Resources::getCurrentScene()->centerSceneToObject(this);
                m_PlayerCamera->followTarget(m_Target,this);
            }
        }
        #pragma endregion

        if(Keyboard::isKeyDownOnce("t") && Resources::getCurrentScene()->getName() != "CapsuleSpace"){
            setTarget(m_PlayerCamera->getObjectInCenterRay(this));
        }
    }
    else{
    }

    for(auto shipSystem:m_ShipSystems) shipSystem.second->update(dt);

    ObjectDynamic::update(dt);
}
void Ship::translateWarp(float amount){
    float amountToAdd = amount * (1.0f / 0.5f);
    if((amount > 0 && m_WarpFactor + amount < 1) || (amount < 0 && m_WarpFactor > 0)){
        m_WarpFactor += amountToAdd * Resources::dt();
    }
}
void Ship::setTarget(Object* target){
    m_Target = target;
}
void Ship::setTarget(std::string target){
    Ship::setTarget(Resources::getObject(target));
}
