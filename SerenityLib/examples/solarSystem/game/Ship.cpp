#include "Ship.h"
#include "GameCamera.h"
#include "map/Map.h"
#include "Packet.h"
#include "Helper.h"
#include "map/Anchor.h"
#include "ResourceManifest.h"
#include "ships/Ships.h"

#include <core/engine/mesh/Mesh.h>
#include <core/engine/Engine.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/materials/Material.h>
#include <core/engine/renderer/Decal.h>

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
#include "ships/shipSystems/ShipSystemHull.h"

#include "hud/SensorStatusDisplay.h"
#include "hud/HUD.h"

#include <glm/gtx/norm.hpp>


using namespace Engine;
using namespace std;

struct ShipLogicFunctor final {void operator()(ComponentLogic& _component, const double& dt) const {
    Ship& ship = *static_cast<Ship*>(_component.getUserPointer());
    Map& map = static_cast<Map&>(ship.entity().scene());

    if (ship.m_IsPlayer) {
        #pragma region PlayerFlightControls
        auto* mytarget = ship.getTarget();
        if (!Engine::paused()) {
            if (ship.m_IsWarping && ship.m_WarpFactor > 0) {
                auto& speed = ship.getWarpSpeedVector3();
                for (auto& pod : epriv::InternalScenePublicInterface::GetEntities(map)) {
                    Entity e = map.getEntity(pod);
                    const EntityDataRequest dataRequest(e);
                    auto* cam = e.getComponent<ComponentCamera>(dataRequest);
                    //TODO: parent->child relationship
                    if (e != ship.m_Entity && !cam && (e != map.getHUD().getSensorDisplay().radarRingEntity() && e != map.getHUD().getSensorDisplay().radarCameraEntity())) {
                        auto _otherBody = e.getComponent<ComponentBody>(dataRequest);
                        if (_otherBody) {
                            auto& otherBody = *_otherBody;
                            otherBody.setPosition(otherBody.position() + (speed * static_cast<decimal>(dt)));
                        }
                    }
                }
            }
        }
        #pragma endregion

        #pragma region PlayerCameraControls
        auto& camera = *ship.m_PlayerCamera;
        const auto& cameraState = camera.getState();
        const auto* target = camera.getTarget();
        if (Engine::isKeyDownOnce(KeyboardKey::F1)) {
            if (cameraState != CameraState::Follow || (cameraState == CameraState::Follow && target != &ship)) {
                map.centerSceneToObject(ship.m_Entity);
                camera.follow(&ship);
            }
        }else if (Engine::isKeyDownOnce(KeyboardKey::F2)) {
            if (cameraState == CameraState::Follow || !mytarget || target != &ship) {
                map.centerSceneToObject(ship.m_Entity);
                camera.orbit(&ship);
            }else if (mytarget) {
                const auto dist2 = glm::distance2(ship.getPosition(), mytarget->getComponent<ComponentBody>()->position());
                if (dist2 < static_cast<decimal>(10000000000.0)) { //to prevent FP issues when viewing things billions of km away
                    map.centerSceneToObject(mytarget->entity());
                    camera.orbit(mytarget);
                }
            }
        }else if (Engine::isKeyDownOnce(KeyboardKey::F3)) {
            if (cameraState == CameraState::FollowTarget || (!mytarget && cameraState != CameraState::Follow) || target != &ship) {
                map.centerSceneToObject(ship.m_Entity);
                camera.follow(&ship);
            }else if (mytarget) {
                map.centerSceneToObject(ship.m_Entity);
                camera.followTarget(mytarget, &ship);
            }
        }else if (Engine::isKeyDownOnce(KeyboardKey::F4)) {
			camera.m_State = CameraState::Freeform;
        }
        #pragma endregion

        if (Engine::isKeyDownOnce(KeyboardKey::T) && map.name() != "Menu") {
            Entity scan = camera.getObjectInCenterRay(ship.m_Entity);
            if (!scan.null()) {
                auto* componentName = scan.getComponent<ComponentName>();
                if (componentName) {
                    EntityWrapper* scannedTarget = nullptr;
                    for (auto& obj : map.m_Objects) {
                        auto* componentName1 = obj->getComponent<ComponentName>();
                        if (componentName1 && componentName1->name() == componentName->name()) {
                            scannedTarget = obj;
                            break;
                        }
                    }
                    ship.setTarget(scannedTarget, true);
                }
            }
        }
    }
    for (auto& shipSystem : ship.m_ShipSystems) {
        if (shipSystem.second) { //some ships wont have all the systems (cloaking device, etc)
            shipSystem.second->update(dt);
        }
    }
    if (ship.IsPlayer()) {
        for (auto& shipSystem : ship.m_ShipSystems) {
            if (shipSystem.second) {
                shipSystem.second->render();
            }
        }
    }
    for (auto& decal : ship.m_DamageDecals) {
        auto& _decal = *decal;
        auto& shipBody = *ship.getComponent<ComponentBody>();
        const auto shipRotation = shipBody.rotation();
        _decal.setPosition(shipBody.position() + (shipRotation * _decal.initialPosition()));
        _decal.setRotation(shipRotation * _decal.initialRotation());
        _decal.update(dt);
        if (!_decal.active()) {
            removeFromVector(ship.m_DamageDecals, decal); //might be dangerous
        }
    }
}};

