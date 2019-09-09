#include "ShipSystemWeapons.h"
#include "../../Ship.h"
#include "../../ResourceManifest.h"
#include "../../Packet.h"
#include "../../map/Map.h"
#include <core/engine/lights/Lights.h>
#include <core/engine/mesh/Mesh.h>

#include <core/engine/events/Engine_Events.h>
#include <core/engine/math/Engine_Math.h>

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

ShipWeapon::ShipWeapon(Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc, const uint& _dmg, const float& _impactRad, const float& _impactTime, const float& _volume, const uint& _numRounds, const float& _rechargeTimerPerRound):ship(_ship) {
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
    auto cannonForward = glm::normalize(shipRotation * forward);

    const auto vecToTarget = shipPosition - targetPosition;
    const auto vecToForward = shipPosition - (cannonForward * 100000000.0f);
    const auto angleToTarget = Math::getAngleBetweenTwoVectors(glm::normalize(vecToTarget), glm::normalize(vecToForward), true);
    if (angleToTarget <= _arc) {
        return true;
    }
    return false;
}


PrimaryWeaponCannonProjectile::PrimaryWeaponCannonProjectile(Map& map, const glm::vec3& position, const glm::vec3& forward) {
    entity       = map.createEntity();
    currentTime  = 0.0f;
    maxTime      = 2.5f;
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
    if (active) {
        active = false;
        entity.destroy();
        if (light) {
            light->destroy();
            SAFE_DELETE(light);
        }
    }
}


