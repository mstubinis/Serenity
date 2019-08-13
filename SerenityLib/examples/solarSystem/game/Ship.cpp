#include "Ship.h"
#include "GameCamera.h"
#include "map/Map.h"
#include "Packet.h"
#include "Helper.h"
#include "map/Anchor.h"
#include "ResourceManifest.h"

#include <core/engine/mesh/Mesh.h>
#include <core/engine/Engine.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/materials/Material.h>

#include <core/engine/utils/Engine_Debugging.h>

#include <BulletCollision/CollisionShapes/btCollisionShape.h>

#include <ecs/ComponentName.h>

#include "ships/shipSystems/ShipSystemCloakingDevice.h"
#include "ships/shipSystems/ShipSystemMainThrusters.h"
#include "ships/shipSystems/ShipSystemPitchThrusters.h"
#include "ships/shipSystems/ShipSystemReactor.h"
#include "ships/shipSystems/ShipSystemRollThrusters.h"
#include "ships/shipSystems/ShipSystemSensors.h"
#include "ships/shipSystems/ShipSystemShields.h"
#include "ships/shipSystems/ShipSystemWarpDrive.h"
#include "ships/shipSystems/ShipSystemYawThrusters.h"
#include "ships/shipSystems/ShipSystemWeapons.h"

#include <glm/gtx/norm.hpp>

using namespace Engine;
using namespace std;

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
                        auto _otherBody = e.getComponent<ComponentBody>(dataRequest);
                        if (_otherBody) {
                            auto& otherBody = *_otherBody;
                            otherBody.setPosition(otherBody.position() + (speed * static_cast<float>(dt)));
                        }
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
            if (cameraState == CameraState::Follow || !ship.m_Target || target != ship.m_Entity) {
                currentScene.centerSceneToObject(ship.m_Entity);
                camera.orbit(ship.m_Entity);
            }else if (ship.m_Target) {
                auto dist = glm::distance2(ship.getPosition(), ship.m_Target->getComponent<ComponentBody>()->position());
                if (dist < 10000000000.0f) { //to prevent FP issues when viewing things billions of km away
                    currentScene.centerSceneToObject(ship.m_Target->entity());
                    camera.orbit(ship.m_Target->entity());
                }
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
        if(shipSystem.second) //some ships wont have all the systems (cloaking device, etc)
            shipSystem.second->update(dt);
}};


Ship::Ship(Client& client, Handle& mesh, Handle& mat, const string& shipClass, bool player, const string& name, glm::vec3 pos, glm::vec3 scl, CollisionType::Type _type, Map* map):EntityWrapper(*map),m_Client(client){
    m_WarpFactor    = 0;
    m_IsPlayer      = player;
    m_ShipClass     = shipClass;
    m_IsWarping     = false;
    m_Target        = nullptr;
    m_PlayerCamera  = nullptr;
    m_MouseFactor   = glm::dvec2(0.0);

    auto& modelComponent     = *addComponent<ComponentModel>(mesh, mat);
    auto& rigidBodyComponent = *addComponent<ComponentBody>(_type);
    auto& nameComponent      = *addComponent<ComponentName>(name);
    auto& logicComponent     = *addComponent<ComponentLogic>(ShipLogicFunctor(), this);

    setModel(mesh);

    const_cast<btRigidBody&>(rigidBodyComponent.getBody()).setDamping(0.01f, 0.2f);
    rigidBodyComponent.getBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
	rigidBodyComponent.setPosition(pos);
	rigidBodyComponent.setScale(scl);

	if (player) {
		m_PlayerCamera = static_cast<GameCamera*>(map->getActiveCamera());
	}
    rigidBodyComponent.setUserPointer1(this);

    map->m_Objects.push_back(this);
    map->getShips().emplace(name, this);

    //derived classes need to add their own ship systems
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
const string Ship::getName() {
    return getComponent<ComponentName>()->name();
}
const glm::vec3 Ship::getPosition() {
    return getComponent<ComponentBody>()->position();
}
const glm::quat Ship::getRotation() {
    return getComponent<ComponentBody>()->rotation();
}
const glm::vec3 Ship::getPosition(const EntityDataRequest& dataRequest) {
    return getComponent<ComponentBody>(dataRequest)->position();
}
const glm::quat Ship::getRotation(const EntityDataRequest& dataRequest) {
    return getComponent<ComponentBody>(dataRequest)->rotation();
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
        return (cloak.m_Active || cloak.getCloakTimer() < 1.0f) ? true : false;
    }
    return false;
}
bool Ship::isFullyCloaked() {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        return (cloak.m_Active && cloak.m_CloakTimer <= 0.0) ? true : false;
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