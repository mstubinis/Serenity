#include "ShipSystemWeapons.h"
#include "ShipSystemSensors.h"
#include "../../weapons/Weapons.h"
#include "../../Ship.h"
#include "../../ResourceManifest.h"
#include "../../networking/packets/Packets.h"
#include "../../map/Map.h"
#include <core/engine/lights/Lights.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/physics/Engine_Physics.h>
#include <core/engine/physics/Collision.h>

#include <core/engine/events/Engine_Events.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <core/engine/system/Engine.h>
#include <glm/gtx/norm.hpp>

#include "../../ships/shipSystems/ShipSystemShields.h"
#include "../../ships/shipSystems/ShipSystemHull.h"

using namespace Engine;
using namespace std;

#define PREDICTED_PHYSICS_CONSTANT 0.11111111f

const decimal ShipSystemWeapons::calculate_quadratic_time_till_hit(const glm_vec3& pos, const glm_vec3& vel, const decimal& s) {
    auto a = s * s - glm::dot(vel, vel);
    auto b = glm::dot(pos, vel);
    auto c = glm::dot(pos, pos);
    auto d = b * b + a * c;
    auto t = 0.0;
    if (d >= 0.0) {
        t = (b + sqrt(d)) / a;
        if (t < 0.0)
            t = 0.0;
    }
    return t;
}

ShipWeapon::ShipWeapon(Map& map, WeaponType::Type _type, Ship& _ship, const glm_vec3& _position, const glm_vec3& _forward, const float& _arc, const float& _dmg, const float& _impactRad, const float& _impactTime, const float& _volume, const uint& _numRounds, const float& _rechargeTimerPerRound, const unsigned int& _modelIndex):ship(_ship),m_Map(map){
    type                     = _type;
    position                 = _position;
    forward                  = glm::normalize(_forward);
    arc                      = _arc;
    damage                   = _dmg;
    impactRadius             = _impactRad;
    impactTime               = _impactTime;
    volume                   = _volume;
    numRounds = numRoundsMax = _numRounds;
    rechargeTimePerRound     = _rechargeTimerPerRound;
    rechargeTimer            = 0.0f;
    soundEffect              = nullptr;
    index                    = 0;
    modelIndex               = _modelIndex;
}
const bool ShipWeapon::isInArc(const glm_vec3& world_position, const float _arc) {
    auto& shipBody     = *ship.getComponent<ComponentBody>();
    auto shipPosition  = shipBody.position();
    auto shipRotation  = shipBody.rotation();

    glm_vec3 launcher_pos = shipPosition + Math::rotate_vec3(shipRotation, position);

    const auto cannonForward = glm::normalize(Math::rotate_vec3(shipRotation, forward));
    const auto vecToTarget   = launcher_pos - world_position;
    const auto vecToForward  = launcher_pos - (cannonForward * static_cast<decimal>(100000000.0));
    const auto angleToTarget = Math::getAngleBetweenTwoVectors(glm::normalize(vecToTarget), glm::normalize(vecToForward), true);
    if (angleToTarget <= _arc) {
        return true;
    }
    return false;
}
const bool ShipWeapon::isInArc(EntityWrapper* target, const float _arc) {
    if (!target)
        return false;
    auto& shipBody     = *ship.getComponent<ComponentBody>();
    auto shipPosition  = shipBody.position();
    auto shipRotation  = shipBody.rotation();

    auto& targetBody   = *target->getComponent<ComponentBody>();
    auto* targetIsShip = dynamic_cast<Ship*>(target);
    glm_vec3 targetPosition;
    if (targetIsShip) {
        targetPosition = targetIsShip->getAimPositionDefault();
    }else{
        targetPosition = targetBody.position();
    }
    glm_vec3 launcher_pos = shipPosition + Math::rotate_vec3(shipRotation, position);

    const auto cannonForward = glm::normalize(Math::rotate_vec3(shipRotation, forward));
    const auto vecToTarget   = launcher_pos - targetPosition;
    const auto vecToForward  = launcher_pos - (cannonForward * static_cast<decimal>(100000000.0));
    const auto angleToTarget = Math::getAngleBetweenTwoVectors(glm::normalize(vecToTarget), glm::normalize(vecToForward), true);
    if (angleToTarget <= _arc) {
        return true;
    }
    return false;
}
const glm_vec3 ShipWeapon::getWorldPosition() {
    return ship.getPosition() + (Math::rotate_vec3(ship.getRotation(), position));
}
const decimal ShipWeapon::getDistanceSquared(const glm_vec3& pos) {
    auto res = glm::distance2(pos, getWorldPosition());
    return res;
}
const decimal ShipWeapon::getDistanceSquared(DetectedShip& ship) {
    auto res = glm::distance2(ship.ship->getPosition(), getWorldPosition());
    return res;
}
const decimal ShipWeapon::getDistanceSquared(EntityWrapper* target) {
    auto res = glm::distance2(target->getComponent<ComponentBody>()->position(), getWorldPosition());
    return res;
}

const int ShipWeapon::acquire_index() {
     return -1;
}