PrimaryWeaponCannon::PrimaryWeaponCannon(Ship& _ship,const glm::vec3& _pos,const glm::vec3& _fwd,const float& _arc,const uint& _maxCharges,const uint& _dmg,const float& _rechargePerRound,const float& _impactRad,const float& _impactTime, const float& _travelSpeed, const float& _volume) : ShipWeapon(_ship, _pos, _fwd, _arc, _dmg, _impactRad, _impactTime, _volume, _maxCharges, _rechargePerRound) {
    damage                   = _dmg;
    impactRadius             = _impactRad;
    impactTime               = _impactTime;
    travelSpeed              = _travelSpeed;
    volume                   = _volume;
}
const PrimaryWeaponCannonPrediction PrimaryWeaponCannon::calculatePredictedVector(ComponentBody& projectileBody) {
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
        auto* targetIsShip = dynamic_cast<Ship*>(mytarget);
        glm::vec3 targetPosition;
        if (targetIsShip) {
            targetPosition = targetIsShip->getAimPositionDefault();
        }else{
            targetPosition = targetBody.position();
        }
        const auto vecToTarget     = shipPosition - targetPosition;
        const auto vecToForward    = shipPosition - (cannonForward * 100000000.0f);
        const auto angleToTarget   = Math::getAngleBetweenTwoVectors(glm::normalize(vecToTarget), glm::normalize(vecToForward), true);
        if (angleToTarget <= arc) {
            const auto launcherPosition = shipPosition + (shipRotation * position);
            const auto targetLinearVelocity = targetBody.getLinearVelocity();
            const auto distanceToTarget = glm::distance(targetPosition, launcherPosition);

            const auto finalTravelTime = distanceToTarget / finalSpeed;

            auto myShipVelocity = shipBody.getLinearVelocity();
            auto combinedVelocity = targetLinearVelocity - myShipVelocity;
            const auto predictedSpeed = combinedVelocity + (cannonForward * finalTravelTime) /* * target.acceleration */; //TODO: figure this out later
            const auto averageSpeed = (combinedVelocity + predictedSpeed) * 0.5f;
            auto predictedPos = targetPosition + (averageSpeed * finalTravelTime);

            returnValue.pedictedVector = -glm::normalize(launcherPosition - predictedPos);
            returnValue.pedictedPosition = predictedPos;
            return returnValue;
        }
    }
    returnValue.pedictedVector = cannonForward;
    returnValue.pedictedPosition = cannonForward;
    return returnValue;
}
const bool PrimaryWeaponCannon::fire() {
    if (numRounds > 0) {
        --numRounds;
        return true;
    }
    return false;
}
void PrimaryWeaponCannon::forceFire() {

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
PrimaryWeaponBeam::PrimaryWeaponBeam(Ship& _ship, Map& map, const glm::vec3& _pos, const glm::vec3& _fwd, const float& _arc, const uint& _dmg, const float& _impactRad, const float& _impactTime, const float& _volume, vector<glm::vec3>& _windupPts,const uint& _maxCharges,const float& _rechargeTimePerRound, const float& _chargeTimerSpeed, const float& _firingTime) : ShipWeapon(_ship, _pos, _fwd, _arc, _dmg, _impactRad, _impactTime, _volume, _maxCharges, _rechargeTimePerRound) {
    windupPoints = _windupPts;
    chargeTimer = 0.0f;
    chargeTimerSpeed = _chargeTimerSpeed;
    isFiring = false;
    isFiringWeapon = false;
    firingTimeMax = _firingTime;
    firingTime = 0.0f;

    beamLight = new RodLight(_pos, 2.0f, &map);
    beamLight->deactivate();

    beamGraphic = new EntityWrapper(map);

    auto* body = beamGraphic->addComponent<ComponentBody>();
    auto* model = beamGraphic->addComponent<ComponentModel>(ResourceManifest::PhaserBeamMesh, ResourceManifest::PhaserBeamMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& beamModel = model->getModel(0);
    beamModel.hide();
    beamModel.setScale(0.06f);

    beamEndPointGraphic = new EntityWrapper(map);
    auto* body1 = beamEndPointGraphic->addComponent<ComponentBody>();
    auto* model1 = beamEndPointGraphic->addComponent<ComponentModel>(Mesh::Plane, (Material*)ResourceManifest::TorpedoGlow2Material.get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& beamModelEnd = model1->getModel(0);
    beamModelEnd.setScale(0.5f);
    beamModelEnd.hide();
}
PrimaryWeaponBeam::~PrimaryWeaponBeam() {
    if (beamGraphic) {
        beamGraphic->destroy();
        SAFE_DELETE(beamGraphic);
    }
    if (beamEndPointGraphic) {
        beamEndPointGraphic->destroy();
        SAFE_DELETE(beamEndPointGraphic);
    }
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
    return ret;
}
const bool PrimaryWeaponBeam::fire(const double& dt) {
    if (numRounds > 0) {
        return true;
    }
    return false;
}
void PrimaryWeaponBeam::forceFire(const double& dt) {

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
SecondaryWeaponTorpedo::SecondaryWeaponTorpedo(Ship& _ship,const glm::vec3& _pos,const glm::vec3& _fwd,const float& _arc,const uint& _maxCharges,const uint& _dmg,const float& _rechargePerRound,const float& _impactRad,const float& _impactTime,const float& _travelSpeed,const float& _volume,const float& _rotAngleSpeed) : ShipWeapon(_ship,_pos,_fwd,_arc, _dmg, _impactRad, _impactTime, _volume, _maxCharges, _rechargePerRound) {
    damage                   = _dmg;
    impactRadius             = _impactRad;
    impactTime               = _impactTime;
    travelSpeed              = _travelSpeed;
    volume                   = _volume;
    rotationAngleSpeed       = _rotAngleSpeed;
}

const SecondaryWeaponTorpedoPrediction SecondaryWeaponTorpedo::calculatePredictedVector(ComponentBody& projectileBody) {
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
        glm::vec3 targetPosition;
        if (targetIsShip) {
            targetPosition = targetIsShip->getAimPositionDefault();
        }else{
            targetPosition = targetBody.position();
        }
        const auto vecToTarget = shipPosition - targetPosition;
        const auto vecToForward = shipPosition - (torpForward * 100000000.0f);
        const auto angleToTarget = Math::getAngleBetweenTwoVectors(glm::normalize(vecToTarget), glm::normalize(vecToForward), true);
        if (angleToTarget <= arc) {
            returnValue.hasLock = true;
            returnValue.target = mytarget;

            const auto launcherPosition = shipPosition + (shipRotation * position);
            const auto targetLinearVelocity = targetBody.getLinearVelocity();
            const auto distanceToTarget = glm::distance(targetPosition, launcherPosition);


            const auto finalTravelTime = distanceToTarget / finalSpeed;
            auto myShipVelocity = shipBody.getLinearVelocity();

            auto combinedVelocity = targetLinearVelocity - myShipVelocity;
            const auto predictedSpeed = combinedVelocity + (torpForward * finalTravelTime) /* * target.acceleration */; //TODO: figure this out later
            const auto averageSpeed = (combinedVelocity + predictedSpeed) * 0.5f;
            auto predictedPos = targetPosition + (averageSpeed * finalTravelTime);

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
const bool SecondaryWeaponTorpedo::canFire() {
    if (numRounds > 0) {
        return true;
    }
    return false;
}
const bool SecondaryWeaponTorpedo::fire() {
    if (numRounds > 0) {
        --numRounds;
        return true;
    }
    return false;
}
void SecondaryWeaponTorpedo::forceFire() {

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



ShipSystemWeapons::ShipSystemWeapons(Ship& _ship) : ShipSystem(ShipSystemType::Weapons, _ship) {

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
    if (isPlayer && Engine::isMouseButtonDownOnce(MouseButton::Left) && !isCloaked && !isWarping) {
        vector<uint> primaryWeaponsBeamsFired;
        vector<uint> primaryWeaponsCannonsFired;
        for (uint i = 0; i < m_PrimaryWeaponsBeams.size(); ++i) {
            const auto res = m_PrimaryWeaponsBeams[i]->fire(dt);
            if (res) {
                primaryWeaponsBeamsFired.push_back(i);
            }
        }
        for (uint i = 0; i < m_PrimaryWeaponsCannons.size(); ++i) {
            const auto res = m_PrimaryWeaponsCannons[i]->fire();
            if (res) {
                primaryWeaponsCannonsFired.push_back(i);
            }
        }
        //send packet with indices
        if (primaryWeaponsBeamsFired.size() > 0) {
            PacketMessage pOut;
            pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Beams;
            pOut.name = m_Ship.getName();
            pOut.data = to_string(primaryWeaponsBeamsFired[0]);
            for (uint i = 1; i < primaryWeaponsBeamsFired.size(); ++i)
                pOut.data += "," + to_string(primaryWeaponsBeamsFired[i]);
            m_Ship.m_Client.send(pOut);
        }
        if (primaryWeaponsCannonsFired.size() > 0) {
            PacketMessage pOut;
            pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Cannons;
            pOut.name = m_Ship.getName();
            pOut.data = to_string(primaryWeaponsCannonsFired[0]);
            for (uint i = 1; i < primaryWeaponsCannonsFired.size(); ++i)
                pOut.data += "," + to_string(primaryWeaponsCannonsFired[i]);
            m_Ship.m_Client.send(pOut);
        }
    }
    if (isPlayer && Engine::isMouseButtonDownOnce(MouseButton::Right) && !isCloaked && !isWarping) {
        vector<uint> secWeaponsTorpedosFired;

        //first, get the arcs that are in range of the target if applicable
        vector<uint> secWeaponsTorpedosFiredInValidArc;
        auto* mytarget = m_Ship.getTarget();
        if (mytarget) {
            for (uint i = 0; i < m_SecondaryWeaponsTorpedos.size(); ++i) {
                auto torpedo = m_SecondaryWeaponsTorpedos[i];
                if (torpedo->isInControlledArc(mytarget)) {
                    if (torpedo->canFire()) {
                        secWeaponsTorpedosFiredInValidArc.push_back(i);
                    }
                }
            }
        }else{
            for (uint i = 0; i < m_SecondaryWeaponsTorpedos.size(); ++i) {
                auto torpedo = m_SecondaryWeaponsTorpedos[i];
                secWeaponsTorpedosFiredInValidArc.push_back(i);
            }
        }

        //now we have our valid torpedo launchers, pick the launcher with the most rounds
        uint maxRounds = 0;
        uint chosenIndex = 0;
        for (auto& weapIndex : secWeaponsTorpedosFiredInValidArc) {
            auto& launcher = m_SecondaryWeaponsTorpedos[weapIndex];
            if (launcher->numRounds > maxRounds) {
                maxRounds = launcher->numRounds;
                chosenIndex = weapIndex;
            }
        }
        //now fire this chosen launcher
        if(secWeaponsTorpedosFiredInValidArc.size() > 0){
            const auto res = m_SecondaryWeaponsTorpedos[chosenIndex]->fire();
            if (res) {
                secWeaponsTorpedosFired.push_back(chosenIndex);
            }
        }




        //send packet with indices
        if (secWeaponsTorpedosFired.size() > 0) {
            PacketMessage pOut;
            pOut.PacketType = PacketType::Client_To_Server_Client_Fired_Torpedos;
            pOut.name = m_Ship.getName();
            pOut.data = to_string(secWeaponsTorpedosFired[0]);
            for (uint i = 1; i < secWeaponsTorpedosFired.size(); ++i)
                pOut.data += "," + to_string(secWeaponsTorpedosFired[i]);
            m_Ship.m_Client.send(pOut);
        }
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
    m_PrimaryWeaponsBeams.push_back(&beam);
}
void ShipSystemWeapons::addPrimaryWeaponCannon(PrimaryWeaponCannon& cannon) {
    m_PrimaryWeaponsCannons.push_back(&cannon);
}
void ShipSystemWeapons::addSecondaryWeaponTorpedo(SecondaryWeaponTorpedo& torpedo) {
    m_SecondaryWeaponsTorpedos.push_back(&torpedo);
}