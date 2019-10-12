#include "ShipSystemWeapons.h"
#include "../../weapons/Weapons.h"
#include "../../Ship.h"
#include "../../ResourceManifest.h"
#include "../../Packet.h"
#include "../../map/Map.h"
#include <core/engine/lights/Lights.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/math/Engine_Math.h>

#include <core/engine/events/Engine_Events.h>
#include <core/engine/math/Engine_Math.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>

using namespace Engine;
using namespace std;

#define PREDICTED_PHYSICS_CONSTANT 0.11111111f

float ShipSystemWeapons::calculate_quadratic_time_till_hit(const glm::vec3& pos, const glm::vec3& vel, const float& s) {
    float a = s * s - glm::dot(vel, vel);
    float b = glm::dot(pos, vel);
    float c = glm::dot(pos, pos);
    float d = b * b + a * c;
    float t = 0;
    if (d >= 0) {
        t = (b + sqrt(d)) / a;
        if (t < 0)
            t = 0;
    }
    return t;
}

ShipWeapon::ShipWeapon(Map& map, WeaponType::Type _type, Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc, const float& _dmg, const float& _impactRad, const float& _impactTime, const float& _volume, const uint& _numRounds, const float& _rechargeTimerPerRound):ship(_ship),m_Map(map){
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
}
const bool ShipWeapon::isInArc(EntityWrapper* target, const float _arc) {
    if (!target)
        return false;
    auto& shipBody = *ship.getComponent<ComponentBody>();
    auto shipPosition = shipBody.position();
    auto shipRotation = shipBody.rotation();

    auto& targetBody = *target->getComponent<ComponentBody>();
    auto* targetIsShip = dynamic_cast<Ship*>(target);
    glm::vec3 targetPosition;
    if (targetIsShip) {
        targetPosition = targetIsShip->getAimPositionDefault();
    }else{
        targetPosition = targetBody.position();
    }
    const auto cannonForward = glm::normalize(shipRotation * forward);

    const auto vecToTarget = shipPosition - targetPosition;
    const auto vecToForward = shipPosition - (cannonForward * 100000000.0f);
    const auto angleToTarget = Math::getAngleBetweenTwoVectors(glm::normalize(vecToTarget), glm::normalize(vecToForward), true);
    if (angleToTarget <= _arc) {
        return true;
    }
    return false;
}