WeaponProjectile::WeaponProjectile(Map& map_, const int index) : map(map_) {
    entity           = map_.createEntity();
    currentTime      = 0.0f;
    maxTime          = 2.5f;
    active           = false;
    destroyed        = false;
    projectile_index = index;
    light            = nullptr;
}
WeaponProjectile::~WeaponProjectile() {

}
void WeaponProjectile::update(const double& dt) {
    if (active) {
        const float fdt = static_cast<float>(dt);
        currentTime += fdt;
        if (light) {
            auto& lightBody = *light->getComponent<ComponentBody>();
            lightBody.setPosition(entity.getComponent<ComponentBody>()->position());
        }
        if (currentTime >= maxTime) {
            destroy();
        }
    }
}
void WeaponProjectile::destroy() {
    if (!destroyed) {
        entity.destroy();
        if (light) {
            light->destroy();
            SAFE_DELETE(light);
        }
        active = false;
        destroyed = true;
    }
}
void WeaponProjectile::clientToServerImpactShields(Ship& sourceShip, const bool cannon, Client& client, Ship& shipHit, const glm::vec3& impactModelSpacePosition, const glm::vec3& impactNormal, const float& impactRadius, const float& damage, const float& time, const unsigned int& shield_face) {
    if (!active)
        return;
    PacketProjectileImpact packet;
    if(cannon)
        packet.PacketType = PacketType::Client_To_Server_Projectile_Cannon_Impact;
    else
        packet.PacketType = PacketType::Client_To_Server_Projectile_Torpedo_Impact;
    packet.damage = damage;
    packet.model_index = 0;
    packet.impactX = impactModelSpacePosition.x;
    packet.impactY = impactModelSpacePosition.y;
    packet.impactZ = impactModelSpacePosition.z;
    packet.shields = true;
    packet.shield_side = shield_face;
    packet.source_ship_map_key = sourceShip.getMapKey();
    packet.impacted_ship_map_key = shipHit.getMapKey();
    packet.projectile_index = projectile_index;
    Math::Float16From32(&packet.time, time);
    Math::Float16From32(&packet.radius, impactRadius);
    Math::Float16From32(&packet.normalX, impactNormal.x);
    Math::Float16From32(&packet.normalY, impactNormal.y);
    Math::Float16From32(&packet.normalZ, impactNormal.z);
    active = false;
    client.send(packet);
}
void WeaponProjectile::clientToServerImpactHull(Ship& sourceShip, const bool cannon, Client& client, Ship& shipHit, const glm::vec3& impactModelSpacePosition, const glm::vec3& impactNormal, const float& impactRadius, const float& damage, const float& time, const size_t modelIndex) {
    if (!active)
        return;
    PacketProjectileImpact packet;
    if (cannon)
        packet.PacketType = PacketType::Client_To_Server_Projectile_Cannon_Impact;
    else
        packet.PacketType = PacketType::Client_To_Server_Projectile_Torpedo_Impact;
    packet.damage = damage;
    packet.model_index = static_cast<int>(modelIndex);
    packet.impactX = impactModelSpacePosition.x;
    packet.impactY = impactModelSpacePosition.y;
    packet.impactZ = impactModelSpacePosition.z;
    packet.shields = false;
    packet.shield_side = -1;
    packet.source_ship_map_key = sourceShip.getMapKey();
    packet.impacted_ship_map_key = shipHit.getMapKey();
    packet.projectile_index = projectile_index;
    Math::Float16From32(&packet.time, time);
    Math::Float16From32(&packet.radius, impactRadius);
    Math::Float16From32(&packet.normalX, impactNormal.x);
    Math::Float16From32(&packet.normalY, impactNormal.y);
    Math::Float16From32(&packet.normalZ, impactNormal.z);
    active = false;
    client.send(packet);
}

PrimaryWeaponCannonProjectile::PrimaryWeaponCannonProjectile(Map& _map, const glm_vec3& position, const glm_vec3& forward, const int index) : WeaponProjectile(_map, index) {
}
PrimaryWeaponCannonProjectile::~PrimaryWeaponCannonProjectile() {

}
void PrimaryWeaponCannonProjectile::destroy() {
    WeaponProjectile::destroy();
    map.removeCannonProjectile(projectile_index);
}

PrimaryWeaponCannon::PrimaryWeaponCannon(Map& map, WeaponType::Type _type, Ship& _ship,const glm_vec3& _pos,const glm_vec3& _fwd,const float& _arc,const uint& _maxCharges,const float& _dmg,const float& _rechargePerRound,const float& _impactRad,const float& _impactTime, const float& _travelSpeed, const float& _volume, const unsigned int& _modelIndex) : ShipWeapon(map, _type, _ship, _pos, _fwd, _arc, _dmg, _impactRad, _impactTime, _volume, _maxCharges, _rechargePerRound, _modelIndex){
    damage                   = _dmg;
    impactRadius             = _impactRad;
    impactTime               = _impactTime;
    travelSpeed              = _travelSpeed;
    volume                   = _volume;
    rangeInKMSquared         = 100.0 * 100.0;
}
const PrimaryWeaponCannonPrediction PrimaryWeaponCannon::calculatePredictedVector(EntityWrapper* mytarget, ComponentBody& projectileBody, const glm_vec3& chosen_target_pos_world) {
    auto& shipBody     = *ship.getComponent<ComponentBody>();
    auto shipRotation  = shipBody.rotation();
    auto cannonForward = glm::normalize(Math::rotate_vec3(shipRotation, forward));
    auto shipPosition  = shipBody.position();

    auto returnValue = PrimaryWeaponCannonPrediction();

    auto defaultVelocityVector = cannonForward * travelSpeed;
    auto currentVelocityVector = projectileBody.getLinearVelocity();
    auto finalSpeed = glm::length(defaultVelocityVector + currentVelocityVector);
    returnValue.finalProjectileSpeed = finalSpeed;
    if (mytarget) {
        auto& targetBody                = *mytarget->getComponent<ComponentBody>();
        auto* targetIsShip              = dynamic_cast<Ship*>(mytarget);

        const auto launcherPosition     = projectileBody.position();

        const auto vecToTarget          = launcherPosition - chosen_target_pos_world;
        const auto vecToForward         = launcherPosition - (cannonForward * static_cast<decimal>(100000000.0));
        const auto angleToTarget        = Math::getAngleBetweenTwoVectors(glm::normalize(vecToTarget), glm::normalize(vecToForward), true);
        if (angleToTarget <= arc) {
            const auto targetLinearVelocity = targetBody.getLinearVelocity();
            const auto distanceToTarget = glm::distance(chosen_target_pos_world, launcherPosition);

            const auto finalTravelTime = distanceToTarget / finalSpeed;

            auto myShipVelocity = shipBody.getLinearVelocity();
            auto combinedVelocity = targetLinearVelocity - myShipVelocity;
            const auto predictedSpeed = combinedVelocity + (cannonForward * finalTravelTime) /* * target.acceleration */; //TODO: figure this out later
            const auto averageSpeed = (combinedVelocity + predictedSpeed) * static_cast<decimal>(0.5);
            auto predictedPos = chosen_target_pos_world + (averageSpeed * finalTravelTime);

            returnValue.pedictedVector = -glm::normalize(launcherPosition - predictedPos);
            returnValue.pedictedPosition = predictedPos;
            return returnValue;
        }
    }
    returnValue.pedictedVector = cannonForward;
    returnValue.pedictedPosition = cannonForward;
    return returnValue;
}
const bool PrimaryWeaponCannon::isInControlledArc(EntityWrapper* target) {
    return isInArc(target, arc + 5.0f);
}
const int PrimaryWeaponCannon::acquire_index() {
    if (numRounds > 0) {
        return m_Map.get_and_use_next_cannon_projectile_index();
    }
    return -1;
}
const bool PrimaryWeaponCannon::forceFire(EntityWrapper* target, const int index, const glm_vec3& chosen_target_pos) {
    if (numRounds > 0) {
        const bool can = m_Map.try_addCannonProjectile(index);
        if (can) {
            Weapons::spawnProjectile(target , *this, ship, m_Map, position, forward, index, const_cast<glm_vec3&>(chosen_target_pos), modelIndex);
            return true;
        }
    }
    return false;
}
void PrimaryWeaponCannon::update(const double& dt) {
    if (soundEffect && (soundEffect->status() == SoundStatus::Playing || soundEffect->status() == SoundStatus::PlayingLooped)) {
        const auto shipRotation = ship.getRotation();
        const auto shipPosition = ship.getPosition();
        const auto launcherPosition = shipPosition + Math::rotate_vec3(shipRotation, position);
        soundEffect->setPosition(launcherPosition);
    }
    if (numRounds < numRoundsMax) {
        const float fdt = static_cast<float>(dt);
        rechargeTimer += fdt;
        if (rechargeTimer >= rechargeTimePerRound) {
            ++numRounds;
            rechargeTimer = 0.0f;
        }
    }
}

