#include "ShipSystemWeapons.h"
#include "../../Ship.h"
#include "../../Packet.h"

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

ShipWeapon::ShipWeapon(Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc):ship(_ship) {
    position = _position;
    forward  = _forward;
    arc      = _arc;
}
const bool ShipWeapon::isInArc(EntityWrapper* target, const float _arc) {
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

PrimaryWeaponCannon::PrimaryWeaponCannon(Ship& _ship,const glm::vec3& _pos,const glm::vec3& _fwd,const float& _arc,const uint& _maxCharges,const uint& _dmg,const float& _rechargePerRound,const float& _impactRad,const float& _impactTime, const float& _travelSpeed, const float& _volume) : ShipWeapon(_ship, _pos, _fwd, _arc) {
    damage                   = _dmg;
    impactRadius             = _impactRad;
    impactTime               = _impactTime;
    numRounds = numRoundsMax = _maxCharges;
    rechargeTimePerRound     = _rechargePerRound;
    travelSpeed              = _travelSpeed;
    volume                   = _volume;
    rechargeTimer            = 0.0f;
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
    if (numRounds < numRoundsMax) {
        const float fdt = static_cast<float>(dt);
        rechargeTimer += fdt;
        if (rechargeTimer >= rechargeTimePerRound) {
            ++numRounds;
            rechargeTimer = 0.0f;
        }
    }
}
PrimaryWeaponBeam::PrimaryWeaponBeam(Ship& _ship, const glm::vec3& _pos, const glm::vec3& _fwd, const float& _arc) : ShipWeapon(_ship, _pos, _fwd, _arc) {

}
const const glm::vec3 PrimaryWeaponBeam::calculatePredictedVector() {
    glm::vec3 ret = glm::vec3(0.0f);
    return ret;
}
const bool PrimaryWeaponBeam::fire() {
    return false;
}
void PrimaryWeaponBeam::forceFire() {

}
void PrimaryWeaponBeam::update(const double& dt) {

}
SecondaryWeaponTorpedo::SecondaryWeaponTorpedo(Ship& _ship,const glm::vec3& _pos,const glm::vec3& _fwd,const float& _arc,const uint& _maxCharges,const uint& _dmg,const float& _rechargePerRound,const float& _impactRad,const float& _impactTime,const float& _travelSpeed,const float& _volume,const float& _rotAngleSpeed) : ShipWeapon(_ship,_pos,_fwd,_arc) {
    damage                   = _dmg;
    impactRadius             = _impactRad;
    impactTime               = _impactTime;
    numRounds = numRoundsMax = _maxCharges;
    rechargeTimePerRound     = _rechargePerRound;
    travelSpeed              = _travelSpeed;
    volume                   = _volume;
    rotationAngleSpeed       = _rotAngleSpeed;
    rechargeTimer            = 0.0f;
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
            auto res = m_PrimaryWeaponsBeams[i]->fire();
            if (res)
                primaryWeaponsBeamsFired.push_back(i);
        }
        for (uint i = 0; i < m_PrimaryWeaponsCannons.size(); ++i) {
            auto res = m_PrimaryWeaponsCannons[i]->fire();
            if (res)
                primaryWeaponsCannonsFired.push_back(i);
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
            auto res = m_SecondaryWeaponsTorpedos[chosenIndex]->fire();
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