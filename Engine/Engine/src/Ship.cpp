#include "Ship.h"
#include "core/engine/Engine.h"
#include "core/Light.h"
#include "GameCamera.h"
#include "SolarSystem.h"
#include "ResourceManifest.h"

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
void ShipSystem::update(const float& dt){
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
void ShipSystemReactor::update(const float& dt){
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemShields
ShipSystemShields::ShipSystemShields(Ship* _ship):ShipSystem(SHIP_SYSTEM_SHIELDS, _ship){

}
ShipSystemShields::~ShipSystemShields(){

}
void ShipSystemShields::update(const float& dt){
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemMainThrusters
ShipSystemMainThrusters::ShipSystemMainThrusters(Ship* _ship):ShipSystem(SHIP_SYSTEM_MAIN_THRUSTERS, _ship){

}
ShipSystemMainThrusters::~ShipSystemMainThrusters(){

}
void ShipSystemMainThrusters::update(const float& dt){
    if(isOnline()){
        OLD_ComponentBody& body = *m_Ship->getComponent<OLD_ComponentBody>();
        glm::vec3 velocity = body.getLinearVelocity();
        // apply dampening
        body.setLinearVelocity(velocity * 0.9991f,false);
        if(m_Ship->IsPlayer()){
            if(!m_Ship->IsWarping()){
                float amount = (  (body.mass() * 0.4f)  +  1.3f  );
                if(Engine::isKeyDown(KeyboardKey::W)){ body.applyForce(0,0,-amount); }
                if(Engine::isKeyDown(KeyboardKey::S)){ body.applyForce(0,0, amount); }
                if(Engine::isKeyDown(KeyboardKey::A)){ body.applyForce(-amount,0,0); }
                if(Engine::isKeyDown(KeyboardKey::D)){ body.applyForce( amount,0,0); }
                if(Engine::isKeyDown(KeyboardKey::F)){ body.applyForce(0,-amount,0); }
                if(Engine::isKeyDown(KeyboardKey::R)){ body.applyForce(0, amount,0); }
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
void ShipSystemPitchThrusters::update(const float& dt){
    if(isOnline()){
        OLD_ComponentBody& body = *m_Ship->getComponent<OLD_ComponentBody>();
        glm::vec3 velocity = body.getAngularVelocity();
        velocity.x *= 0.9970f;
        // apply dampening
        body.setAngularVelocity(velocity,false);
        if(m_Ship->IsPlayer()){
            if(m_Ship->getPlayerCamera()->getState() != CAMERA_STATE_ORBIT){
                float mouseAmount = Engine::getMouseDifference().y * 0.02f;
                float massFactor = 1.0f / (body.mass() * 3.0f);
                float amount = mouseAmount * massFactor;
                body.applyTorque(-amount,0,0);
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
void ShipSystemYawThrusters::update(const float& dt){
    if(isOnline()){
        OLD_ComponentBody& body = *m_Ship->getComponent<OLD_ComponentBody>();
        glm::vec3 velocity = body.getAngularVelocity();
        velocity.y *= 0.9970f;
        // apply dampening
        body.setAngularVelocity(velocity,false);
        if(m_Ship->IsPlayer()){
            if(m_Ship->getPlayerCamera()->getState() != CAMERA_STATE_ORBIT){
                float mouseAmount = Engine::getMouseDifference().x * 0.02f;
                float massFactor = 1.0f / (body.mass() * 3.0f);
                float amount = mouseAmount * massFactor;
                body.applyTorque(0,-amount,0);
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
void ShipSystemRollThrusters::update(const float& dt){
    if(isOnline()){
        auto* body = m_Ship->getComponent<OLD_ComponentBody>();
        glm::vec3 velocity = body->getAngularVelocity();
        velocity.z *= 0.9970f;
        // apply dampening
        body->setAngularVelocity(velocity,false);
        if(m_Ship->IsPlayer()){
            float amount = 1.0f / body->mass();
            if(Engine::isKeyDown(KeyboardKey::Q)){
                body->applyTorque(0,0,amount);
            }
            if(Engine::isKeyDown(KeyboardKey::E)){
                body->applyTorque(0,0,-amount);
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
void ShipSystemWarpDrive::update(const float& dt){
    if(isOnline()){	
        if(!Engine::paused()){	
            if(m_Ship->IsPlayer()){
                if(Engine::isKeyDownOnce(KeyboardKey::L)){
                    m_Ship->toggleWarp();
                }
                if(m_Ship->IsWarping()){
                    if(Engine::isKeyDown(KeyboardKey::W)){
                        m_Ship->translateWarp(0.1f,dt);
                    }
                    else if(Engine::isKeyDown(KeyboardKey::S)){
                        m_Ship->translateWarp(-0.1f,dt);
                    }
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
void ShipSystemSensors::update(const float& dt){

    ShipSystem::update(dt);
}
#pragma endregion

Ship::Ship(Handle& mesh, Handle& mat, bool player, string name, glm::vec3 pos, glm::vec3 scl, CollisionType::Type _type,Scene* scene){
    scene->OLD_addEntity(*this);
    OLD_ComponentModel* modelComponent = new OLD_ComponentModel(mesh, mat, this);
    addComponent(modelComponent);
    OLD_ComponentBody* rigidBodyComponent = new OLD_ComponentBody(_type);
	addComponent(rigidBodyComponent);

    glm::vec3 boundingBox = modelComponent->boundingBox();
    float volume = boundingBox.x * boundingBox.y * boundingBox.z;

	rigidBodyComponent->setMass(  ( volume * 0.004f ) + 1.0f  );
	rigidBodyComponent->setPosition(pos);
	rigidBodyComponent->setScale(scl);
	rigidBodyComponent->setDamping(0, 0);//we dont want default dampening, we want the ship systems to manually control that

	m_WarpFactor = 0;
	m_IsPlayer = player;
	m_IsWarping = false;
	m_Target = nullptr;
	m_PlayerCamera = nullptr;

	if (player) {
		m_PlayerCamera = (GameCamera*)(scene->getActiveCamera());
	}
	for (uint i = 0; i < SHIP_SYSTEM_NUMBER; ++i) {
		ShipSystem* system = nullptr;
		if (i == 0)       system = new ShipSystemReactor(this, 1000);
		else if (i == 1)  system = new ShipSystemPitchThrusters(this);
		else if (i == 2)  system = new ShipSystemYawThrusters(this);
		else if (i == 3)  system = new ShipSystemRollThrusters(this);
		else if (i == 4)  system = new ShipSystemShields(this);
		else if (i == 5)  system = new ShipSystemMainThrusters(this);
		else if (i == 6)  system = new ShipSystemWarpDrive(this);
		else if (i == 7)  system = new ShipSystemSensors(this);
        m_ShipSystems.emplace(i, system);
	}
}
Ship::~Ship(){
	SAFE_DELETE_MAP(m_ShipSystems);
}
void Ship::update(const float& dt){
    Scene& currentScene = *Resources::getCurrentScene();
    if(m_IsPlayer){
        #pragma region PlayerFlightControls

        if(!Engine::paused()){
            if(m_IsWarping && m_WarpFactor > 0){
                auto* body = getComponent<OLD_ComponentBody>();
                float speed = (m_WarpFactor * 1.0f / 0.46f) * 2.0f;
                glm::vec3 s = (body->forward() * glm::pow(speed, 15.0f)) / body->mass();
                for(auto id: epriv::InternalScenePublicInterface::OLD_GetEntities(currentScene)){
                    OLD_Entity* e = currentScene.OLD_getEntity(id);
                    if(e){
                        auto* cam = e->getComponent<OLD_ComponentCamera>();
                        auto* camGame = (e->getComponent<GameCameraComponent>());
                        //TODO: parent->child relationship
                        if(e != this && !cam && !camGame){
                            auto* ebody = e->getComponent<OLD_ComponentBody>();
                            ebody->setPosition(ebody->position() + (s * dt));
                        }
                    }
                }
            }
        }
        #pragma endregion

        #pragma region PlayerCameraControls
        if(Engine::isKeyDownOnce(KeyboardKey::F1)){
            if(m_PlayerCamera->getState() != CAMERA_STATE_FOLLOW || (m_PlayerCamera->getState() == CAMERA_STATE_FOLLOW && m_PlayerCamera->getTarget() != this)){
                Resources::getCurrentScene()->centerSceneToObject(*this);
                m_PlayerCamera->follow(this);
            }
        }else if(Engine::isKeyDownOnce(KeyboardKey::F2)){
            if(m_PlayerCamera->getState() == CAMERA_STATE_FOLLOW || !m_Target || m_PlayerCamera->getTarget() != this){
                Resources::getCurrentScene()->centerSceneToObject(*this);
                m_PlayerCamera->orbit(this);
            }
            else if(m_Target){
                Resources::getCurrentScene()->centerSceneToObject(*m_Target);
                m_PlayerCamera->orbit(m_Target);
            }
        }else if(Engine::isKeyDownOnce(KeyboardKey::F3)){
            if(m_PlayerCamera->getState() == CAMERA_STATE_FOLLOWTARGET || (!m_Target && m_PlayerCamera->getState() != CAMERA_STATE_FOLLOW) || m_PlayerCamera->getTarget() != this){
                Resources::getCurrentScene()->centerSceneToObject(*this);
                m_PlayerCamera->follow(this);
            }else if(m_Target){
                Resources::getCurrentScene()->centerSceneToObject(*this);
                m_PlayerCamera->followTarget(m_Target,this);
            }
        }
        #pragma endregion

        if(Engine::isKeyDownOnce(KeyboardKey::T) && Resources::getCurrentScene()->name() != "CapsuleSpace"){
            setTarget(m_PlayerCamera->getObjectInCenterRay(this));
        }
    }
    for(auto shipSystem:m_ShipSystems) shipSystem.second->update(dt);
}
void Ship::translateWarp(float amount,float dt){
    float amountToAdd = amount * (1.0f / 0.5f);
    if((amount > 0 && m_WarpFactor + amount < 1) || (amount < 0 && m_WarpFactor > 0)){
        m_WarpFactor += amountToAdd * dt;
    }
}
void Ship::setTarget(OLD_Entity* target){
    m_Target = target;
}
void Ship::onEvent(const Event& e){

}