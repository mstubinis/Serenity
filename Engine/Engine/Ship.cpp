#include "Engine_Resources.h"
#include "Light.h"
#include "Ship.h"
#include "Engine_Events.h"
#include "GameCamera.h"
#include "SolarSystem.h"

#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>

using namespace Engine;
using namespace std;

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
                if(Engine::isKeyDown("w")){
                    m_Ship->applyForceZ(-1*(m_Ship->getMass()*3));
                }
                if(Engine::isKeyDown("s")){
                    m_Ship->applyForceZ(1*(m_Ship->getMass()*3));
                }
                if(Engine::isKeyDown("a")){
                    m_Ship->applyForceX(-1*(m_Ship->getMass()*3));
                }
                if(Engine::isKeyDown("d")){
                    m_Ship->applyForceX(1*(m_Ship->getMass()*3));
                }
                if(Engine::isKeyDown("f")){
                    m_Ship->applyForceY(-1*(m_Ship->getMass()*3));
                }
                if(Engine::isKeyDown("r")){
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
                m_Ship->applyTorqueX(-Engine::getMouseDifference().y*0.002f*(1/(m_Ship->getMass()*3)));
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
                m_Ship->applyTorqueY(-Engine::getMouseDifference().x*0.002f*(1/(m_Ship->getMass()*3)));
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
            if(Engine::isKeyDown("q")){
                m_Ship->applyTorqueZ(1*1/(m_Ship->getMass()));
            }
            if(Engine::isKeyDown("e")){
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
            if(Engine::isKeyDownOnce("l")){
                m_Ship->toggleWarp();
            }
            if(m_Ship->IsWarping()){
                if(Engine::isKeyDown("w")){
                    m_Ship->translateWarp(0.1f);
                }
                else if(Engine::isKeyDown("s")){
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

Ship::Ship(string mesh,string mat, bool player,string name,glm::vec3 pos, glm::vec3 scl, Collision* collision,Scene* scene): ObjectDynamic(mesh,mat,pos,scl,name,collision,scene){
    m_WarpFactor = 0;
    m_IsPlayer = player;
    m_IsWarping = false;
    m_Target = nullptr;
    m_PlayerCamera = nullptr;

    if(player){
        m_PlayerCamera = (GameCamera*)(scene->getActiveCamera());
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
            float speed = (m_WarpFactor * 1.0f/0.46f)*2.0f;
            glm::vec3 s = (getForward() * glm::pow(speed,15.0f))/getMass();
            for(auto obj:Resources::getCurrentScene()->objects()){
                if((obj.second->name().find("Camera") == string::npos) && obj.second != this && obj.second->parent() == nullptr){
                    obj.second->setPosition(obj.second->getPosition() + (s * dt));
                }
            }
        }

        #pragma endregion

        #pragma region PlayerCameraControls
        if(Engine::isKeyDownOnce("f1")){
            if(m_PlayerCamera->getState() != CAMERA_STATE_FOLLOW || (m_PlayerCamera->getState() == CAMERA_STATE_FOLLOW && m_PlayerCamera->getTarget() != this)){
                Resources::getCurrentScene()->centerSceneToObject(this);
                m_PlayerCamera->follow(this);
            }
        }
        else if(Engine::isKeyDownOnce("f2")){
            if(m_PlayerCamera->getState() == CAMERA_STATE_FOLLOW || m_Target == nullptr || m_PlayerCamera->getTarget() != this){
                Resources::getCurrentScene()->centerSceneToObject(this);
                m_PlayerCamera->orbit(this);
            }
            else if(m_Target != nullptr){
                Resources::getCurrentScene()->centerSceneToObject(m_Target);
                m_PlayerCamera->orbit(m_Target);
            }
        }
        else if(Engine::isKeyDownOnce("f3")){
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

        if(Engine::isKeyDownOnce("t") && Resources::getCurrentScene()->name() != "CapsuleSpace"){
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
void Ship::onEvent(const Event& e){

}