//TODO: move to the hull system?
struct HullCollisionFunctor final { void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
    auto ownerShipVoid = owner.getUserPointer1();
    if (ownerShipVoid) {
        auto otherShipVoid = other.getUserPointer1();
        if (otherShipVoid && ownerShipVoid != otherShipVoid) { //redundant?
            if (owner.getCollisionGroup() == CollisionFilter::_Custom_4 && other.getCollisionGroup() == CollisionFilter::_Custom_4) { //hull on hull only
                auto* ownerShip = static_cast<Ship*>(ownerShipVoid);
                auto* otherShip = static_cast<Ship*>(otherShipVoid);
                auto* ownerHull = static_cast<ShipSystemHull*>(ownerShip->getShipSystem(ShipSystemType::Hull));
                auto* otherHull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                if (ownerHull && otherHull) {
                    const auto ownerMass = owner.mass() * 3000.0f;
                    const auto otherMass = other.mass() * 3000.0f;
                    const auto massTotal = ownerMass + otherMass;

                    const auto ownerLocal = ownerHit - glm::vec3(owner.position());
                    const auto otherLocal = otherHit - glm::vec3(other.position());

                    const auto ownerMomentum = ownerMass * glm::vec3(ownerShip->getLinearVelocity());
                    const auto otherMomentum = otherMass * glm::vec3(otherShip->getLinearVelocity());
                    const auto totalMomentum = ownerMomentum + otherMomentum;

                    const auto damageTotal1 = (ownerMass / massTotal) * totalMomentum;
                    const auto damageTotal2 = (otherMass / massTotal) * totalMomentum;

                    const auto damageRadiusOwner = 4.0f;
                    const auto damageRadiusOther = 4.0f;

                    ownerHull->receiveCollision( normal, ownerLocal, damageRadiusOwner, glm::length(damageTotal2) );
                    otherHull->receiveCollision( normal, otherLocal, damageRadiusOther, glm::length(damageTotal1) );
                }
            }
        }
    }
}};

