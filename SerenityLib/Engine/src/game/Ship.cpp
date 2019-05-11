#include "Ship.h"
#include "core/engine/Engine.h"
#include "core/engine/lights/Light.h"
#include "GameCamera.h"
#include "SolarSystem.h"
#include "ResourceManifest.h"

#include <ecs/ComponentName.h>

using namespace Engine;
using namespace std;

#pragma region ShipSystem
ShipSystem::ShipSystem(uint _type, Ship* _ship){
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
ShipSystemReactor::ShipSystemReactor(Ship* _ship, float maxPower, float currentPower):ShipSystem(ShipSystemType::Reactor,_ship){
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
ShipSystemShields::ShipSystemShields(Ship* _ship):ShipSystem(ShipSystemType::Shields, _ship){

}
ShipSystemShields::~ShipSystemShields(){

}
void ShipSystemShields::update(const float& dt){
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemMainThrusters
ShipSystemMainThrusters::ShipSystemMainThrusters(Ship* _ship):ShipSystem(ShipSystemType::ThrustersMain, _ship){

}
ShipSystemMainThrusters::~ShipSystemMainThrusters(){

}
void ShipSystemMainThrusters::update(const float& dt){
    if(isOnline()){
        auto& body = *m_Ship->entity().getComponent<ComponentBody>();
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
ShipSystemPitchThrusters::ShipSystemPitchThrusters(Ship* _ship):ShipSystem(ShipSystemType::ThrustersPitch, _ship){

}
ShipSystemPitchThrusters::~ShipSystemPitchThrusters(){

}
void ShipSystemPitchThrusters::update(const float& dt){
    if(isOnline()){
        auto& body = *m_Ship->entity().getComponent<ComponentBody>();
        glm::vec3 velocity = body.getAngularVelocity();
        velocity.x *= 0.9970f;
        // apply dampening
        body.setAngularVelocity(velocity,false);
        if(m_Ship->IsPlayer()){
            if(m_Ship->getPlayerCamera()->getState() != CameraState::Orbit){
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
ShipSystemYawThrusters::ShipSystemYawThrusters(Ship* _ship):ShipSystem(ShipSystemType::ThrustersYaw, _ship){

}
ShipSystemYawThrusters::~ShipSystemYawThrusters(){

}
void ShipSystemYawThrusters::update(const float& dt){
    if(isOnline()){
        auto& body = *m_Ship->entity().getComponent<ComponentBody>();
        glm::vec3 velocity = body.getAngularVelocity();
        velocity.y *= 0.9970f;
        // apply dampening
        body.setAngularVelocity(velocity,false);
        if(m_Ship->IsPlayer()){
            if(m_Ship->getPlayerCamera()->getState() != CameraState::Orbit){
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
ShipSystemRollThrusters::ShipSystemRollThrusters(Ship* _ship):ShipSystem(ShipSystemType::ThrustersRoll, _ship){

}
ShipSystemRollThrusters::~ShipSystemRollThrusters(){

}
void ShipSystemRollThrusters::update(const float& dt){
    if(isOnline()){
        auto& body = *m_Ship->entity().getComponent<ComponentBody>();
        glm::vec3 velocity = body.getAngularVelocity();
        velocity.z *= 0.9970f;
        // apply dampening
        body.setAngularVelocity(velocity,false);
        if(m_Ship->IsPlayer()){
            float amount = 1.0f / body.mass();
            if(Engine::isKeyDown(KeyboardKey::Q)){
                body.applyTorque(0,0,amount);
            }
            if(Engine::isKeyDown(KeyboardKey::E)){
                body.applyTorque(0,0,-amount);
            }
        }
    }
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemWarpDrive
ShipSystemWarpDrive::ShipSystemWarpDrive(Ship* _ship):ShipSystem(ShipSystemType::WarpDrive, _ship){

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
                    }else if(Engine::isKeyDown(KeyboardKey::S)){
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
ShipSystemSensors::ShipSystemSensors(Ship* _ship):ShipSystem(ShipSystemType::Sensors, _ship){

}
ShipSystemSensors::~ShipSystemSensors(){

}
void ShipSystemSensors::update(const float& dt){

    ShipSystem::update(dt);
}
#pragma endregion

struct ShipLogicFunctor final {void operator()(ComponentLogic& _component, const float& dt) const {
    Ship& ship = *(Ship*)_component.getUserPointer();
    Scene& currentScene = *Resources::getCurrentScene();

    if (ship.m_IsPlayer) {
        #pragma region PlayerFlightControls

        if (!Engine::paused()) {
            if (ship.m_IsWarping && ship.m_WarpFactor > 0) {
                auto& body = *ship.m_Entity.getComponent<ComponentBody>();
                float speed = (ship.m_WarpFactor * 1.0f / 0.46f) * 2.0f;
                glm::vec3 s = (body.forward() * glm::pow(speed, 15.0f)) / body.mass();
                for (auto& pod : epriv::InternalScenePublicInterface::GetEntities(currentScene)) {
                    Entity e = currentScene.getEntity(pod);
                    epriv::EntitySerialization _s(e);
                    auto* cam = e.getComponent<ComponentCamera>(_s);
                    //TODO: parent->child relationship
                    if (e != ship.m_Entity && !cam) {
                        auto& ebody = *e.getComponent<ComponentBody>(_s);
                        ebody.setPosition(ebody.position() + (s * dt));
                    }
                }
            }
        }
        #pragma endregion

        #pragma region PlayerCameraControls
        auto& camera = *ship.m_PlayerCamera;
        const auto& cameraState = camera.getState();
        const Entity& target = camera.getTarget();
        if (Engine::isKeyDownOnce(KeyboardKey::F1)) {
            if (cameraState != CameraState::Follow || (cameraState == CameraState::Follow && target != ship.m_Entity)) {
                currentScene.centerSceneToObject(ship.m_Entity);
                camera.follow(ship.m_Entity);
            }
        }else if (Engine::isKeyDownOnce(KeyboardKey::F2)) {
            if (cameraState == CameraState::Follow || ship.m_Target.null() || target != ship.m_Entity) {
                currentScene.centerSceneToObject(ship.m_Entity);
                camera.orbit(ship.m_Entity);
            }else if (!ship.m_Target.null()) {
                currentScene.centerSceneToObject(ship.m_Target);
                camera.orbit(ship.m_Target);
            }
        }else if (Engine::isKeyDownOnce(KeyboardKey::F3)) {
            if (cameraState == CameraState::FollowTarget || (ship.m_Target.null() && cameraState != CameraState::Follow) || target != ship.m_Entity) {
                currentScene.centerSceneToObject(ship.m_Entity);
                camera.follow(ship.m_Entity);
            }else if (!ship.m_Target.null()) {
                currentScene.centerSceneToObject(ship.m_Entity);
                camera.followTarget(ship.m_Target, ship.m_Entity);
            }
        }
        #pragma endregion

        if (Engine::isKeyDownOnce(KeyboardKey::T) && currentScene.name() != "CapsuleSpace") {
            Entity scan = camera.getObjectInCenterRay(ship.m_Entity);
            if (!scan.null()) {
                ship.setTarget(scan);
            }
        }
    }
    for (auto& shipSystem : ship.m_ShipSystems) shipSystem.second->update(dt);
}};


Ship::Ship(Handle& mesh, Handle& mat, bool player, string name, glm::vec3 pos, glm::vec3 scl, CollisionType::Type _type,SolarSystem* scene):EntityWrapper(*scene){
    auto* modelComponent = m_Entity.addComponent<ComponentModel>(mesh, mat);
    auto& rigidBodyComponent = *m_Entity.addComponent<ComponentBody>(_type);
    m_Entity.addComponent<ComponentLogic>(ShipLogicFunctor(), this);

    glm::vec3 boundingBox = modelComponent->boundingBox();
    float volume = boundingBox.x * boundingBox.y * boundingBox.z;

	rigidBodyComponent.setMass(  ( volume * 0.004f ) + 1.0f  );
	rigidBodyComponent.setPosition(pos);
	rigidBodyComponent.setScale(scl);
	rigidBodyComponent.setDamping(0, 0);//we dont want default dampening, we want the ship systems to manually control that

	m_WarpFactor = 0;
	m_IsPlayer = player;
	m_IsWarping = false;
	m_Target = Entity::_null;
	m_PlayerCamera = nullptr;

	if (player) {
		m_PlayerCamera = (GameCamera*)(scene->getActiveCamera());
	}
	for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
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
    scene->m_Objects.push_back(this);
}
Ship::~Ship(){
	SAFE_DELETE_MAP(m_ShipSystems);
}
void Ship::translateWarp(float amount,float dt){
    float amountToAdd = amount * (1.0f / 0.5f);
    if((amount > 0 && m_WarpFactor + amount < 1) || (amount < 0 && m_WarpFactor > 0)){
        m_WarpFactor += amountToAdd * dt;
    }
}
void Ship::setTarget(const string& target) {
    SolarSystem* s = (SolarSystem*)Resources::getCurrentScene();
    for (auto& entity : s->m_Objects) {
        auto* componentName = entity->entity().getComponent<ComponentName>();
        if (componentName) {
            if (componentName->name() == target) {
                m_Target = entity->entity();
                return;
            }
        }
    }
}
void Ship::setTarget(const Entity& target){
    m_Target = target;
}
void Ship::onEvent(const Event& e){

}