#pragma region Beam

PrimaryWeaponBeam::PrimaryWeaponBeam(WeaponType::Type _type, Ship& _ship, Map& map, const glm_vec3& _pos, const glm_vec3& _fwd, const float& _arc, const float& _dmg, const float& _impactRad, const float& _impactTime, const float& _volume, vector<glm::vec3>& _windupPts,const uint& _maxCharges,const float& _rechargeTimePerRound, const float& _chargeTimerSpeed, const float& _firingTime, const unsigned int& _modelIndex, const float& endpointExtraScale, const float& beamSizeExtraScale, const float& RangeInKM, const float& BeamLaunchSpeed) : ShipWeapon(map, _type, _ship, _pos, _fwd, _arc, _dmg, _impactRad, _impactTime, _volume, _maxCharges, _rechargeTimePerRound, _modelIndex) {
    windupPoints                = _windupPts;
    target                      = nullptr;
    targetCoordinates           = glm::vec3(0.0f);
    chargeTimer                 = 0.0f;
    chargeTimerSpeed            = _chargeTimerSpeed;
    state                       = BeamWeaponState::Off;
    firingTimeMax               = _firingTime;
    firingTime                  = 0.0f;
    firingTimeShieldGraphicPing = 0.0f;
    additionalEndPointScale     = endpointExtraScale;
    additionalBeamSizeScale     = beamSizeExtraScale;
    const auto range            = RangeInKM * 10.0f;
    rangeInKMSquared            = range * range;
    launchSpeed                 = BeamLaunchSpeed;

    beamLight = NEW RodLight(_pos, 2.0f, &map);
    beamLight->setAttenuation(LightRange::_20);
    beamLight->deactivate();

    beamGraphic = map.createEntity();

    auto* body = beamGraphic.addComponent<ComponentBody>();

    beamEndPointGraphic = map.createEntity();
    auto& planeMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh();
    auto& modelEndPt = *beamEndPointGraphic.addComponent<ComponentModel>(&planeMesh, (Material*)ResourceManifest::TorpedoGlowMaterial.get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& beamModelEnd = modelEndPt.getModel(0);
    beamModelEnd.hide();

    auto& body1 = *beamEndPointGraphic.addComponent<ComponentBody>(CollisionType::Box);

    btBoxShape& box = *static_cast<btBoxShape*>(body1.getCollision()->getBtShape());
    const auto& _scl = btVector3(0.001f, 0.001f, 0.001f);
    box.setLocalScaling(_scl);
    box.setMargin(0.01f);
    box.setImplicitShapeDimensions(_scl);
    beamModelEnd.setScale(0.17f * additionalBeamSizeScale);

    body1.addCollisionFlag(CollisionFlag::NoContactResponse);
    body1.setCollisionGroup(CollisionFilter::_Custom_2); //i belong to weapons (group 2)
    body1.setCollisionMask(CollisionFilter::_Custom_1 | CollisionFilter::_Custom_3); //i should only collide with shields and hull (group 1 and group 3)
    body1.setInternalPhysicsUserPointer(&body1);
    body1.setDamping(static_cast<decimal>(0.0), static_cast<decimal>(0.0));
}
PrimaryWeaponBeam::~PrimaryWeaponBeam() {
    beamGraphic.destroy();
    beamEndPointGraphic.destroy();
    if (beamLight) {
        beamLight->destroy();
        SAFE_DELETE(beamLight);
    }
}
void PrimaryWeaponBeam::setTarget(EntityWrapper* target_) {
    target = target_;
}
EntityWrapper* PrimaryWeaponBeam::getTarget() {
    return target;
}

void PrimaryWeaponBeam::modifyBeamMesh(ComponentModel& beamModel, const float length) {
    auto& mesh = *(Mesh*)ResourceManifest::PhaserBeamMesh.get();
    VertexData& data = const_cast<VertexData&>(mesh.getVertexData());
    modPts = data.getData<glm::vec3>(0);
    modUvs = data.getData<glm::vec2>(1);

    auto& beamModelOne = beamModel.getModel(0);
    const auto len2 = length * (1.0f / beamModelOne.getScale().x);

    if (modPts.size() == 0 || modUvs.size() == 0) //TODO: find out why this is needed
        return;

    modPts[0].z = len2;
    modPts[4].z = len2;
    modPts[6].z = len2;
    modPts[10].z = len2;
    modPts[12].z = len2;
    modPts[16].z = len2;
    modPts[18].z = len2;
    modPts[22].z = len2;
    modPts[24].z = len2;
    modPts[25].z = len2;
    modPts[26].z = len2;
    modPts[27].z = len2;
    modPts[28].z = len2;
    modPts[29].z = len2;
    modPts[30].z = len2;
    modPts[31].z = len2;


    modUvs[0].x = length;
    modUvs[4].x = length;
    modUvs[6].x = length;
    modUvs[10].x = length;
    modUvs[12].x = length;
    modUvs[16].x = length;
    modUvs[18].x = length;
    modUvs[22].x = length;
    modUvs[24].x = length;
    modUvs[25].x = length;
    modUvs[26].x = length;
    modUvs[27].x = length;
    modUvs[28].x = length;
    modUvs[29].x = length;
    modUvs[30].x = length;
    modUvs[31].x = length;
}
const glm_vec3 PrimaryWeaponBeam::calculatePredictedVector() {
    auto ret = glm_vec3(0.0f);
    auto mytarget = ship.getTarget();
    if (mytarget) {
        auto& targetBody = *mytarget->getComponent<ComponentBody>();
        auto* targetIsShip = dynamic_cast<Ship*>(mytarget);
        if (targetIsShip) {
            ret = targetIsShip->getAimPositionRandom();
        }else{
            ret = targetBody.position();
        }
    }
    return ret;
}
const int PrimaryWeaponBeam::acquire_index() {
    if (numRounds > 0) {
        return 1;
    }
    return -1;
}
const bool PrimaryWeaponBeam::fire(const double& dt, const glm_vec3& chosen_target_pt) {
    if (numRounds > 0) {
        return true;
    }
    return false;
}
const bool PrimaryWeaponBeam::forceFire(const double& dt) {
    return false;
}

void PrimaryWeaponBeam::internal_update_initial_firing(const double& dt) {
    auto& firstWindupModel = *firstWindupGraphic.getComponent<ComponentModel>();
    auto& secondWindupModel = *secondWindupGraphic.getComponent<ComponentModel>();
    firstWindupModel.show();
    secondWindupModel.show();
    firstWindupLight->activate();
    secondWindupLight->activate();
    beamLight->activate();
    state = BeamWeaponState::WindingUp;
}
void PrimaryWeaponBeam::internal_update_winding_up(const double& dt) {
    glm::vec3 firstWindupPos;
    glm::vec3 secondWindupPos;
    auto& firstWindupBody = *firstWindupGraphic.getComponent<ComponentBody>();
    auto& secondWindupBody = *secondWindupGraphic.getComponent<ComponentBody>();
    auto& firstWindupLightBody = *firstWindupLight->getComponent<ComponentBody>();
    auto& secondWindupLightBody = *secondWindupLight->getComponent<ComponentBody>();

    auto& firstWindupModel = *firstWindupGraphic.getComponent<ComponentModel>();
    auto& secondWindupModel = *secondWindupGraphic.getComponent<ComponentModel>();

    auto& beamEndModel = *beamEndPointGraphic.getComponent<ComponentModel>();

    auto& cam = *firstWindupBody.getOwner().scene().getActiveCamera();
    auto camRotation = cam.getOrientation();

    firstWindupModel.getModel().setOrientation(camRotation);
    secondWindupModel.getModel().setOrientation(camRotation);
    beamEndModel.getModel().setOrientation(camRotation);

    const auto chargeSpeedModifier = static_cast<float>(dt) * chargeTimerSpeed;
    chargeTimer += chargeSpeedModifier;

    //place the windups properly
    const auto shipRotation = ship.getRotation();
    const auto shipPosition = ship.getPosition();
    const auto launcherPosition = shipPosition + Math::rotate_vec3(shipRotation, position);
    if (soundEffect) {
        soundEffect->setPosition(launcherPosition);
    }
    if (windupPoints.size() == 1) {
        firstWindupPos = secondWindupPos = (shipPosition + Math::rotate_vec3(shipRotation, windupPoints[0]));
    }else{
        const auto halfCharge = chargeTimer * 0.5f;
        firstWindupPos = shipPosition + Math::rotate_vec3(shipRotation, Math::polynomial_interpolate_cubic(windupPoints, halfCharge));
        secondWindupPos = shipPosition + Math::rotate_vec3(shipRotation, Math::polynomial_interpolate_cubic(windupPoints, 1.0f - halfCharge));
    }
    firstWindupBody.setPosition(firstWindupPos);
    secondWindupBody.setPosition(secondWindupPos);
    firstWindupLightBody.setPosition(firstWindupPos);
    secondWindupLightBody.setPosition(secondWindupPos);
    if (chargeTimer >= 1.0f) {
        state = BeamWeaponState::Firing;
        chargeTimer = 1.0f;
        --numRounds;
    }
}
void PrimaryWeaponBeam::internal_update_firing(const double& dt) {
    const auto fdt = static_cast<float>(dt);
    firingTime += fdt;
    firingTimeShieldGraphicPing += fdt;
    glm::vec3 firstWindupPos;
    glm::vec3 secondWindupPos;
    auto& firstWindupBody = *firstWindupGraphic.getComponent<ComponentBody>();
    auto& secondWindupBody = *secondWindupGraphic.getComponent<ComponentBody>();
    auto& firstWindupLightBody = *firstWindupLight->getComponent<ComponentBody>();
    auto& secondWindupLightBody = *secondWindupLight->getComponent<ComponentBody>();

    auto& firstWindupModel = *firstWindupGraphic.getComponent<ComponentModel>();
    auto& secondWindupModel = *secondWindupGraphic.getComponent<ComponentModel>();
    auto& beamLightBody = *beamLight->getComponent<ComponentBody>();

    auto& beamEndBody = *beamEndPointGraphic.getComponent<ComponentBody>();
    auto& beamEndModel = *beamEndPointGraphic.getComponent<ComponentModel>();
    auto& body = *beamGraphic.getComponent<ComponentBody>();

    auto& beamModel = *beamGraphic.getComponent<ComponentModel>();
    auto& beamModelInstance = beamModel.getModel(0);
    beamEndModel.show();
    beamModelInstance.show();
    beamModelInstance.forceRender(true);


    Ship* targetShip = dynamic_cast<Ship*>(target);
    const auto shipRotation = ship.getRotation();
    const auto shipPosition = ship.getPosition();
    unsigned short group = CollisionFilter::_Custom_2;
    unsigned short mask = -1;
    mask = mask & ~CollisionFilter::_Custom_4; //do not ray cast against the convex hull (custom 4)
    mask = mask & ~CollisionFilter::_Custom_5; //do not ray cast against the shields if they are down
    mask = mask & ~CollisionFilter::_Custom_2; //and ignore other weapons!
    auto* shipShields = static_cast<ShipSystemShields*>(ship.getShipSystem(ShipSystemType::Shields));
    auto* shipHull = static_cast<ShipSystemHull*>(ship.getShipSystem(ShipSystemType::Hull));
    vector<Entity> ignored; ignored.reserve(3);
    if (shipShields) {
        ignored.push_back(shipShields->getEntity());
    }
    ignored.push_back(shipHull->getEntity());
    ignored.push_back(ship.entity());
    auto target_central_position = glm::vec3(target->getComponent<ComponentBody>()->position());

    
    glm::vec3 target_world_position = target_central_position + targetCoordinates;
    if (windupPoints.size() == 1) {
        firstWindupPos = secondWindupPos = (shipPosition + Math::rotate_vec3(shipRotation, windupPoints[0]));
    }else{
        const auto halfCharge = chargeTimer * 0.5f;
        firstWindupPos = shipPosition + Math::rotate_vec3(shipRotation, Math::polynomial_interpolate_cubic(windupPoints, halfCharge));
        secondWindupPos = shipPosition + Math::rotate_vec3(shipRotation, Math::polynomial_interpolate_cubic(windupPoints, 1.0f - halfCharge));
    }
    glm::vec3 beam_starting_position = firstWindupPos;
    glm::vec3 beam_ending_position = firstWindupPos;

    auto rayCastPoints = Physics::rayCast(beam_starting_position, target_world_position, ignored, group, mask);
    Engine::RayCastResult* closest = nullptr;

    auto lambda_get_closest = [&](vector<Engine::RayCastResult>& vec, float& min_dist, size_t& closest_index, Engine::RayCastResult*& closest_, const glm::vec3& startPos_) {
        for (size_t i = 0; i < vec.size(); ++i) {
            const auto dist = glm::distance2(vec[i].hitPosition, startPos_);
            if (dist < min_dist) {
                min_dist = dist;
                closest_ = &const_cast<Engine::RayCastResult&>(vec[i]);
                closest_index = i;
            }
        }
    };

    //get closest 2 points
    float minDist = 9999999999999.0f;
    size_t closestIndex = 0;
    lambda_get_closest(rayCastPoints, minDist, closestIndex, closest, beam_starting_position);
    if (!closest || closest->hitNormal == glm::vec3(0.0f)) {
        state = BeamWeaponState::JustTurnedOff;
        return;
    }
    if (targetShip) {
        auto* targetShields = static_cast<ShipSystemShields*>(targetShip->getShipSystem(ShipSystemType::Shields));
        if (targetShields) {
            auto positionModelSpace = (closest->hitPosition - target_central_position) * glm::quat(targetShip->getRotation());
            const auto side = targetShields->getImpactSide(positionModelSpace);
            if (targetShields->getHealthCurrent(side) <= 0.0f) {
                rayCastPoints.erase(rayCastPoints.begin() + closestIndex);
            }
        }
        minDist = 9999999999999.0f;
        closestIndex = 0;
        lambda_get_closest(rayCastPoints, minDist, closestIndex, closest, beam_starting_position);
    }
    target_world_position = closest->hitPosition;

    glm::vec3 vector_to_target = beam_starting_position - target_world_position;

    const float length_to_target = glm::length(vector_to_target);
    const auto normal_to_target = vector_to_target / length_to_target;

    float end_point_length   = glm::min(firingTime * launchSpeed, length_to_target) - 0.004f;
    float start_point_length = glm::min((firingTimeMax - firingTime) * launchSpeed, length_to_target);
    end_point_length = glm::max(0.0f, end_point_length);
    start_point_length = glm::max(0.0f, start_point_length);

    beam_ending_position = firstWindupPos + (-normal_to_target * end_point_length);
    beam_starting_position = target_world_position - (-normal_to_target * start_point_length);

    const auto len2 = glm::min(start_point_length, end_point_length);
    glm_quat q = glm_quat(1.0, 0.0, 0.0, 0.0);
    Math::alignTo(q, -normal_to_target);
    beamModelInstance.setOrientation(q);

    body.setPosition(beam_starting_position);
    firstWindupBody.setPosition(beam_starting_position);
    secondWindupBody.setPosition(beam_starting_position);
    firstWindupLightBody.setPosition(beam_starting_position);
    secondWindupLightBody.setPosition(beam_starting_position);

    beamEndBody.setPosition(beam_ending_position);
    beamEndBody.setRotation(q);
    const glm::vec3 midpt = Math::midpoint(beam_starting_position, beam_ending_position);
    beamLightBody.setPosition(midpt);
    beamLightBody.setRotation(q);
    beamLight->setRodLength(len2);
    modifyBeamMesh(beamModel, len2);
  


    auto& cam = *firstWindupBody.getOwner().scene().getActiveCamera();
    auto camRotation = cam.getOrientation();
    firstWindupModel.getModel().setOrientation(camRotation);
    secondWindupModel.getModel().setOrientation(camRotation);
    beamEndModel.getModel().setOrientation(camRotation);



    const auto x = firingTimeMax - (len2 / launchSpeed);
    if (!isInArc(target, arc + 5.0f) && firingTime < x) {
        firingTime = x;
    }  
    if (firingTime >= firingTimeMax) {
        state = BeamWeaponState::JustTurnedOff;
    }
}
void PrimaryWeaponBeam::internal_update_ending(const double& dt) {
    auto& firstWindupModel  = *firstWindupGraphic.getComponent<ComponentModel>();
    auto& secondWindupModel = *secondWindupGraphic.getComponent<ComponentModel>();
    auto& beamEndModel      = *beamEndPointGraphic.getComponent<ComponentModel>();
    auto& beamModel         = *beamGraphic.getComponent<ComponentModel>();
    auto& beamEndBody       = *beamEndPointGraphic.getComponent<ComponentBody>();

    firingTime = 0.0f;
    chargeTimer = 0.0f;
    firstWindupModel.hide();
    secondWindupModel.hide();
    beamModel.hide();
    beamEndModel.hide();
    beamEndBody.setPosition(99999999999999.0f, 9999999999999999999999.0f, 9999999999999999999999999.0f);
    firstWindupLight->deactivate();
    secondWindupLight->deactivate();
    beamLight->deactivate();
    if (soundEffect) {
        soundEffect->stop();
    }
    state = BeamWeaponState::Off;
}

void PrimaryWeaponBeam::update(const double& dt) {
    if (numRounds < numRoundsMax) {
        const float fdt = static_cast<float>(dt);
        rechargeTimer += fdt;
        if (rechargeTimer >= rechargeTimePerRound) {
            ++numRounds;
            rechargeTimer = 0.0f;
        }
    }
}
#pragma endregion

#pragma region Torpedo

SecondaryWeaponTorpedo::SecondaryWeaponTorpedo(Map& map, WeaponType::Type _type, Ship& _ship,const glm_vec3& _pos,const glm_vec3& _fwd,const float& _arc,const uint& _maxCharges,const float& _dmg,const float& _rechargePerRound,const float& _impactRad,const float& _impactTime,const float& _travelSpeed,const float& _volume,const float& _rotAngleSpeed, const unsigned int& _modelIndex) : ShipWeapon(map, _type, _ship,_pos,_fwd,_arc, _dmg, _impactRad, _impactTime, _volume, _maxCharges, _rechargePerRound, _modelIndex) {
    damage                   = _dmg;
    impactRadius             = _impactRad;
    impactTime               = _impactTime;
    travelSpeed              = _travelSpeed;
    volume                   = _volume;
    rotationAngleSpeed       = _rotAngleSpeed;
    rangeInKMSquared         = 100.0 * 100.0;
}

const SecondaryWeaponTorpedoPrediction SecondaryWeaponTorpedo::calculatePredictedVector(EntityWrapper* mytarget, ComponentBody& projectileBody, const glm_vec3& chosen_target_pos_world) {
    auto& shipBody = *ship.getComponent<ComponentBody>();
    auto shipRotation = shipBody.rotation();
    auto torpForward = glm::normalize(Math::rotate_vec3(shipRotation, forward));
    auto shipPosition = shipBody.position();

    auto returnValue = SecondaryWeaponTorpedoPrediction();
    auto defaultVelocityVector = torpForward * travelSpeed;
    auto currentVelocityVector = projectileBody.getLinearVelocity();
    auto finalSpeed = glm::length(defaultVelocityVector + currentVelocityVector);
    returnValue.finalProjectileSpeed = finalSpeed;
    if (mytarget) {
        auto& targetBody = *mytarget->getComponent<ComponentBody>();
        auto* targetIsShip = dynamic_cast<Ship*>(mytarget);

        const auto launcherPosition = projectileBody.position();

        const auto vecToTarget = launcherPosition - chosen_target_pos_world;
        const auto vecToForward = launcherPosition - (torpForward * static_cast<decimal>(100000000.0));
        const auto angleToTarget = Math::getAngleBetweenTwoVectors(glm::normalize(vecToTarget), glm::normalize(vecToForward), true);
        if (angleToTarget <= arc) {
            returnValue.hasLock = true;
            returnValue.target = mytarget;

            const auto targetLinearVelocity = targetBody.getLinearVelocity();
            const auto distanceToTarget = glm::distance(chosen_target_pos_world, launcherPosition);


            const auto finalTravelTime = distanceToTarget / finalSpeed;
            auto myShipVelocity = shipBody.getLinearVelocity();

            auto combinedVelocity = targetLinearVelocity - myShipVelocity;
            const auto predictedSpeed = combinedVelocity + (torpForward * finalTravelTime) /* * target.acceleration */; //TODO: figure this out later
            const auto averageSpeed = (combinedVelocity + predictedSpeed) * static_cast<decimal>(0.5);
            auto predictedPos = chosen_target_pos_world + (averageSpeed * finalTravelTime);

            returnValue.pedictedPosition = predictedPos;
            returnValue.pedictedVector = -glm::normalize(launcherPosition - predictedPos);
            return returnValue;
        }
    }
    //not predicted firing
    returnValue.hasLock = false;
    returnValue.pedictedPosition = torpForward;
    returnValue.pedictedVector = torpForward;
    return returnValue;
}
const bool SecondaryWeaponTorpedo::isInControlledArc(EntityWrapper* target) {
    return isInArc(target, arc + 10.0f);
}
const int SecondaryWeaponTorpedo::acquire_index() {
    if (numRounds > 0) {
        return m_Map.get_and_use_next_torpedo_projectile_index();
    }
    return -1;
}
const bool SecondaryWeaponTorpedo::forceFire(EntityWrapper* target, const int index, const glm_vec3& chosen_target_pos) {
    if (numRounds > 0) {
        const bool can = m_Map.try_addTorpedoProjectile(index);
        if (can) {
            Weapons::spawnProjectile(target, *this, ship, m_Map, position, forward, index, const_cast<glm_vec3&>(chosen_target_pos), modelIndex);
            return true;
        }
    }
    return false;
}
void SecondaryWeaponTorpedo::update(const double& dt) {
    if (soundEffect && (soundEffect->status() == SoundStatus::Playing || soundEffect->status() == SoundStatus::PlayingLooped)) {
        const auto shipRotation = ship.getRotation();
        const auto shipPosition = ship.getPosition();
        const auto launcherPosition = shipPosition + Math::rotate_vec3(shipRotation, position);
        soundEffect->setPosition(launcherPosition);
    }
    if (numRounds < numRoundsMax) {
        const float fdt = static_cast<float>(dt);
        rechargeTimer += fdt;
        if (rechargeTimer >= rechargeTimePerRound) {
            ++numRounds;
            rechargeTimer = 0.0f;
        }
    }
}

SecondaryWeaponTorpedoProjectile::SecondaryWeaponTorpedoProjectile(Map& _map, const glm_vec3& position, const glm_vec3& forward, const int index) : WeaponProjectile(_map, index) {
    maxTime            = 30.5f;
    target             = nullptr;
    rotationAngleSpeed = 0.0f;
    active             = true;
    hasLock            = false;
}
SecondaryWeaponTorpedoProjectile::~SecondaryWeaponTorpedoProjectile() {
    //active = false;
}
void SecondaryWeaponTorpedoProjectile::update(const double& dt) {
    if (active) {
        auto& glowModel = *entity.getComponent<ComponentModel>();
        auto& body = *entity.getComponent<ComponentBody>();
        const auto glowBodyPos = glm_vec3(body.position_render());
        auto* activeCam = glowModel.getOwner().scene().getActiveCamera();
        const auto camPos = activeCam->getPosition();

        //TODO: hacky workaround for messed up camera forward vector
        auto vec = glm::normalize(glowBodyPos - camPos);
        vec *= static_cast<decimal>(0.01);
        body.setRotation(activeCam->getOrientation());
        glowModel.getModel(1).setPosition(vec);

    }
    WeaponProjectile::update(dt);
}
void SecondaryWeaponTorpedoProjectile::destroy() {
    WeaponProjectile::destroy();
    map.removeTorpedoProjectile(projectile_index);
}
#pragma endregion




ShipSystemWeapons::ShipSystemWeapons(Ship& _ship) : ShipSystem(ShipSystemType::Weapons, _ship) {
    cannonTargetPoint = torpedoTargetPoint = glm::vec3(0.0f);
}
ShipSystemWeapons::~ShipSystemWeapons() {
    for (auto& weapon : m_PrimaryWeaponsCannons)
        SAFE_DELETE(weapon.cannon);
    for (auto& weapon : m_PrimaryWeaponsBeams)
        SAFE_DELETE(weapon.beam);
    for (auto& weapon : m_SecondaryWeaponsTorpedos)
        SAFE_DELETE(weapon.torpedo);
}
void ShipSystemWeapons::fireBeamWeapons(EntityWrapper* target, Ship* target_as_ship, vector<ShipSystemWeapons::WeaponBeam>& beamWeapons) {
    vector<std::tuple<uint, EntityWrapper*>> primaryWeaponsBeamsFired;
    for (size_t i = 0; i < beamWeapons.size(); ++i) {
        const int res = beamWeapons[i].beam->acquire_index();
        if (res >= 0) {
            if (target && !target_as_ship || (target_as_ship && !target_as_ship->isAlly(m_Ship))) {
                primaryWeaponsBeamsFired.push_back(std::make_tuple(static_cast<unsigned int>(i), target));
            }
        }
    }
    //send packet with indices
    if (primaryWeaponsBeamsFired.size() > 0) {
        PacketMessage pOut;
        pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Beams;
        pOut.name = m_Ship.getName();
        pOut.data = to_string(std::get<0>(primaryWeaponsBeamsFired[0]));

        glm::vec3 target_point;

        if (target_as_ship) {
            target_point = target_as_ship->getAimPositionRandomLocal();
        }else{
            target_point = glm::vec3(0.0f);
        }
        pOut.r = target_point.r;
        pOut.g = target_point.g;
        pOut.b = target_point.b;

        auto* target_ptr = std::get<1>(primaryWeaponsBeamsFired[0]);
        string target_name = "N/A";
        if (target_ptr) {
            if (target_as_ship) {
                target_name = target_as_ship->getMapKey();
            }else{
                target_name = target_ptr->getComponent<ComponentName>()->name();
            }
        }
        pOut.data += "," + target_name;
        for (size_t i = 1; i < primaryWeaponsBeamsFired.size(); ++i) {
            pOut.data += "," + to_string(std::get<0>(primaryWeaponsBeamsFired[i]));

            target_ptr = std::get<1>(primaryWeaponsBeamsFired[0]);
            target_name = "N/A";
            if (target_ptr) {
                if (target_as_ship) {
                    target_name = target_as_ship->getMapKey();
                }else{
                    target_name = target_ptr->getComponent<ComponentName>()->name();
                }
            }
            pOut.data += "," + target_name;
        }
        m_Ship.m_Client.send(pOut);
    }
}
void ShipSystemWeapons::fireCannonWeapons(EntityWrapper* target, Ship* target_as_ship, vector<ShipSystemWeapons::WeaponCannon>& cannonWeapons) {
    vector<std::tuple<uint, int, EntityWrapper*>> primaryWeaponsCannonsFired;
    if (target && !target_as_ship || (target_as_ship && !target_as_ship->isAlly(m_Ship))) {
        for (size_t i = 0; i < cannonWeapons.size(); ++i) {
            auto* cannon = cannonWeapons[i].cannon;
            if (cannon->isInControlledArc(target)) {
                const int resIndex = cannon->acquire_index();
                if (resIndex >= 0) {
                    primaryWeaponsCannonsFired.push_back(std::make_tuple(static_cast<unsigned int>(i), resIndex, target));
                }
            }
        }
    }else{
        for (size_t i = 0; i < cannonWeapons.size(); ++i) {
            auto& cannon = cannonWeapons[i];
            if (cannon.isForward) {
                const int resIndex = cannon.cannon->acquire_index();
                if (resIndex >= 0) {
                    primaryWeaponsCannonsFired.push_back(std::make_tuple(static_cast<unsigned int>(cannon.main_container_index), resIndex, nullptr));
                }
            }
        }
    }
    if (primaryWeaponsCannonsFired.size() > 0) {
        PacketMessage pOut;
        pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Cannons;
        pOut.name = m_Ship.getName();
        /*
        TODO: add a bit of random offset to this...
        */
        pOut.r = cannonTargetPoint.x;
        pOut.g = cannonTargetPoint.y;
        pOut.b = cannonTargetPoint.z;
        pOut.data = to_string(std::get<0>(primaryWeaponsCannonsFired[0])) + "," + to_string(std::get<1>(primaryWeaponsCannonsFired[0]));

        auto* target_ptr = std::get<2>(primaryWeaponsCannonsFired[0]);
        string target_name = "N/A";
        if (target_ptr) {
            if (target_as_ship) {
                target_name = target_as_ship->getMapKey();
            }else{
                target_name = target_ptr->getComponent<ComponentName>()->name();
            }
        }
        pOut.data += "," + target_name;

        for (size_t i = 1; i < primaryWeaponsCannonsFired.size(); ++i) {
            pOut.data += "," + to_string(std::get<0>(primaryWeaponsCannonsFired[i])) + "," + to_string(std::get<1>(primaryWeaponsCannonsFired[i]));

            target_ptr = std::get<2>(primaryWeaponsCannonsFired[i]);
            target_name = "N/A";
            if (target_ptr) {
                if (target_as_ship) {
                    target_name = target_as_ship->getMapKey();
                }else{
                    target_name = target_ptr->getComponent<ComponentName>()->name();
                }
            }
            pOut.data += "," + target_name;
        }
        m_Ship.m_Client.send(pOut);
    }
}
void ShipSystemWeapons::fireTorpedoWeapons(EntityWrapper* target, Ship* target_as_ship, vector<ShipSystemWeapons::WeaponTorpedo>& torpedoWeapons) {
    vector<std::tuple<uint, int, EntityWrapper*>> secWeaponsTorpedosFired;

    //first, get the arcs that are in range of the target if applicable
    vector<std::tuple<uint, int, EntityWrapper*>> secWeaponsTorpedosFiredInValidArc;
    if (target && !target_as_ship || (target_as_ship && !target_as_ship->isAlly(m_Ship))) {
        for (size_t i = 0; i < torpedoWeapons.size(); ++i) {
            auto* torpedo = torpedoWeapons[i].torpedo;
            if (torpedo->isInControlledArc(target)) {
                const int resIndex = torpedo->acquire_index();
                if (resIndex >= 0) {
                    secWeaponsTorpedosFiredInValidArc.push_back(std::make_tuple(static_cast<unsigned int>(i), resIndex, target));
                }
            }
        }
    }else{
        for (size_t i = 0; i < torpedoWeapons.size(); ++i) {
            auto* torpedo = torpedoWeapons[i].torpedo;
            if (torpedoWeapons[i].isForward) {
                const int resIndex = torpedo->acquire_index();
                if (resIndex >= 0) {
                    secWeaponsTorpedosFiredInValidArc.push_back(std::make_tuple(static_cast<unsigned int>(i), resIndex, nullptr));
                }
            }
        }
    }

    //now we have our valid torpedo launchers, pick the launcher with the most rounds
    uint maxRounds = 0;
    uint chosenIndex = 0;
    EntityWrapper* chosen_torp_target = nullptr;
    for (auto& weapIndex : secWeaponsTorpedosFiredInValidArc) {
        auto& launcher = torpedoWeapons[std::get<0>(weapIndex)].torpedo;
        if (launcher->numRounds > maxRounds) {
            maxRounds = launcher->numRounds;
            chosenIndex = std::get<0>(weapIndex);
            chosen_torp_target = std::get<2>(weapIndex);
        }
    }
    //now fire this chosen launcher
    if (secWeaponsTorpedosFiredInValidArc.size() > 0) {
        const int resIndex = torpedoWeapons[chosenIndex].torpedo->acquire_index();
        if (resIndex >= 0) {
            secWeaponsTorpedosFired.push_back(std::make_tuple(chosenIndex, resIndex, chosen_torp_target));
        }
    }

    //send packet with indices
    if (secWeaponsTorpedosFired.size() > 0) {
        PacketMessage pOut;
        pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Torpedos;
        pOut.name = m_Ship.getName();
        /*
        TODO: add a bit of random offset to this...
        */
        pOut.r = torpedoTargetPoint.x;
        pOut.g = torpedoTargetPoint.y;
        pOut.b = torpedoTargetPoint.z;
        pOut.data = to_string(std::get<0>(secWeaponsTorpedosFired[0])) + "," + to_string(std::get<1>(secWeaponsTorpedosFired[0]));

        auto* target_ptr = std::get<2>(secWeaponsTorpedosFired[0]);
        string target_name = "N/A";
        if (target_ptr) {
            if (target_as_ship) {
                target_name = target_as_ship->getMapKey();
            }else{
                target_name = target_ptr->getComponent<ComponentName>()->name();
            }
        }
        pOut.data += "," + target_name;
        for (size_t i = 1; i < secWeaponsTorpedosFired.size(); ++i) {
            pOut.data += "," + to_string(std::get<0>(secWeaponsTorpedosFired[i])) + "," + to_string(std::get<1>(secWeaponsTorpedosFired[i]));
            target_ptr = std::get<2>(secWeaponsTorpedosFired[i]);
            target_name = "N/A";
            if (target_ptr) {
                if (target_as_ship) {
                    target_name = target_as_ship->getMapKey();
                }else{
                    target_name = target_ptr->getComponent<ComponentName>()->name();
                }
            }
            pOut.data += "," + target_name;
        }
        m_Ship.m_Client.send(pOut);
    }
}

void ShipSystemWeapons::update(const double& dt) {
    const bool isDestroyed = m_Ship.isDestroyed();

    if (!isDestroyed) {
        const bool isCloaked = m_Ship.isCloaked();
        const bool isWarping = m_Ship.IsWarping();
        const bool isPlayer  = m_Ship.IsPlayer();

        auto* mytarget = m_Ship.getTarget();
        Ship* ship = nullptr;
        if (mytarget) {
            ship = dynamic_cast<Ship*>(mytarget);
            if (ship) {
                cannonTargetPoint = ship->getAimPositionRandomLocal();
                torpedoTargetPoint = ship->getAimPositionRandomLocal();
            }else{
                cannonTargetPoint = glm::vec3(0.0f);
                torpedoTargetPoint = glm::vec3(0.0f);
            }
        }
        if (!isCloaked && !isWarping) {
            if (isPlayer && Engine::isMouseButtonDownOnce(MouseButton::Left)) {
                m_Ship.fireBeams(*this, mytarget, ship);
                m_Ship.fireCannons(*this, mytarget, ship);
            }
            if (isPlayer && Engine::isMouseButtonDownOnce(MouseButton::Right)) {
                m_Ship.fireTorpedos(*this, mytarget, ship);
            }
        }
    }
    for (auto& beam : m_PrimaryWeaponsBeams) { 
        beam.beam->update(dt); 
    }
    for (auto& cannon : m_PrimaryWeaponsCannons) { 
        cannon.cannon->update(dt); 
    }
    for (auto& torpedo : m_SecondaryWeaponsTorpedos) { 
        torpedo.torpedo->update(dt); 
    }
}
void ShipSystemWeapons::addPrimaryWeaponBeam(PrimaryWeaponBeam& beam, const bool isForwardWeapon) {
    beam.index = m_PrimaryWeaponsBeams.size();

    WeaponBeam b;
    b.main_container_index = beam.index;
    b.beam = &beam;
    if (isForwardWeapon) {
        b.isForward = true;
    }else{
        b.isForward = false;
    }
    m_PrimaryWeaponsBeams.push_back(b);

}
void ShipSystemWeapons::addPrimaryWeaponCannon(PrimaryWeaponCannon& cannon, const bool isForwardWeapon) {
    cannon.index = m_PrimaryWeaponsCannons.size();

    WeaponCannon c;
    c.main_container_index = cannon.index;
    c.cannon = &cannon;
    if (isForwardWeapon) {
        c.isForward = true;
    }else{
        c.isForward = false;
    }
    m_PrimaryWeaponsCannons.push_back(c);
}
void ShipSystemWeapons::addSecondaryWeaponTorpedo(SecondaryWeaponTorpedo& torpedo, const bool isForwardWeapon) {
    torpedo.index = m_SecondaryWeaponsTorpedos.size();

    WeaponTorpedo t;
    t.main_container_index = torpedo.index;
    t.torpedo = &torpedo;
    if (isForwardWeapon) {
        t.isForward = true;
    }else{
        t.isForward = false;
    }
    m_SecondaryWeaponsTorpedos.push_back(t);
}
PrimaryWeaponBeam& ShipSystemWeapons::getPrimaryWeaponBeam(const uint index) {
    return *m_PrimaryWeaponsBeams[index].beam;
}
PrimaryWeaponCannon& ShipSystemWeapons::getPrimaryWeaponCannon(const uint index) {
    return *m_PrimaryWeaponsCannons[index].cannon;
}
SecondaryWeaponTorpedo& ShipSystemWeapons::getSecondaryWeaponTorpedo(const uint index) {
    return *m_SecondaryWeaponsTorpedos[index].torpedo;
}

vector<ShipSystemWeapons::WeaponCannon>& ShipSystemWeapons::getCannons() {
    return m_PrimaryWeaponsCannons;
}
vector<ShipSystemWeapons::WeaponBeam>& ShipSystemWeapons::getBeams() {
    return m_PrimaryWeaponsBeams;
}
vector<ShipSystemWeapons::WeaponTorpedo>& ShipSystemWeapons::getTorpedos() {
    return m_SecondaryWeaponsTorpedos;
}