Ship::Ship(Client& client, const string& shipClass, Map& map, bool player, const string& name, const glm_vec3 pos, const glm_vec3 scl, CollisionType::Type collisionType, const glm::vec3 aimPosDefault, const glm::vec3 camOffsetDefault):EntityWrapper(map),m_Client(client){
    m_WarpFactor          = 0;
    m_IsPlayer            = player;
    m_ShipClass           = shipClass;
    m_IsWarping           = false;
    m_PlayerCamera        = nullptr;
    m_MouseFactor         = glm::dvec2(0.0);
    m_AimPositionDefaults.push_back(aimPosDefault);
    m_CameraOffsetDefault = camOffsetDefault;

    auto& shipInfo = Ships::Database[shipClass];

    auto& modelComponent     = *addComponent<ComponentModel>(shipInfo.MeshHandles[0], shipInfo.MaterialHandles[0]);
    auto& body               = *addComponent<ComponentBody>(collisionType);
    auto& nameComponent      = *addComponent<ComponentName>(name);
    auto& logicComponent     = *addComponent<ComponentLogic>(ShipLogicFunctor(), this);

    setModel(shipInfo.MeshHandles[0]);

    const_cast<btRigidBody&>(body.getBtBody()).setDamping(static_cast<btScalar>(0.01), static_cast<btScalar>(0.2));
    body.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    body.setPosition(pos);
    body.setScale(scl);

    //the body is using a convex hull for ship to ship ramming
    body.setCollisionGroup(CollisionFilter::_Custom_4); //i belong to ramming hull group (group 4)
    body.setCollisionMask(CollisionFilter::_Custom_4); //i should only collide with other ramming hulls only
    body.setCollisionFunctor(HullCollisionFunctor());

	if (player) {
		m_PlayerCamera = static_cast<GameCamera*>(map.getActiveCamera());
	}
    body.setUserPointer1(this);
    
    map.m_Objects.push_back(this);
    map.getShips().emplace(name, this);

    registerEvent(EventType::WindowResized);

    //derived classes need to add their own ship systems
}
Ship::~Ship(){
    unregisterEvent(EventType::WindowResized);
    SAFE_DELETE_VECTOR(m_DamageDecals);
	SAFE_DELETE_MAP(m_ShipSystems);
}
void Ship::addHullTargetPoints(vector<glm::vec3>& points) {
    for (auto& pt : points) {
        m_AimPositionDefaults.push_back(pt);
    }
}
const glm::vec3 Ship::getAimPositionDefault() {
    if (m_AimPositionDefaults.size() == 0) {
        return getPosition();
    }
    auto& body = *getComponent<ComponentBody>();
    return body.position() + Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[0]);
}
const glm::vec3 Ship::getAimPositionRandom() {
    if (m_AimPositionDefaults.size() == 0) {
        return getPosition();
    }
    auto& body = *getComponent<ComponentBody>();
    if (m_AimPositionDefaults.size() == 1) {
        return body.position() + Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[0]);
    }
    const auto randIndex = Helper::GetRandomIntFromTo(0, m_AimPositionDefaults.size() - 1);
    return body.position() + Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[randIndex]);
}
const glm::vec3 Ship::getAimPositionDefaultLocal() {
    if (m_AimPositionDefaults.size() == 0) {
        return glm::vec3(0.0f);
    }
    auto& body = *getComponent<ComponentBody>();
    if (m_AimPositionDefaults.size() == 1) {
        return Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[0]);
    }
    return Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[0]);
}

const uint Ship::getAimPositionRandomLocalIndex() {
    if (m_AimPositionDefaults.size() == 1) {
        return 0;
    }
    const auto randIndex = Helper::GetRandomIntFromTo(0, m_AimPositionDefaults.size() - 1);
    return randIndex;
}
const glm::vec3 Ship::getAimPositionRandomLocal() {
    return getAimPositionLocal(getAimPositionRandomLocalIndex());
}
const glm::vec3 Ship::getAimPositionLocal(const uint index) {
    if (m_AimPositionDefaults.size() == 0) {
        return glm::vec3(0.0f);
    }
    auto& body = *getComponent<ComponentBody>();
    if (m_AimPositionDefaults.size() == 1) {
        return Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[0]);
    }
    return Math::rotate_vec3(body.rotation(), m_AimPositionDefaults[index]);
}