PrimaryWeaponCannonProjectile::PrimaryWeaponCannonProjectile(Map& _map, const glm::vec3& position, const glm::vec3& forward, const int index):map(_map) {
    entity       = _map.createEntity();
    currentTime  = 0.0f;
    maxTime      = 2.5f;
    active       = false;
    destroyed    = false;
    projectile_index = index;
}
PrimaryWeaponCannonProjectile::~PrimaryWeaponCannonProjectile() {

}
void PrimaryWeaponCannonProjectile::update(const double& dt) {
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
void PrimaryWeaponCannonProjectile::destroy() {
    if (!destroyed) {
        entity.destroy();
        if (light) {
            light->destroy();
            SAFE_DELETE(light);
        }
        map.removeCannonProjectile(projectile_index);
        active = false;
        destroyed = true;
    }
}
void PrimaryWeaponCannonProjectile::clientToServerImpact(Client& client, Ship& shipHit, const glm::vec3& impactLocalPosition, const glm::vec3& impactNormal, const float& impactRadius, const float& damage, const float& time, const bool& shields) {
    PacketProjectileImpact packet;
    packet.PacketType = PacketType::Client_To_Server_Projectile_Cannon_Impact;
    packet.damage     = damage;
    packet.impactX    = impactLocalPosition.x;
    packet.impactY    = impactLocalPosition.y;
    packet.impactZ    = impactLocalPosition.z;
    packet.shields    = shields;
    packet.data       = shipHit.getName() + ",";
    packet.index      = projectile_index;
    Math::Float16From32(&packet.time, time);
    Math::Float16From32(&packet.radius, impactRadius);
    Math::Float16From32(&packet.normalX, impactNormal.x);
    Math::Float16From32(&packet.normalY, impactNormal.y);
    Math::Float16From32(&packet.normalZ, impactNormal.z);
    active = false;
    client.send(packet);
}


PrimaryWeaponCannon::PrimaryWeaponCannon(Map& map, WeaponType::Type _type, Ship& _ship,const glm::vec3& _pos,const glm::vec3& _fwd,const float& _arc,const uint& _maxCharges,const float& _dmg,const float& _rechargePerRound,const float& _impactRad,const float& _impactTime, const float& _travelSpeed, const float& _volume) : ShipWeapon(map, _type, _ship, _pos, _fwd, _arc, _dmg, _impactRad, _impactTime, _volume, _maxCharges, _rechargePerRound){
    damage                   = _dmg;
    impactRadius             = _impactRad;
    impactTime               = _impactTime;
    travelSpeed              = _travelSpeed;
    volume                   = _volume;
}
const PrimaryWeaponCannonPrediction PrimaryWeaponCannon::calculatePredictedVector(ComponentBody& projectileBody, const glm::vec3& chosen_target_pos) {
    auto& shipBody = *ship.getComponent<ComponentBody>();
    auto shipRotation = shipBody.rotation();
    auto cannonForward = glm::normalize(shipRotation * forward);
    auto shipPosition = shipBody.position();
    auto mytarget = ship.getTarget();

    auto returnValue = PrimaryWeaponCannonPrediction();

    auto defaultVelocityVector = cannonForward * travelSpeed;
    auto currentVelocityVector = projectileBody.getLinearVelocity();
    auto finalSpeed = glm::length(defaultVelocityVector + currentVelocityVector);
    returnValue.finalProjectileSpeed = finalSpeed;
    if (mytarget) {
        auto& targetBody           = *mytarget->getComponent<ComponentBody>();
        auto* targetIsShip         = dynamic_cast<Ship*>(mytarget);
        const auto vecToTarget     = shipPosition - chosen_target_pos;
        const auto vecToForward    = shipPosition - (cannonForward * 100000000.0f);
        const auto angleToTarget   = Math::getAngleBetweenTwoVectors(glm::normalize(vecToTarget), glm::normalize(vecToForward), true);
        if (angleToTarget <= arc) {
            const auto launcherPosition = shipPosition + (shipRotation * position);
            const auto targetLinearVelocity = targetBody.getLinearVelocity();
            const auto distanceToTarget = glm::distance(chosen_target_pos, launcherPosition);

            const auto finalTravelTime = distanceToTarget / finalSpeed;

            auto myShipVelocity = shipBody.getLinearVelocity();
            auto combinedVelocity = targetLinearVelocity - myShipVelocity;
            const auto predictedSpeed = combinedVelocity + (cannonForward * finalTravelTime) /* * target.acceleration */; //TODO: figure this out later
            const auto averageSpeed = (combinedVelocity + predictedSpeed) * 0.5f;
            auto predictedPos = chosen_target_pos + (averageSpeed * finalTravelTime);

            returnValue.pedictedVector = -glm::normalize(launcherPosition - predictedPos);
            returnValue.pedictedPosition = predictedPos;
            return returnValue;
        }
    }
    returnValue.pedictedVector = cannonForward;
    returnValue.pedictedPosition = cannonForward;
    return returnValue;
}
const int PrimaryWeaponCannon::canFire() {
    if (numRounds > 0) {
        return m_Map.try_addCannonProjectile();
    }
    return -1;
}
const bool PrimaryWeaponCannon::forceFire(const int index, const glm::vec3& chosen_target_pos) {
    if (numRounds > 0) {
        const bool can = m_Map.try_addCannonProjectile(index);
        if (can) {
            Weapons::spawnProjectile(*this, ship, m_Map, position, forward, index, chosen_target_pos);
            return true;
        }
    }
    return false;
}
void PrimaryWeaponCannon::update(const double& dt) {
    if (soundEffect && (soundEffect->status() == SoundStatus::Playing || soundEffect->status() == SoundStatus::PlayingLooped)) {
        const auto shipRotation = ship.getRotation();
        const auto shipPosition = ship.getPosition();
        const auto launcherPosition = shipPosition + (shipRotation * position);
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
PrimaryWeaponBeam::PrimaryWeaponBeam(WeaponType::Type _type, Ship& _ship, Map& map, const glm::vec3& _pos, const glm::vec3& _fwd, const float& _arc, const float& _dmg, const float& _impactRad, const float& _impactTime, const float& _volume, vector<glm::vec3>& _windupPts,const uint& _maxCharges,const float& _rechargeTimePerRound, const float& _chargeTimerSpeed, const float& _firingTime) : ShipWeapon(map, _type, _ship, _pos, _fwd, _arc, _dmg, _impactRad, _impactTime, _volume, _maxCharges, _rechargeTimePerRound) {
    windupPoints = _windupPts;
    targetCoordinates = glm::vec3(0.0f);
    chargeTimer = 0.0f;
    chargeTimerSpeed = _chargeTimerSpeed;
    state = BeamWeaponState::Off;
    firingTimeMax = _firingTime;
    firingTime = 0.0f;
    firingTimeShieldGraphicPing = 0.0f;

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
    beamModelEnd.setScale(10.7f);

    body1.addCollisionFlag(CollisionFlag::NoContactResponse);
    body1.setCollisionGroup(CollisionFilter::_Custom_2); //i belong to weapons (group 2)
    body1.setCollisionMask(CollisionFilter::_Custom_1 | CollisionFilter::_Custom_3); //i should only collide with shields and hull (group 1 and group 3)
    body1.setInternalPhysicsUserPointer(&body1);
    const_cast<btRigidBody&>(body1.getBtBody()).setDamping(0.0f, 0.0f);
}
PrimaryWeaponBeam::~PrimaryWeaponBeam() {
    beamGraphic.destroy();
    beamEndPointGraphic.destroy();
    if (beamLight) {
        beamLight->destroy();
        SAFE_DELETE(beamLight);
    }
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
const glm::vec3 PrimaryWeaponBeam::calculatePredictedVector() {
    glm::vec3 ret = glm::vec3(0.0f);
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
const bool PrimaryWeaponBeam::fire(const double& dt) {
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
SecondaryWeaponTorpedo::SecondaryWeaponTorpedo(Map& map, WeaponType::Type _type, Ship& _ship,const glm::vec3& _pos,const glm::vec3& _fwd,const float& _arc,const uint& _maxCharges,const float& _dmg,const float& _rechargePerRound,const float& _impactRad,const float& _impactTime,const float& _travelSpeed,const float& _volume,const float& _rotAngleSpeed) : ShipWeapon(map, _type, _ship,_pos,_fwd,_arc, _dmg, _impactRad, _impactTime, _volume, _maxCharges, _rechargePerRound) {
    damage                   = _dmg;
    impactRadius             = _impactRad;
    impactTime               = _impactTime;
    travelSpeed              = _travelSpeed;
    volume                   = _volume;
    rotationAngleSpeed       = _rotAngleSpeed;
}

const SecondaryWeaponTorpedoPrediction SecondaryWeaponTorpedo::calculatePredictedVector(ComponentBody& projectileBody, const glm::vec3& chosen_target_pos) {
    auto& shipBody = *ship.getComponent<ComponentBody>();
    auto shipRotation = shipBody.rotation();
    auto torpForward = glm::normalize(shipRotation * forward);
    auto shipPosition = shipBody.position();
    auto mytarget = ship.getTarget();

    auto returnValue = SecondaryWeaponTorpedoPrediction();
    auto defaultVelocityVector = torpForward * travelSpeed;
    auto currentVelocityVector = projectileBody.getLinearVelocity();
    auto finalSpeed = glm::length(defaultVelocityVector + currentVelocityVector);
    returnValue.finalProjectileSpeed = finalSpeed;
    if (mytarget) {
        auto& targetBody = *mytarget->getComponent<ComponentBody>();
        auto* targetIsShip = dynamic_cast<Ship*>(mytarget);

        const auto vecToTarget = shipPosition - chosen_target_pos;
        const auto vecToForward = shipPosition - (torpForward * 100000000.0f);
        const auto angleToTarget = Math::getAngleBetweenTwoVectors(glm::normalize(vecToTarget), glm::normalize(vecToForward), true);
        if (angleToTarget <= arc) {
            returnValue.hasLock = true;
            returnValue.target = mytarget;

            const auto launcherPosition = shipPosition + (shipRotation * position);
            const auto targetLinearVelocity = targetBody.getLinearVelocity();
            const auto distanceToTarget = glm::distance(chosen_target_pos, launcherPosition);


            const auto finalTravelTime = distanceToTarget / finalSpeed;
            auto myShipVelocity = shipBody.getLinearVelocity();

            auto combinedVelocity = targetLinearVelocity - myShipVelocity;
            const auto predictedSpeed = combinedVelocity + (torpForward * finalTravelTime) /* * target.acceleration */; //TODO: figure this out later
            const auto averageSpeed = (combinedVelocity + predictedSpeed) * 0.5f;
            auto predictedPos = chosen_target_pos + (averageSpeed * finalTravelTime);

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
    return isInArc(target, 45.0f);
}
const int SecondaryWeaponTorpedo::canFire() {
    if (numRounds > 0) {
        return m_Map.try_addTorpedoProjectile();
    }
    return -1;
}
const bool SecondaryWeaponTorpedo::forceFire(const int index, const glm::vec3& chosen_target_pos) {
    if (numRounds > 0) {
        const bool can = m_Map.try_addTorpedoProjectile(index);
        if (can) {
            Weapons::spawnProjectile(*this, ship, m_Map, position, forward, index, chosen_target_pos);
            return true;
        }
    }
    return false;
}
void SecondaryWeaponTorpedo::update(const double& dt) {
    if (soundEffect && (soundEffect->status() == SoundStatus::Playing || soundEffect->status() == SoundStatus::PlayingLooped)) {
        const auto shipRotation = ship.getRotation();
        const auto shipPosition = ship.getPosition();
        const auto launcherPosition = shipPosition + (shipRotation * position);
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
SecondaryWeaponTorpedoProjectile::SecondaryWeaponTorpedoProjectile(Map& _map, const glm::vec3& position, const glm::vec3& forward, const int index) :map(_map) {
    entity             = _map.createEntity();
    currentTime        = 0.0f;
    maxTime            = 30.5f;
    hasLock            = false;
    target             = nullptr;
    light              = nullptr;
    rotationAngleSpeed = 0.0f;
    active             = true;
    destroyed          = false;
    projectile_index   = index;
}
SecondaryWeaponTorpedoProjectile::~SecondaryWeaponTorpedoProjectile() {
    active = false;
}
void SecondaryWeaponTorpedoProjectile::clientToServerImpact(Client& client, Ship& shipHit, const glm::vec3& impactLocalPosition, const glm::vec3& impactNormal, const float& impactRadius, const float& damage, const float& time, const bool& shields) {
    PacketProjectileImpact packet;
    packet.PacketType = PacketType::Client_To_Server_Projectile_Torpedo_Impact;
    packet.damage     = damage;
    packet.impactX    = impactLocalPosition.x;
    packet.impactY    = impactLocalPosition.y;
    packet.impactZ    = impactLocalPosition.z;
    packet.shields    = shields;
    packet.index      = projectile_index;
    packet.data = shipHit.getName() + ",";
    Math::Float16From32(&packet.time, time);
    Math::Float16From32(&packet.radius, impactRadius);
    Math::Float16From32(&packet.normalX, impactNormal.x);
    Math::Float16From32(&packet.normalY, impactNormal.y);
    Math::Float16From32(&packet.normalZ, impactNormal.z);
    active = false;
    client.send(packet);
}
void SecondaryWeaponTorpedoProjectile::update(const double& dt) {
    if (active) {
        const float fdt = static_cast<float>(dt);
        currentTime += fdt;

        auto& glowModel = *entity.getComponent<ComponentModel>();
        auto& body = *entity.getComponent<ComponentBody>();
        const auto glowBodyPos = body.position_render();
        auto* activeCam = glowModel.getOwner().scene().getActiveCamera();
        const auto camPos = activeCam->getPosition();

        //TODO: hacky workaround for messed up camera forward vector
        auto vec = glm::normalize(glowBodyPos - camPos);
        vec *= 0.01f;
        body.setRotation(activeCam->getOrientation());
        glowModel.getModel(1).setPosition(vec);

        if (light) {
            auto& lightBody = *light->getComponent<ComponentBody>();
            lightBody.setPosition(glowBodyPos);
        }

        if (currentTime >= maxTime) {
            destroy();
        }
    }
}
void SecondaryWeaponTorpedoProjectile::destroy() {
    if (!destroyed) {
        entity.destroy();
        if (light) {
            light->destroy();
            SAFE_DELETE(light);
        }
        map.removeTorpedoProjectile(projectile_index);
        active = false;
        destroyed = true;
    }
}





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
    if (mytarget) {
        Ship* ship = dynamic_cast<Ship*>(mytarget);
        if (ship) {
            cannonTargetPoint = ship->getAimPositionRandom();
            torpedoTargetPoint = ship->getAimPositionRandom();
        }else{
            auto& targetBody = *mytarget->getComponent<ComponentBody>();
            const auto pos = targetBody.position();
            cannonTargetPoint = pos;
            torpedoTargetPoint = pos;
        }
    }


    if (isPlayer && Engine::isMouseButtonDownOnce(MouseButton::Left) && !isCloaked && !isWarping) {
        #pragma region primary weapons
        vector<std::pair<uint, int>> primaryWeaponsBeamsFired;
        vector<std::pair<uint, int>> primaryWeaponsCannonsFired;
        for (uint i = 0; i < m_PrimaryWeaponsBeams.size(); ++i) {
            const auto res = m_PrimaryWeaponsBeams[i]->canFire();
            if (res) {
                primaryWeaponsBeamsFired.push_back(std::make_pair(i, res));
            }
        }
        for (uint i = 0; i < m_PrimaryWeaponsCannons.size(); ++i) {
            const int resIndex = m_PrimaryWeaponsCannons[i]->canFire();
            if (resIndex >= 0) {
                primaryWeaponsCannonsFired.push_back(std::make_pair(i, resIndex));
            }
        }
        //send packet with indices
        if (primaryWeaponsBeamsFired.size() > 0) {
            PacketMessage pOut;
            pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Beams;
            pOut.name = m_Ship.getName();
            //TODO: sync this
            //pOut.r = cannonTargetPoint.x;
            //pOut.g = cannonTargetPoint.y;
            //pOut.b = cannonTargetPoint.z;
            pOut.data = to_string(primaryWeaponsBeamsFired[0].first) + "," + to_string(primaryWeaponsBeamsFired[0].second);
            for (uint i = 1; i < primaryWeaponsBeamsFired.size(); ++i)
                pOut.data += "," + to_string(primaryWeaponsBeamsFired[i].first) + "," + to_string(primaryWeaponsBeamsFired[i].second);
            m_Ship.m_Client.send(pOut);
        }
        if (primaryWeaponsCannonsFired.size() > 0) {
            PacketMessage pOut;
            pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Cannons;
            pOut.name = m_Ship.getName();
            pOut.r = cannonTargetPoint.x;
            pOut.g = cannonTargetPoint.y;
            pOut.b = cannonTargetPoint.z;
            pOut.data = to_string(primaryWeaponsCannonsFired[0].first) + "," + to_string(primaryWeaponsCannonsFired[0].second);
            for (uint i = 1; i < primaryWeaponsCannonsFired.size(); ++i)
                pOut.data += "," + to_string(primaryWeaponsCannonsFired[i].first) + "," + to_string(primaryWeaponsCannonsFired[i].second);
            m_Ship.m_Client.send(pOut);
        }
        #pragma endregion
    }
    if (isPlayer && Engine::isMouseButtonDownOnce(MouseButton::Right) && !isCloaked && !isWarping) {
        #pragma region secondary weapons
        vector<std::pair<uint, int>> secWeaponsTorpedosFired;

        //first, get the arcs that are in range of the target if applicable
        vector<std::pair<uint, int>> secWeaponsTorpedosFiredInValidArc;
        auto* mytarget = m_Ship.getTarget();
        if (mytarget) {
            for (uint i = 0; i < m_SecondaryWeaponsTorpedos.size(); ++i) {
                auto torpedo = m_SecondaryWeaponsTorpedos[i];
                if (torpedo->isInControlledArc(mytarget)) {
                    const int resIndex = torpedo->canFire();
                    if (resIndex >= 0) {
                        secWeaponsTorpedosFiredInValidArc.push_back(std::make_pair(i, resIndex));
                    }
                }
            }
        }else{
            for (uint i = 0; i < m_SecondaryWeaponsTorpedos.size(); ++i) {
                auto torpedo = m_SecondaryWeaponsTorpedos[i];
                const int resIndex = torpedo->canFire();
                if (resIndex >= 0) {
                    secWeaponsTorpedosFiredInValidArc.push_back(std::make_pair(i, resIndex));
                }
            }
        }

        //now we have our valid torpedo launchers, pick the launcher with the most rounds
        uint maxRounds = 0;
        uint chosenIndex = 0;
        for (auto& weapIndex : secWeaponsTorpedosFiredInValidArc) {
            auto& launcher = m_SecondaryWeaponsTorpedos[weapIndex.first];
            if (launcher->numRounds > maxRounds) {
                maxRounds = launcher->numRounds;
                chosenIndex = weapIndex.first;
            }
        }
        //now fire this chosen launcher
        if(secWeaponsTorpedosFiredInValidArc.size() > 0){
            const int resIndex = m_SecondaryWeaponsTorpedos[chosenIndex]->canFire();
            if (resIndex >= 0) {
                secWeaponsTorpedosFired.push_back(std::make_pair(chosenIndex, resIndex));
            }
        }




        //send packet with indices
        if (secWeaponsTorpedosFired.size() > 0) {
            PacketMessage pOut;
            pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Torpedos;
            pOut.name = m_Ship.getName();
            pOut.r = torpedoTargetPoint.x;
            pOut.g = torpedoTargetPoint.y;
            pOut.b = torpedoTargetPoint.z;
            pOut.data = to_string(secWeaponsTorpedosFired[0].first) + "," + to_string(secWeaponsTorpedosFired[0].second);
            for (uint i = 1; i < secWeaponsTorpedosFired.size(); ++i)
                pOut.data += "," + to_string(secWeaponsTorpedosFired[i].first) + "," + to_string(secWeaponsTorpedosFired[i].second);
            m_Ship.m_Client.send(pOut);
        }
        #pragma endregion
    }
    for (auto& beam : m_PrimaryWeaponsBeams) {
        beam->update(dt);
    }
    for (auto& cannon : m_PrimaryWeaponsCannons) {
        cannon->update(dt);
    }
    for (auto& torpedo : m_SecondaryWeaponsTorpedos) {
        torpedo->update(dt);
    }
}
void ShipSystemWeapons::addPrimaryWeaponBeam(PrimaryWeaponBeam& beam) {
    beam.index = m_PrimaryWeaponsBeams.size();
    m_PrimaryWeaponsBeams.push_back(&beam);
}
void ShipSystemWeapons::addPrimaryWeaponCannon(PrimaryWeaponCannon& cannon) {
    cannon.index = m_PrimaryWeaponsCannons.size();
    m_PrimaryWeaponsCannons.push_back(&cannon);
}
void ShipSystemWeapons::addSecondaryWeaponTorpedo(SecondaryWeaponTorpedo& torpedo) {
    torpedo.index = m_SecondaryWeaponsTorpedos.size();
    m_SecondaryWeaponsTorpedos.push_back(&torpedo);
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