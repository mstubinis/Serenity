#include "Ship.h"
#include "GameCamera.h"
#include "map/Map.h"
#include "Packet.h"
#include "Helper.h"
#include "map/Anchor.h"
#include "ResourceManifest.h"

#include <core/engine/Engine.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/materials/Material.h>

#include <core/engine/utils/Engine_Debugging.h>

#include <ecs/ComponentName.h>

using namespace Engine;
using namespace std;

#pragma region ShipSystem
ShipSystem::ShipSystem(const uint& _type, Ship& _ship):m_Ship(_ship){
    m_Health = 1.0f;
    m_Power = 1.0f;
    m_Type = _type;
}
ShipSystem::~ShipSystem(){
    m_Health = 0.0f;
    m_Power = 0.0f;
}
void ShipSystem::update(const double& dt){
    // handle power transfers...?
}
#pragma endregion

#pragma region ShipSystemReactor
ShipSystemReactor::ShipSystemReactor(Ship& _ship, const float maxPower, const float currentPower):ShipSystem(ShipSystemType::Reactor,_ship){
    if( currentPower == -1){
        m_TotalPower = maxPower;
    }else{
        m_TotalPower = currentPower;
    }
    m_TotalPowerMax = maxPower;
}
ShipSystemReactor::~ShipSystemReactor(){

}
void ShipSystemReactor::update(const double& dt){
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemShields
ShipSystemShields::ShipSystemShields(Ship& _ship):ShipSystem(ShipSystemType::Shields, _ship){

}
ShipSystemShields::~ShipSystemShields(){

}
void ShipSystemShields::update(const double& dt){
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemMainThrusters
ShipSystemMainThrusters::ShipSystemMainThrusters(Ship& _ship):ShipSystem(ShipSystemType::ThrustersMain, _ship){

}
ShipSystemMainThrusters::~ShipSystemMainThrusters(){

}
void ShipSystemMainThrusters::update(const double& dt){
    auto& rigidbody = *m_Ship.getComponent<ComponentBody>();
    if(isOnline()){
        if(m_Ship.IsPlayer()){
            bool ismoving = false;
            if(!m_Ship.IsWarping()){
                const float& amount =  1.3f / ((rigidbody.mass()* 0.15f) + 1.0f);  
                if(Engine::isKeyDown(KeyboardKey::W)){ 
                    rigidbody.applyForce(0,0,-amount);
                    ismoving = true;
                }
                if(Engine::isKeyDown(KeyboardKey::S)){ 
                    rigidbody.applyForce(0,0, amount); 
                    ismoving = true;
                }
                if(Engine::isKeyDown(KeyboardKey::A)){ 
                    rigidbody.applyForce(-amount,0,0); 
                    ismoving = true;
                }
                if(Engine::isKeyDown(KeyboardKey::D)){ 
                    rigidbody.applyForce( amount,0,0);
                    ismoving = true;
                }
                if(Engine::isKeyDown(KeyboardKey::F)){ 
                    rigidbody.applyForce(0,-amount,0); 
                    ismoving = true;
                }
                if(Engine::isKeyDown(KeyboardKey::R)){ 
                    rigidbody.applyForce(0, amount,0); 
                    ismoving = true;
                }
            }
            if (!ismoving) {
                auto current = rigidbody.getLinearVelocity();
                const_cast<btRigidBody&>(rigidbody.getBody()).setDamping(0.05f, 0.2f);
            }else{
                const_cast<btRigidBody&>(rigidbody.getBody()).setDamping(0.0f, 0.2f);
            }
        }
    }else{
        const_cast<btRigidBody&>(rigidbody.getBody()).setDamping(0.0f, 0.2f);
    }
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemPitchThrusters
ShipSystemPitchThrusters::ShipSystemPitchThrusters(Ship& _ship):ShipSystem(ShipSystemType::ThrustersPitch, _ship){

}
ShipSystemPitchThrusters::~ShipSystemPitchThrusters(){

}
void ShipSystemPitchThrusters::update(const double& dt){
    if(isOnline()){
        auto& rigidbody = *m_Ship.getComponent<ComponentBody>();
        if(m_Ship.IsPlayer()){
            if(m_Ship.getPlayerCamera()->getState() != CameraState::Orbit){
				const auto& diff = Engine::getMouseDifference().y;
                m_Ship.m_MouseFactor.y += diff * 0.00065;
				const float& massFactor = 1.0f / (rigidbody.mass() * 5.0f);
				const float& amount = m_Ship.m_MouseFactor.y * massFactor;
				rigidbody.applyTorque(amount, 0, 0);
				const double& step = (1.0 - dt);
                m_Ship.m_MouseFactor.y *= (step * step);
            }
        }
    }
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemYawThrusters
ShipSystemYawThrusters::ShipSystemYawThrusters(Ship& _ship):ShipSystem(ShipSystemType::ThrustersYaw, _ship){

}
ShipSystemYawThrusters::~ShipSystemYawThrusters(){

}
void ShipSystemYawThrusters::update(const double& dt){
    if(isOnline()){
        auto& rigidbody = *m_Ship.getComponent<ComponentBody>();
        if(m_Ship.IsPlayer()){
            if(m_Ship.getPlayerCamera()->getState() != CameraState::Orbit){
				const auto& diff = -Engine::getMouseDifference().x;
                m_Ship.m_MouseFactor.x += diff * 0.00065;
				const float& massFactor = 1.0f / (rigidbody.mass() * 5.0f);
				const float& amount = m_Ship.m_MouseFactor.x * massFactor;
				rigidbody.applyTorque(0, amount, 0);
				const double& step = (1.0 - dt);
                m_Ship.m_MouseFactor.x *= (step * step);
            }
        }
    }
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemRollThrusters
ShipSystemRollThrusters::ShipSystemRollThrusters(Ship& _ship):ShipSystem(ShipSystemType::ThrustersRoll, _ship){

}
ShipSystemRollThrusters::~ShipSystemRollThrusters(){

}
void ShipSystemRollThrusters::update(const double& dt){
    if(isOnline()){
        auto& rigidbody = *m_Ship.getComponent<ComponentBody>();
        if(m_Ship.IsPlayer()){
            float amount = 1.0f / rigidbody.mass();
            if(Engine::isKeyDown(KeyboardKey::Q)){
				rigidbody.applyTorque(0,0,amount);
            }
            if(Engine::isKeyDown(KeyboardKey::E)){
				rigidbody.applyTorque(0,0,-amount);
            }
        }
    }
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemCloakingDevice
ShipSystemCloakingDevice::ShipSystemCloakingDevice(Ship& _ship) :ShipSystem(ShipSystemType::CloakingDevice, _ship) {
    m_Active = false;
    m_CloakTimer = 1.0f;
}
ShipSystemCloakingDevice::~ShipSystemCloakingDevice() {

}
const bool  ShipSystemCloakingDevice::isCloakActive() const {
    return m_Active;
}
const float ShipSystemCloakingDevice::getCloakTimer() const {
    return m_CloakTimer;
}
bool ShipSystemCloakingDevice::cloak(ComponentModel& model, bool sendPacket) {
    if (m_CloakTimer >= 1.0f) {
        model.setModelShaderProgram(ShaderProgram::Forward, 0, RenderStage::ForwardTransparentTrianglesSorted);
        m_Active = true;
        auto effect = Sound::playEffect(ResourceManifest::SoundCloakingActivated);
        auto& body = *m_Ship.getComponent<ComponentBody>();
        effect->setPosition(body.position());
        effect->setAttenuation(0.15f);
        if (sendPacket) {
            PacketCloakUpdate pOut(m_Ship);
            pOut.PacketType = PacketType::Client_To_Server_Ship_Cloak_Update;
            pOut.justTurnedOn = true;
            m_Ship.m_Client.send(pOut);
        }
        return true;
    }
    return false;
}
bool ShipSystemCloakingDevice::decloak(ComponentModel& model, bool sendPacket) {
    if (m_CloakTimer <= 0.0f) {
        m_Active = false;
        m_CloakTimer = 0.0f;
        model.show();
        auto effect = Sound::playEffect(ResourceManifest::SoundCloakingDeactivated);
        auto& body = *m_Ship.getComponent<ComponentBody>();
        effect->setPosition(body.position());
        effect->setAttenuation(0.15f);
        if (sendPacket) {
            PacketCloakUpdate pOut(m_Ship);
            pOut.PacketType = PacketType::Client_To_Server_Ship_Cloak_Update;
            pOut.justTurnedOff = true;
            m_Ship.m_Client.send(pOut);
        }
        return true;
    }
    return false;
}
void ShipSystemCloakingDevice::update(const double& dt) {
    auto _fdt = static_cast<float>(dt) * 0.37f;
    auto& model = *m_Ship.getComponent<ComponentModel>();
    auto& instance = model.getModel(0);
    if (isOnline()) {
        if (m_Ship.IsPlayer()) {
            if (Engine::isKeyDownOnce(KeyboardKey::C)) {
                if (!m_Active) {
                    ShipSystemCloakingDevice::cloak(model);
                }else{
                    ShipSystemCloakingDevice::decloak(model);
                }
            }
        } 
        if (m_Active) {
            if (m_Ship.canSeeCloak()) {
                if (m_CloakTimer > -0.2f) {
                    if (m_CloakTimer > 0.0f) {
                        m_CloakTimer -= _fdt;
                        instance.setColor(1, 1, 1, glm::abs(m_CloakTimer));
                    }else{
                        m_CloakTimer -= _fdt * 0.35f;
                        if (m_CloakTimer < -0.2f) {
                            m_CloakTimer = -0.2f;
                        }
                        instance.setColor(0.369f, 0.912f, 1, glm::abs(m_CloakTimer));
                    }
                }
            }else{
                if (m_CloakTimer > 0.0f) {
                    m_CloakTimer -= _fdt;
                    if (m_CloakTimer < 0.0f) {
                        m_CloakTimer = 0.0f;
                        model.hide();
                    }
                    instance.setColor(1, 1, 1, glm::abs(m_CloakTimer));
                }
            } 
        }else{
            if (m_CloakTimer < 1.0f) {
                model.show();
                m_CloakTimer += _fdt;
                if (m_CloakTimer > 1.0f) {
                    m_CloakTimer = 1.0f;
                    model.setModelShaderProgram(ShaderProgram::Deferred, 0, RenderStage::GeometryOpaque);
                }
            }
            instance.setColor(1, 1, 1, glm::abs(m_CloakTimer));
        }
    }else{
        if (m_CloakTimer <= 0.0f) {
            ShipSystemCloakingDevice::decloak(model);
        }
        m_Active = false;
        if (m_CloakTimer < 1.0f) {
            m_CloakTimer += _fdt;
            if (m_CloakTimer > 1.0f) {
                m_CloakTimer = 1.0f;
                model.setModelShaderProgram(ShaderProgram::Deferred, 0, RenderStage::GeometryOpaque);
            }
        }
        instance.setColor(1, 1, 1, glm::abs(m_CloakTimer));
    }
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemWarpDrive
ShipSystemWarpDrive::ShipSystemWarpDrive(Ship& _ship):ShipSystem(ShipSystemType::WarpDrive, _ship){

}
ShipSystemWarpDrive::~ShipSystemWarpDrive(){

}
void ShipSystemWarpDrive::update(const double& dt){
    if(isOnline()){	
        if(!Engine::paused()){	
            if(m_Ship.IsPlayer()){
                if(Engine::isKeyDownOnce(KeyboardKey::L)){
                    m_Ship.toggleWarp();
                }
                if(m_Ship.IsWarping()){
                    if(Engine::isKeyDown(KeyboardKey::W)){
                        m_Ship.translateWarp(0.1, dt);
                    }else if(Engine::isKeyDown(KeyboardKey::S)){
                        m_Ship.translateWarp(-0.1, dt);
                    }
                }
            }
        }
    }
    ShipSystem::update(dt);
}
#pragma endregion

#pragma region ShipSystemSensors
ShipSystemSensors::ShipSystemSensors(Ship& _ship):ShipSystem(ShipSystemType::Sensors, _ship){

}
ShipSystemSensors::~ShipSystemSensors(){

}
void ShipSystemSensors::update(const double& dt){
    if (m_Ship.getTarget()) {
        Ship* target = dynamic_cast<Ship*>(m_Ship.getTarget());
        if (target) {
            if (target->isFullyCloaked()) {
                m_Ship.setTarget(nullptr);
            }
        }
    }
    ShipSystem::update(dt);
}
#pragma endregion

struct ShipLogicFunctor final {void operator()(ComponentLogic& _component, const double& dt) const {
    Ship& ship = *static_cast<Ship*>(_component.getUserPointer());
    Map& currentScene = *static_cast<Map*>(Resources::getCurrentScene());

    if (ship.m_IsPlayer) {
        #pragma region PlayerFlightControls

        if (!Engine::paused()) {
            if (ship.m_IsWarping && ship.m_WarpFactor > 0) {
                auto& speed = ship.getWarpSpeedVector3();
                for (auto& pod : epriv::InternalScenePublicInterface::GetEntities(currentScene)) {
                    Entity e = currentScene.getEntity(pod);
                    const EntityDataRequest dataRequest(e);
                    auto* cam = e.getComponent<ComponentCamera>(dataRequest);
                    //TODO: parent->child relationship
                    if (e != ship.m_Entity && !cam) {
                        auto& otherBody = *e.getComponent<ComponentBody>(dataRequest);
                        otherBody.setPosition(otherBody.position() + (speed * static_cast<float>(dt)));
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
        }else if (Engine::isKeyDownOnce(KeyboardKey::F2)) { //if you store that positional value and revert to it when you switch camera perspectives you'll avoid the whole issue
            if (cameraState == CameraState::Follow || !ship.m_Target || target != ship.m_Entity) {
                currentScene.centerSceneToObject(ship.m_Entity);
                camera.orbit(ship.m_Entity);
                ship.restorePositionState();
            }else if (ship.m_Target) {
                ship.savePositionState();
                currentScene.centerSceneToObject(ship.m_Target->entity());
                camera.orbit(ship.m_Target->entity());
            }
        }else if (Engine::isKeyDownOnce(KeyboardKey::F3)) {
            if (cameraState == CameraState::FollowTarget || (!ship.m_Target && cameraState != CameraState::Follow) || target != ship.m_Entity) {
                currentScene.centerSceneToObject(ship.m_Entity);
                camera.follow(ship.m_Entity);
            }else if (ship.m_Target) {
                currentScene.centerSceneToObject(ship.m_Entity);
                camera.followTarget(ship.m_Target->entity(), ship.m_Entity);
            }
        }else if (Engine::isKeyDownOnce(KeyboardKey::F4)) {
			camera.m_State = CameraState::Freeform;
        }
        #pragma endregion

        if (Engine::isKeyDownOnce(KeyboardKey::T) && currentScene.name() != "Menu") {
            Entity scan = camera.getObjectInCenterRay(ship.m_Entity);
            if (!scan.null()) {
                auto* componentName = scan.getComponent<ComponentName>();
                if (componentName) {
                    EntityWrapper* scannedTarget = nullptr;
                    for (auto& obj : currentScene.m_Objects) {
                        auto* componentName1 = obj->getComponent<ComponentName>();
                        if (componentName1 && componentName1->name() == componentName->name()) {
                            scannedTarget = obj;
                            break;
                        }
                    }
                    ship.setTarget(scannedTarget);
                }
            }
        }
    }
    for (auto& shipSystem : ship.m_ShipSystems) 
        shipSystem.second->update(dt);
}};


Ship::Ship(Client& client, Handle& mesh, Handle& mat, const string& shipClass, bool player, const string& name, glm::vec3 pos, glm::vec3 scl, CollisionType::Type _type, Map* scene):EntityWrapper(*scene),m_Client(client){
    m_WarpFactor    = 0;
    m_IsPlayer      = player;
    m_ShipClass     = shipClass;
    m_IsWarping     = false;
    m_Target        = nullptr;
    m_PlayerCamera  = nullptr;
    m_MouseFactor   = glm::dvec2(0.0);
    m_SavedOldStateBefore = false;

    auto& rigidBodyComponent = *addComponent<ComponentBody>(_type);
    auto& modelComponent     = *addComponent<ComponentModel>(mesh, mat);
    auto& nameComponent      = *addComponent<ComponentName>(name);
    auto& logicComponent     = *addComponent<ComponentLogic>(ShipLogicFunctor(), this);

    setModel(mesh);

    const_cast<btRigidBody&>(rigidBodyComponent.getBody()).setDamping(0.01f, 0.2f);
    rigidBodyComponent.getBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
	rigidBodyComponent.setPosition(pos);
	rigidBodyComponent.setScale(scl);

	if (player) {
		m_PlayerCamera = static_cast<GameCamera*>(scene->getActiveCamera());
	}
	for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
		ShipSystem* system = nullptr;
		if      (i == 0)  system = new ShipSystemReactor(*this, 1000);
		else if (i == 1)  system = new ShipSystemPitchThrusters(*this);
		else if (i == 2)  system = new ShipSystemYawThrusters(*this);
		else if (i == 3)  system = new ShipSystemRollThrusters(*this);
        else if (i == 4)  system = new ShipSystemCloakingDevice(*this);
		else if (i == 5)  system = new ShipSystemShields(*this);
		else if (i == 6)  system = new ShipSystemMainThrusters(*this);
		else if (i == 7)  system = new ShipSystemWarpDrive(*this);
		else if (i == 8)  system = new ShipSystemSensors(*this);
        m_ShipSystems.emplace(i, system);
	}
    scene->m_Objects.push_back(this);
    scene->getShips().emplace(name, this);
}
Ship::~Ship(){
	SAFE_DELETE_MAP(m_ShipSystems);
}
const glm::vec3 Ship::getWarpSpeedVector3() {
    if (m_IsWarping && m_WarpFactor > 0) {
        auto& body = *getComponent<ComponentBody>();
        const float& speed = (m_WarpFactor * 1.0f / 0.46f) * 2.0f;
        return (body.forward() * glm::pow(speed, 15.0f)) / glm::log2(body.mass() + 0.5f);
    }
    return glm::vec3(0.0f);
}
void Ship::updatePhysicsFromPacket(const PacketPhysicsUpdate& packet, Map& map, vector<string>& info) {
    const unsigned int& size = stoi(info[2]);
    Anchor* closest = map.getRootAnchor();
    for (unsigned int i = 3; i < 3 + size; ++i) {
        auto& children = closest->getChildren();
        if (!children.count(info[i])) {
            return;
        }
        closest = children.at(info[i]);
    }
    const auto nearestAnchorPos = closest->getPosition();
    const float x = packet.px + nearestAnchorPos.x;
    const float y = packet.py + nearestAnchorPos.y;
    const float z = packet.pz + nearestAnchorPos.z;

    auto& body = *getComponent<ComponentBody>();
    btRigidBody& bulletBody = *const_cast<btRigidBody*>(&body.getBody());
    bulletBody.activate(true);//this is needed for when objects are far apart, should probably find a way to better do this
    btTransform centerOfMass;
    const btVector3 pos(x, y, z);

    float qx, qy, qz, qw, ax, ay, az, lx, ly, lz;

    Math::Float32From16(&qx, packet.qx);  Math::Float32From16(&qy, packet.qy);  Math::Float32From16(&qz, packet.qz);  Math::Float32From16(&qw, packet.qw);
    Math::Float32From16(&lx, packet.lx);  Math::Float32From16(&ly, packet.ly);  Math::Float32From16(&lz, packet.lz);
    Math::Float32From16(&ax, packet.ax);  Math::Float32From16(&ay, packet.ay);  Math::Float32From16(&az, packet.az);

    const btQuaternion rot(qx, qy, qz, qw);

    centerOfMass.setOrigin(pos);
    centerOfMass.setRotation(rot);
    bulletBody.getMotionState()->setWorldTransform(centerOfMass);
    bulletBody.setCenterOfMassTransform(centerOfMass);

    body.clearAllForces();
    body.setAngularVelocity(ax, ay, az, false);
    body.setLinearVelocity(lx - (packet.wx * 1.333333333f), ly - (packet.wy * 1.333333333f), lz - (packet.wz * 1.333333333f), false);
}
bool Ship::canSeeCloak() {
    if (m_IsPlayer) { //TODO: or is this ship an ally of the player
        return true;
    }
    return false;
}
void Ship::updateCloakFromPacket(const PacketCloakUpdate& packet) {
    if (!m_ShipSystems[ShipSystemType::CloakingDevice])
        return;
    ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
    cloak.m_CloakTimer = packet.cloakTimer;
    auto& model = *getComponent<ComponentModel>();
    auto& instance = model.getModel(0);
    if (!canSeeCloak()) {
        if (cloak.m_CloakTimer < 0.0) {
            cloak.m_CloakTimer = 0.0f; 
        }
    }
    cloak.m_Active = packet.cloakActive;

    if (packet.justTurnedOn || packet.justTurnedOff) {
        if (packet.justTurnedOn)
            Ship::cloak(false);
        if (packet.justTurnedOff)
            Ship::decloak(false);
    }else{
        if (cloak.m_CloakTimer < 1.0f && cloak.m_CloakTimer >= 0.0f) {
            model.setModelShaderProgram(ShaderProgram::Forward, 0, RenderStage::ForwardTransparentTrianglesSorted);
            instance.setColor(1, 1, 1, glm::abs(cloak.m_CloakTimer));
        }else if(cloak.m_CloakTimer < 0.0f){
            model.setModelShaderProgram(ShaderProgram::Forward, 0, RenderStage::ForwardTransparentTrianglesSorted);
            instance.setColor(0.369f, 0.912f, 1, glm::abs(cloak.m_CloakTimer));
        }else{
            model.setModelShaderProgram(ShaderProgram::Deferred, 0, RenderStage::GeometryOpaque);
            instance.setColor(1, 1, 1, glm::abs(cloak.m_CloakTimer));
        }
    }
}

void Ship::setModel(Handle& modelHandle) {
    auto& rigidBodyComponent = *getComponent<ComponentBody>();
    auto& modelComponent     = *getComponent<ComponentModel>();
    modelComponent.setModelMesh(modelHandle, 0);


    const glm::vec3& boundingBox = modelComponent.boundingBox();
    const float& volume = boundingBox.x * boundingBox.y * boundingBox.z;
    rigidBodyComponent.setMass((volume * 0.4f) + 1.0f);
}
void Ship::translateWarp(const double& amount, const double& dt){
    double amountToAdd = amount * (1.0 / 0.5);
    if((amount > 0.0 && m_WarpFactor + amount < 1.07) || (amount < 0.0 && m_WarpFactor > 0.0f)){
        m_WarpFactor += static_cast<float>(amountToAdd * dt);
    }
}
bool Ship::cloak(bool sendPacket) {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        auto& model = *getComponent<ComponentModel>();
        return cloak.cloak(model, sendPacket);
    }
    return false;
}
bool Ship::decloak(bool sendPacket) {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        auto& model = *getComponent<ComponentModel>();
        return cloak.decloak(model, sendPacket);
    }
    return false;
}
void Ship::setTarget(const string& target) {
    Map* s = static_cast<Map*>(Resources::getCurrentScene());
    for (auto& entity : s->m_Objects) {
        auto* componentName = entity->getComponent<ComponentName>();
        if (componentName) {
            if (componentName->name() == target) {
                Ship* ship = dynamic_cast<Ship*>(entity);
                if (ship && ship->isFullyCloaked()) {
                    return;
                }
                m_Target = entity;
                return;
            }
        }
    }
}
bool Ship::isCloaked() {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        return cloak.m_Active;
    }
    return false;
}
bool Ship::isFullyCloaked() {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        if (cloak.m_Active && cloak.m_CloakTimer <= 0.0) 
            return true;
        return false;
    }
    return false;
}
void Ship::setTarget(EntityWrapper* target){
    if (!target) {
        if (m_IsPlayer && m_PlayerCamera) {
            m_PlayerCamera->follow(entity());
        }
    }
    m_Target = target;
}
void Ship::onEvent(const Event& e){

}

void Ship::savePositionState() {
    if (!m_SavedOldStateBefore) {
        /*
        Map& currentScene = *static_cast<SolarSystem*>(Resources::getCurrentScene());
        const auto& anchorPos = currentScene.getAnchor();
        const auto& shipPos = entity().getComponent<ComponentBody>()->position();
        currentScene.setOldAnchorPos(anchorPos.x, anchorPos.y, anchorPos.z);
        currentScene.setOldClientPos(shipPos.x, shipPos.y, shipPos.z);
        m_SavedOldStateBefore = true;
        */
    }
}
void Ship::restorePositionState() {
    if (m_SavedOldStateBefore) {
        /*
        Map& currentScene = *static_cast<SolarSystem*>(Resources::getCurrentScene());
        const auto& anchorPos = currentScene.getOldAnchorPos();
        currentScene.setAnchor(anchorPos.x, anchorPos.y, anchorPos.z);
        entity().getComponent<ComponentBody>()->setPosition(currentScene.getOldClientPos());
        m_SavedOldStateBefore = false;
        */
    }
}