void Ship::destroy() {
    for (auto& system : m_ShipSystems) {
        if (system.second) {
            system.second->destroy();
        }
    }
    EntityWrapper::destroy();
    SAFE_DELETE_VECTOR(m_DamageDecals);
}
const glm_vec3 Ship::getWarpSpeedVector3() {
    if (m_IsWarping && m_WarpFactor > 0) {
        auto& body = *getComponent<ComponentBody>();
        const auto speed = (m_WarpFactor * 1.0f / 0.46f) * 2.0f;
        return (body.forward() * glm::pow(static_cast<decimal>(speed), static_cast<decimal>(15.0))) / glm::log2(static_cast<decimal>(body.mass()) + static_cast<decimal>(0.5));
    }
    return glm_vec3(static_cast<decimal>(0.0));
}
const string Ship::getName() {
    return getComponent<ComponentName>()->name();
}
const glm_vec3 Ship::getPosition() {
    return getComponent<ComponentBody>()->position();
}
const glm_quat Ship::getRotation() {
    return getComponent<ComponentBody>()->rotation();
}
const glm_vec3 Ship::getPosition(const EntityDataRequest& dataRequest) {
    return getComponent<ComponentBody>(dataRequest)->position();
}
const glm_quat Ship::getRotation(const EntityDataRequest& dataRequest) {
    return getComponent<ComponentBody>(dataRequest)->rotation();
}
void Ship::updateProjectileImpact(const PacketProjectileImpact& packet) {
    glm::vec3 normal;
    float rad, time;
    Engine::Math::Float32From16(&rad, packet.radius);
    Engine::Math::Float32From16(&time, packet.time);
    Engine::Math::Float32From16(&normal.x, packet.normalX);
    Engine::Math::Float32From16(&normal.y, packet.normalY);
    Engine::Math::Float32From16(&normal.z, packet.normalZ);
    Map& map = static_cast<Map&>(entity().scene());
    if (packet.shields) {
        auto* shields = static_cast<ShipSystemShields*>(getShipSystem(ShipSystemType::Shields));
        if (shields) {
            const glm::vec3 local = glm::vec3(packet.impactX, packet.impactY, packet.impactZ);
            const uint shieldSide = static_cast<uint>(shields->getImpactSide(local));
            shields->receiveHit(normal, local, rad, time, packet.damage, shieldSide);
        }
        if (packet.PacketType == PacketType::Server_To_Client_Projectile_Cannon_Impact) {
            auto* proj = map.getCannonProjectile(packet.index);
            if (proj) {
                proj->destroy();
            }
        }else if (packet.PacketType == PacketType::Server_To_Client_Projectile_Torpedo_Impact) {
            auto* proj = map.getTorpedoProjectile(packet.index);
            if (proj) {
                proj->destroy();
            }
        }
        return;
    }else{
        auto* hull = static_cast<ShipSystemHull*>(getShipSystem(ShipSystemType::Hull));
        if (hull) {
            hull->receiveHit(normal, glm::vec3(packet.impactX, packet.impactY, packet.impactZ), rad, time, packet.damage);
        }
        if (packet.PacketType == PacketType::Server_To_Client_Projectile_Cannon_Impact) {
            auto* proj = map.getCannonProjectile(packet.index);
            if (proj) {
                proj->destroy();
            }
        }else if (packet.PacketType == PacketType::Server_To_Client_Projectile_Torpedo_Impact) {
            auto* proj = map.getTorpedoProjectile(packet.index);
            if (proj) {
                proj->destroy();
            }
        }
        return;
    }
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
    const auto x = packet.px + nearestAnchorPos.x;
    const auto y = packet.py + nearestAnchorPos.y;
    const auto z = packet.pz + nearestAnchorPos.z;

    auto& body = *getComponent<ComponentBody>();
    btRigidBody& bulletBody = *const_cast<btRigidBody*>(&body.getBtBody());
    bulletBody.activate(true);//this is needed for when objects are far apart, should probably find a way to better do this
    btTransform centerOfMass;

    float qw, ax, ay, az, lx, ly, lz;

    auto quat_xyz = Math::unpack3NormalsFrom32Int(packet.qXYZ);

    Math::Float32From16(&qw, packet.qw);
    Math::Float32From16(&lx, packet.lx);  Math::Float32From16(&ly, packet.ly);  Math::Float32From16(&lz, packet.lz);
    Math::Float32From16(&ax, packet.ax);  Math::Float32From16(&ay, packet.ay);  Math::Float32From16(&az, packet.az);

    const btQuaternion rot(quat_xyz.x, quat_xyz.y, quat_xyz.z, qw);

    const btVector3 pos(x, y, z);
    centerOfMass.setOrigin(pos);
    centerOfMass.setRotation(rot);
    bulletBody.getMotionState()->setWorldTransform(centerOfMass);
    bulletBody.setCenterOfMassTransform(centerOfMass);

    body.clearAllForces();
    body.setAngularVelocity(ax, ay, az, false);
    body.setLinearVelocity(static_cast<decimal>(lx - (packet.wx * WARP_PHYSICS_MODIFIER)), static_cast<decimal>(ly - (packet.wy * WARP_PHYSICS_MODIFIER)), static_cast<decimal>(lz - (packet.wz * WARP_PHYSICS_MODIFIER)), false);
    //body.setGoal(x, y, z, PHYSICS_PACKET_TIMER_LIMIT);
}
bool Ship::canSeeCloak() {
    if (m_IsPlayer) { //TODO: or is this ship an ally of the player
        return true;
    }
    return false;
}
void Ship::updateDamageDecalsCloak(const float& alpha) {
    for (auto& decal : m_DamageDecals) {
        if (decal && decal->active()) {
            auto& model = *decal->getComponent<ComponentModel>();
            auto& modelOne = model.getModel(0);
            auto& color = modelOne.color();
            modelOne.setColor(color.r, color.g, color.b, alpha);
        }
    }
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
            updateDamageDecalsCloak(glm::abs(cloak.m_CloakTimer));
        }else if(cloak.m_CloakTimer < 0.0f){
            model.setModelShaderProgram(ShaderProgram::Forward, 0, RenderStage::ForwardTransparentTrianglesSorted);
            instance.setColor(0.369f, 0.912f, 1, glm::abs(cloak.m_CloakTimer));
            updateDamageDecalsCloak(glm::abs(cloak.m_CloakTimer));
        }else{
            model.setModelShaderProgram(ShaderProgram::Deferred, 0, RenderStage::GeometryOpaque);
            instance.setColor(1, 1, 1, glm::abs(cloak.m_CloakTimer));
            updateDamageDecalsCloak(glm::abs(cloak.m_CloakTimer));
        }
    }
}
void Ship::updateHealthFromPacket(const PacketHealthUpdate& packet) {
    auto* shields = static_cast<ShipSystemShields*>(m_ShipSystems[ShipSystemType::Shields]);
    auto* hull    = static_cast<ShipSystemHull*>(m_ShipSystems[ShipSystemType::Hull]);
    if (shields) {

        shields->m_HealthPointsCurrent[0] = packet.currentShieldsHealthF;
        shields->m_HealthPointsCurrent[1] = packet.currentShieldsHealthA;
        shields->m_HealthPointsCurrent[2] = packet.currentShieldsHealthP;
        shields->m_HealthPointsCurrent[3] = packet.currentShieldsHealthS;
        shields->m_HealthPointsCurrent[4] = packet.currentShieldsHealthD;
        shields->m_HealthPointsCurrent[5] = packet.currentShieldsHealthV;


        if (packet.flags & PacketHealthUpdate::PacketHealthFlags::ShieldsActive) {
        }
        if (packet.flags & PacketHealthUpdate::PacketHealthFlags::ShieldsInstalled) {
        }
        if (packet.flags & PacketHealthUpdate::PacketHealthFlags::ShieldsTurnedOn) {
            shields->m_ShieldsAreUp = packet.flags;
        }
    }
    if (hull) {
        hull->m_HealthPointsCurrent = packet.currentHullHealth;
    }
}
const float Ship::updateShipDimensions() {
    auto& rigidBodyComponent = *getComponent<ComponentBody>();
    auto& modelComponent = *getComponent<ComponentModel>();

    const auto& boundingBox = modelComponent.boundingBox();
    const auto volume = boundingBox.x * boundingBox.y * boundingBox.z;
    auto mass = (volume * 0.4f) + 1.0f;
    rigidBodyComponent.setMass(mass);
    return mass;
}
void Ship::setModel(Handle& modelHandle) {
    auto& rigidBodyComponent = *getComponent<ComponentBody>();
    auto& modelComponent     = *getComponent<ComponentModel>();
    modelComponent.setModelMesh(modelHandle, 0);
    const auto& boundingBox = modelComponent.boundingBox();
    const auto volume = boundingBox.x * boundingBox.y * boundingBox.z;
    rigidBodyComponent.setMass((volume * 0.4f) + 1.0f);
}
void Ship::translateWarp(const double& amount, const double& dt){
    const auto amountToAdd = amount * (1.0 / 0.5);
    if((amount > 0.0 && m_WarpFactor + amount < 1.07) || (amount < 0.0 && m_WarpFactor > 0.0f)){
        m_WarpFactor += static_cast<float>(amountToAdd * dt);
    }
}
bool Ship::cloak(const bool sendPacket) {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        auto& model = *getComponent<ComponentModel>();
        return cloak.cloak(model, sendPacket);
    }
    return false;
}
bool Ship::decloak(const bool sendPacket) {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        auto& model = *getComponent<ComponentModel>();
        return cloak.decloak(model, sendPacket);
    }
    return false;
}
void Ship::toggleWarp() {
    m_IsWarping = !m_IsWarping;
    m_WarpFactor = 0;
    auto& rigidBodyComponent = *getComponent<ComponentBody>();
    rigidBodyComponent.clearLinearForces();
}
const string& Ship::getClass() const { 
    return m_ShipClass; 
}
GameCamera* Ship::getPlayerCamera() { 
    return m_PlayerCamera; 
}
const bool Ship::IsPlayer() const {
    return m_IsPlayer; 
}
const bool Ship::IsWarping() const {
    return m_IsWarping; 
}
Entity& Ship::entity() { 
    return m_Entity; 
}
ShipSystem* Ship::getShipSystem(const uint type) { 
    return m_ShipSystems[type]; 
}
const glm_vec3 Ship::getLinearVelocity() {
    if (m_IsWarping && m_IsPlayer) {
        return -(getWarpSpeedVector3() * static_cast<decimal>(WARP_PHYSICS_MODIFIER));
    }
    return getComponent<ComponentBody>()->getLinearVelocity();
}
EntityWrapper* Ship::getTarget() { 
    auto* sensors = static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]);
    if(sensors)
        return sensors->m_Target;
    return nullptr;
}
void Ship::setTarget(const string& target, const bool sendPacket) {
    auto* sensors = static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]);
    if (sensors)
        sensors->setTarget(target, sendPacket);
}
void Ship::setTarget(EntityWrapper* target, const bool sendPacket) {
    auto* sensors = static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]);
    if (sensors)
        sensors->setTarget(target, sendPacket);
}
const glm_vec3& Ship::forward() {
    return getComponent<ComponentBody>()->forward();
}
const glm_vec3& Ship::right() {
    return getComponent<ComponentBody>()->right();
}
const glm_vec3& Ship::up() {
    return getComponent<ComponentBody>()->up();
}
const bool Ship::isCloaked() {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        return (cloak.m_Active || cloak.getCloakTimer() < 1.0f) ? true : false;
    }
    return false;
}
const bool Ship::isFullyCloaked() {
    if (m_ShipSystems[ShipSystemType::CloakingDevice]) {
        ShipSystemCloakingDevice& cloak = *static_cast<ShipSystemCloakingDevice*>(m_ShipSystems[ShipSystemType::CloakingDevice]);
        return (cloak.m_Active && cloak.m_CloakTimer <= 0.0) ? true : false;
    }
    return false;
}
void Ship::onEvent(const Event& e){
    if (e.type == EventType::WindowResized) {
        for (auto& system : m_ShipSystems) {
            if (system.second) {
                system.second->onResize(e.eventWindowResized.width, e.eventWindowResized.height);
            }
        }
    }
}

PrimaryWeaponBeam& Ship::getPrimaryWeaponBeam(const uint index) {
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));
    return *weapons.m_PrimaryWeaponsBeams[index];
}
PrimaryWeaponCannon& Ship::getPrimaryWeaponCannon(const uint index) {
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));
    return *weapons.m_PrimaryWeaponsCannons[index];
}
SecondaryWeaponTorpedo& Ship::getSecondaryWeaponTorpedo(const uint index) {
    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));
    return *weapons.m_SecondaryWeaponsTorpedos[index];
}
void Ship::update(const double& dt) {

}