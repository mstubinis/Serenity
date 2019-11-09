#include "ShipSystemWeapons.h"
#include "ShipSystemSensors.h"
#include "../../weapons/Weapons.h"
#include "../../Ship.h"
#include "../../ResourceManifest.h"
#include "../../networking/Packet.h"
#include "../../map/Map.h"
#include <core/engine/lights/Lights.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/math/Engine_Math.h>

#include <core/engine/events/Engine_Events.h>
#include <core/engine/math/Engine_Math.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>
#include <glm/gtx/norm.hpp>

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
void WeaponProjectile::clientToServerImpactShields(const bool cannon, Client& client, Ship& shipHit, const glm::vec3& impactLocalPosition, const glm::vec3& impactNormal, const float& impactRadius, const float& damage, const float& time, const unsigned int& shield_face) {
    PacketProjectileImpact packet;
    if(cannon)
        packet.PacketType = PacketType::Client_To_Server_Projectile_Cannon_Impact;
    else
        packet.PacketType = PacketType::Client_To_Server_Projectile_Torpedo_Impact;
    packet.damage = damage;
    packet.impactX = impactLocalPosition.x;
    packet.impactY = impactLocalPosition.y;
    packet.impactZ = impactLocalPosition.z;
    packet.shields = true;
    packet.shield_side = shield_face;
    packet.data = shipHit.getName() + ",";
    packet.projectile_index = projectile_index;
    Math::Float16From32(&packet.time, time);
    Math::Float16From32(&packet.radius, impactRadius);
    Math::Float16From32(&packet.normalX, impactNormal.x);
    Math::Float16From32(&packet.normalY, impactNormal.y);
    Math::Float16From32(&packet.normalZ, impactNormal.z);
    active = false;
    client.send(packet);
}
void WeaponProjectile::clientToServerImpactHull(const bool cannon, Client& client, Ship& shipHit, const glm::vec3& impactLocalPosition, const glm::vec3& impactNormal, const float& impactRadius, const float& damage, const float& time) {
    PacketProjectileImpact packet;
    if (cannon)
        packet.PacketType = PacketType::Client_To_Server_Projectile_Cannon_Impact;
    else
        packet.PacketType = PacketType::Client_To_Server_Projectile_Torpedo_Impact;
    packet.damage = damage;
    packet.impactX = impactLocalPosition.x;
    packet.impactY = impactLocalPosition.y;
    packet.impactZ = impactLocalPosition.z;
    packet.shields = false;
    packet.shield_side = -1;
    packet.data = shipHit.getName() + ",";
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
const int PrimaryWeaponCannon::canFire() {
    if (numRounds > 0) {
        return m_Map.try_addCannonProjectile();
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

PrimaryWeaponBeam::PrimaryWeaponBeam(WeaponType::Type _type, Ship& _ship, Map& map, const glm_vec3& _pos, const glm_vec3& _fwd, const float& _arc, const float& _dmg, const float& _impactRad, const float& _impactTime, const float& _volume, vector<glm::vec3>& _windupPts,const uint& _maxCharges,const float& _rechargeTimePerRound, const float& _chargeTimerSpeed, const float& _firingTime, const unsigned int& _modelIndex, const float& endpointExtraScale, const float& beamSizeExtraScale, const float& RangeInKM) : ShipWeapon(map, _type, _ship, _pos, _fwd, _arc, _dmg, _impactRad, _impactTime, _volume, _maxCharges, _rechargeTimePerRound, _modelIndex) {
    windupPoints = _windupPts;
    target = nullptr;
    targetCoordinates = glm::vec3(0.0f);
    chargeTimer = 0.0f;
    chargeTimerSpeed = _chargeTimerSpeed;
    state = BeamWeaponState::Off;
    firingTimeMax = _firingTime;
    firingTime = 0.0f;
    firingTimeShieldGraphicPing = 0.0f;
    additionalEndPointScale = endpointExtraScale;
    additionalBeamSizeScale = beamSizeExtraScale;
    auto range = RangeInKM * 10.0f;
    rangeInKMSquared = range * range;

    beamLight = new RodLight(_pos, 2.0f, &map);
    beamLight->setAttenuation(LightRange::_7);
    beamLight->deactivate();

    beamGraphic = map.createEntity();

    auto* body = beamGraphic.addComponent<ComponentBody>();

    beamEndPointGraphic = map.createEntity();
    auto& modelEndPt = *beamEndPointGraphic.addComponent<ComponentModel>(Mesh::Plane, (Material*)ResourceManifest::TorpedoGlowMaterial.get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& beamModelEnd = modelEndPt.getModel(0);
    beamModelEnd.hide();

    auto& body1 = *beamEndPointGraphic.addComponent<ComponentBody>(CollisionType::Sphere);

    btMultiSphereShape& sph = *static_cast<btMultiSphereShape*>(body1.getCollision()->getBtShape());
    const auto& _scl = btVector3(0.01f, 0.01f, 0.01f);
    sph.setLocalScaling(_scl);
    sph.setMargin(0.01f);
    sph.setImplicitShapeDimensions(_scl);
    sph.recalcLocalAabb();
    beamModelEnd.setScale(10.7f * additionalBeamSizeScale);

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
const bool PrimaryWeaponBeam::canFire() {
    if (numRounds > 0) {
        return true;
    }
    return false;
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
SecondaryWeaponTorpedo::SecondaryWeaponTorpedo(Map& map, WeaponType::Type _type, Ship& _ship,const glm_vec3& _pos,const glm_vec3& _fwd,const float& _arc,const uint& _maxCharges,const float& _dmg,const float& _rechargePerRound,const float& _impactRad,const float& _impactTime,const float& _travelSpeed,const float& _volume,const float& _rotAngleSpeed, const unsigned int& _modelIndex) : ShipWeapon(map, _type, _ship,_pos,_fwd,_arc, _dmg, _impactRad, _impactTime, _volume, _maxCharges, _rechargePerRound, _modelIndex) {
    damage                   = _dmg;
    impactRadius             = _impactRad;
    impactTime               = _impactTime;
    travelSpeed              = _travelSpeed;
    volume                   = _volume;
    rotationAngleSpeed       = _rotAngleSpeed;
}

#pragma endregion

#pragma region Torpedo
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
const int SecondaryWeaponTorpedo::canFire() {
    if (numRounds > 0) {
        return m_Map.try_addTorpedoProjectile();
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
   SAFE_DELETE_VECTOR(m_PrimaryWeaponsCannons);
   SAFE_DELETE_VECTOR(m_PrimaryWeaponsBeams);
   SAFE_DELETE_VECTOR(m_SecondaryWeaponsTorpedos);
}
void ShipSystemWeapons::update(const double& dt) {
    const bool isCloaked = m_Ship.isCloaked();
    const bool isWarping = m_Ship.IsWarping();
    const bool isPlayer = m_Ship.IsPlayer();

    auto* mytarget = m_Ship.getTarget();
    Ship* ship = nullptr;
    if (mytarget) {
        ship = dynamic_cast<Ship*>(mytarget);
        if (ship) {
            cannonTargetPoint  = ship->getAimPositionRandomLocal();
            torpedoTargetPoint = ship->getAimPositionRandomLocal();
        }else{
            cannonTargetPoint  = glm::vec3(0.0f);
            torpedoTargetPoint = glm::vec3(0.0f);
        }
    }
    if (isPlayer && Engine::isMouseButtonDownOnce(MouseButton::Left) && !isCloaked && !isWarping) {
        #pragma region Beams
        vector<std::tuple<uint, EntityWrapper*>> primaryWeaponsBeamsFired;
        for (uint i = 0; i < m_PrimaryWeaponsBeams.size(); ++i) {
            const auto res = m_PrimaryWeaponsBeams[i]->canFire();
            if (res) {
                if (mytarget) {
                    primaryWeaponsBeamsFired.push_back(std::make_tuple(i, mytarget));
                }
            }
        }

        #pragma endregion
        #pragma region Cannons
        vector<std::tuple<uint, int, EntityWrapper*>> primaryWeaponsCannonsFired;
        if (mytarget) {
            for (uint i = 0; i < m_PrimaryWeaponsCannons.size(); ++i) {
                auto* cannon = m_PrimaryWeaponsCannons[i];
                if (cannon->isInControlledArc(mytarget)) {
                    const int resIndex = cannon->canFire();
                    if (resIndex >= 0) {
                        primaryWeaponsCannonsFired.push_back(std::make_tuple(i, resIndex, mytarget));
                    }
                }
            }
        }else{
            for (uint i = 0; i < m_PrimaryWeaponsCannonsFwd.size(); ++i) {
                auto& cannon = m_PrimaryWeaponsCannonsFwd[i];
                const int resIndex = cannon.cannon->canFire();
                if (resIndex >= 0) {
                    primaryWeaponsCannonsFired.push_back(std::make_tuple(cannon.main_container_index, resIndex, mytarget));
                }
            }
        }
        #pragma endregion

        //send packet with indices
        if (primaryWeaponsBeamsFired.size() > 0) {
            PacketMessage pOut;
            pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Beams;
            pOut.name = m_Ship.getName();
            pOut.data = to_string(std::get<0>(primaryWeaponsBeamsFired[0]));

            glm::vec3 target_point;

            if (ship) {
                target_point = ship->getAimPositionRandomLocal();
            }else{
                target_point = glm::vec3(0.0f);
            }
            pOut.r = target_point.r;
            pOut.g = target_point.g;
            pOut.b = target_point.b;

            auto* target_ptr = std::get<1>(primaryWeaponsBeamsFired[0]);
            string target_name = "N/A";
            if (target_ptr) {
                if (ship) {
                    target_name = ship->getMapKey();
                }else {
                    target_name = target_ptr->getComponent<ComponentName>()->name();
                }
            }
            pOut.data += "," + target_name;
            for (uint i = 1; i < primaryWeaponsBeamsFired.size(); ++i) {
                pOut.data += "," + to_string(std::get<0>(primaryWeaponsBeamsFired[i]));

                target_ptr = std::get<1>(primaryWeaponsBeamsFired[0]);
                target_name = "N/A";
                if (target_ptr) {
                    if (ship) {
                        target_name = ship->getMapKey();
                    }else{
                        target_name = target_ptr->getComponent<ComponentName>()->name();
                    }
                }
                pOut.data += "," + target_name;
            }
            m_Ship.m_Client.send(pOut);
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
                if (ship) {
                    target_name = ship->getMapKey();
                }else{
                    target_name = target_ptr->getComponent<ComponentName>()->name();
                }
            }
            pOut.data += "," + target_name;

            for (uint i = 1; i < primaryWeaponsCannonsFired.size(); ++i) {
                pOut.data += "," + to_string(std::get<0>(primaryWeaponsCannonsFired[i])) + "," + to_string(std::get<1>(primaryWeaponsCannonsFired[i]));

                target_ptr = std::get<2>(primaryWeaponsCannonsFired[i]);
                target_name = "N/A";
                if (target_ptr) {
                    if (ship) {
                        target_name = ship->getMapKey();
                    }else{
                        target_name = target_ptr->getComponent<ComponentName>()->name();
                    }
                }
                pOut.data += "," + target_name;
            }
            m_Ship.m_Client.send(pOut);
        }
    }
    if (isPlayer && Engine::isMouseButtonDownOnce(MouseButton::Right) && !isCloaked && !isWarping) {
        #pragma region Torpedos
        vector<std::tuple<uint, int, EntityWrapper*>> secWeaponsTorpedosFired;

        //first, get the arcs that are in range of the target if applicable
        vector<std::tuple<uint, int, EntityWrapper*>> secWeaponsTorpedosFiredInValidArc;
        if (mytarget) {
            for (uint i = 0; i < m_SecondaryWeaponsTorpedos.size(); ++i) {
                auto torpedo = m_SecondaryWeaponsTorpedos[i];
                if (torpedo->isInControlledArc(mytarget)) {
                    const int resIndex = torpedo->canFire();
                    if (resIndex >= 0) {
                        secWeaponsTorpedosFiredInValidArc.push_back(std::make_tuple(i, resIndex, mytarget));
                    }
                }
            }
        }else{
            for (uint i = 0; i < m_SecondaryWeaponsTorpedos.size(); ++i) {
                auto torpedo = m_SecondaryWeaponsTorpedos[i];
                const int resIndex = torpedo->canFire();
                if (resIndex >= 0) {
                    secWeaponsTorpedosFiredInValidArc.push_back(std::make_tuple(i, resIndex, mytarget));
                }
            }
        }

        //now we have our valid torpedo launchers, pick the launcher with the most rounds
        uint maxRounds = 0;
        uint chosenIndex = 0;
        for (auto& weapIndex : secWeaponsTorpedosFiredInValidArc) {
            auto& launcher = m_SecondaryWeaponsTorpedos[std::get<0>(weapIndex)];
            if (launcher->numRounds > maxRounds) {
                maxRounds = launcher->numRounds;
                chosenIndex = std::get<0>(weapIndex);
            }
        }
        //now fire this chosen launcher
        if(secWeaponsTorpedosFiredInValidArc.size() > 0){
            const int resIndex = m_SecondaryWeaponsTorpedos[chosenIndex]->canFire();
            if (resIndex >= 0) {
                secWeaponsTorpedosFired.push_back(std::make_tuple(chosenIndex, resIndex, mytarget));
            }
        }
        #pragma endregion

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
                if (ship) {
                    target_name = ship->getMapKey();
                }else{
                    target_name = target_ptr->getComponent<ComponentName>()->name();
                }
            }
            pOut.data += "," + target_name;
            for (uint i = 1; i < secWeaponsTorpedosFired.size(); ++i) {
                pOut.data += "," + to_string(std::get<0>(secWeaponsTorpedosFired[i])) + "," + to_string(std::get<1>(secWeaponsTorpedosFired[i]));
                target_ptr = std::get<2>(secWeaponsTorpedosFired[i]);
                target_name = "N/A";
                if (target_ptr) {
                    if (ship) {
                        target_name = ship->getMapKey();
                    }else{
                        target_name = target_ptr->getComponent<ComponentName>()->name();
                    }
                }
                pOut.data += "," + target_name;
            }
            m_Ship.m_Client.send(pOut);
        }
    }
    for (auto& beam : m_PrimaryWeaponsBeams) { beam->update(dt); }
    for (auto& cannon : m_PrimaryWeaponsCannons) { cannon->update(dt); }
    for (auto& torpedo : m_SecondaryWeaponsTorpedos) { torpedo->update(dt); }
}
void ShipSystemWeapons::addPrimaryWeaponBeam(PrimaryWeaponBeam& beam, const bool isForwardWeapon) {
    beam.index = m_PrimaryWeaponsBeams.size();
    m_PrimaryWeaponsBeams.push_back(&beam);

    WeaponBeam b;
    b.main_container_index = beam.index;
    b.beam = &beam;
    if (isForwardWeapon) {
        m_PrimaryWeaponsBeamsFwd.push_back(std::move(b));
    }else{
        m_PrimaryWeaponsBeamsNonFwd.push_back(std::move(b));
    }

}
void ShipSystemWeapons::addPrimaryWeaponCannon(PrimaryWeaponCannon& cannon, const bool isForwardWeapon) {
    cannon.index = m_PrimaryWeaponsCannons.size();
    m_PrimaryWeaponsCannons.push_back(&cannon);

    WeaponCannon c;
    c.main_container_index = cannon.index;
    c.cannon = &cannon;
    if (isForwardWeapon) {
        m_PrimaryWeaponsCannonsFwd.push_back(std::move(c));
    }else{
        m_PrimaryWeaponsCannonsNonFwd.push_back(std::move(c));
    }

}
void ShipSystemWeapons::addSecondaryWeaponTorpedo(SecondaryWeaponTorpedo& torpedo, const bool isForwardWeapon) {
    torpedo.index = m_SecondaryWeaponsTorpedos.size();
    m_SecondaryWeaponsTorpedos.push_back(&torpedo);

    WeaponTorpedo t;
    t.main_container_index = torpedo.index;
    t.torpedo = &torpedo;
    if (isForwardWeapon) {
        m_SecondaryWeaponsTorpedosFwd.push_back(std::move(t));
    }else{
        m_SecondaryWeaponsTorpedosNonFwd.push_back(std::move(t));
    }
}
PrimaryWeaponBeam& ShipSystemWeapons::getPrimaryWeaponBeam(const uint index) {
    return *m_PrimaryWeaponsBeams[index];
}
PrimaryWeaponCannon& ShipSystemWeapons::getPrimaryWeaponCannon(const uint index) {
    return *m_PrimaryWeaponsCannons[index];
}
SecondaryWeaponTorpedo& ShipSystemWeapons::getSecondaryWeaponTorpedo(const uint index) {
    return *m_SecondaryWeaponsTorpedos[index];
}
vector<ShipSystemWeapons::WeaponCannon>& ShipSystemWeapons::getForwardCannons() {
    return m_PrimaryWeaponsCannonsFwd;
}
vector<ShipSystemWeapons::WeaponBeam>& ShipSystemWeapons::getForwardBeams() {
    return m_PrimaryWeaponsBeamsFwd;
}
vector<ShipSystemWeapons::WeaponTorpedo>& ShipSystemWeapons::getForwardTorpedos() {
    return m_SecondaryWeaponsTorpedosFwd;
}

vector<ShipSystemWeapons::WeaponCannon>& ShipSystemWeapons::getNonForwardCannons() {
    return m_PrimaryWeaponsCannonsNonFwd;
}
vector<ShipSystemWeapons::WeaponBeam>& ShipSystemWeapons::getNonForwardBeams() {
    return m_PrimaryWeaponsBeamsNonFwd;
}
vector<ShipSystemWeapons::WeaponTorpedo>& ShipSystemWeapons::getNonForwardTorpedos() {
    return m_SecondaryWeaponsTorpedosNonFwd;
}
vector<PrimaryWeaponCannon*>& ShipSystemWeapons::getCannons() {
    return m_PrimaryWeaponsCannons;
}
vector<PrimaryWeaponBeam*>& ShipSystemWeapons::getBeams() {
    return m_PrimaryWeaponsBeams;
}
vector<SecondaryWeaponTorpedo*>& ShipSystemWeapons::getTorpedos() {
    return m_SecondaryWeaponsTorpedos;
}
