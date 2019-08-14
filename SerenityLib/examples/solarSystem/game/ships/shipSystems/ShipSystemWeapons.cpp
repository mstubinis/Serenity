#include "ShipSystemWeapons.h"
#include "../../Ship.h"
#include "../../Packet.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/math/Engine_Math.h>

using namespace Engine;
using namespace std;

PrimaryWeaponCannon::PrimaryWeaponCannon(Ship& _ship,const glm::vec3& _pos,const glm::vec3& _fwd,const float& _arc,const uint& _maxCharges,const uint& _dmg,const float& _rechargePerRound,const float& _impactRad,const float& _impactTime, const float& _travelSpeed):ship(_ship) {
    position                 = _pos;
    forward                  = _fwd;
    arc                      = _arc;
    damage                   = _dmg;
    impactRadius             = _impactRad;
    impactTime               = _impactTime;
    numRounds = numRoundsMax = _maxCharges;
    rechargeTimePerRound     = _rechargePerRound;
    travelSpeed              = _travelSpeed;
    rechargeTimer            = 0.0f;
}
const glm::vec3 PrimaryWeaponCannon::calculatePredictedVector() {
    auto& shipBody = *ship.getComponent<ComponentBody>();
    auto shipRotation = shipBody.rotation();
    const auto cannonForward = glm::normalize(shipRotation * forward);
    auto shipPosition = shipBody.position();
    auto target = ship.getTarget();
    if (target) {
        auto& targetBody          = *target->getComponent<ComponentBody>();
        const auto targetPosition = targetBody.position();
        const auto vecToTarget    = shipPosition - targetPosition;
        const auto vecToForward   = shipPosition - (cannonForward * 100000000.0f);
        const auto angleToTarget  = Math::getAngleBetweenTwoVectors(glm::normalize(vecToTarget), glm::normalize(vecToForward), true);
        if (angleToTarget <= arc) {
            /*
            traveltime      = distance_to_target / bullet_velocity
            predicted_speed = car.speed + traveltime * car.acceleration
            average_speed   = ( car.speed + predicted_speed ) / 2.0
            predicted_pos   = car.pos + (average_speed * traveltime)
            */
            const auto targetLinearVelocity = targetBody.getLinearVelocity();
            const auto distanceToTarget = glm::distance(targetPosition, shipPosition);
            const auto travelTime = distanceToTarget / travelSpeed;
            
            const auto predictedSpeed = targetLinearVelocity + (cannonForward * travelTime) /* * target.acceleration */; //TODO: figure this out later
            const auto averageSpeed = (targetLinearVelocity + predictedSpeed) / 2.0f;
            auto predictedPos = targetPosition + (averageSpeed * travelTime);

            const auto myVelocity = shipBody.getLinearVelocity();
            predictedPos -= (myVelocity / 2.0f);

            return -glm::normalize(shipPosition - predictedPos);
        }
    }
    //not predicted firing
    return cannonForward;
}
bool PrimaryWeaponCannon::fire() {
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
PrimaryWeaponBeam::PrimaryWeaponBeam(Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc) : ship(_ship) {
    position = _position;
    forward = _forward;
    arc = _arc;
}
const glm::vec3 PrimaryWeaponBeam::calculatePredictedVector() {
    glm::vec3 ret = glm::vec3(0.0f);
    return ret;
}
bool PrimaryWeaponBeam::fire() {
    return false;
}
void PrimaryWeaponBeam::forceFire() {

}
void PrimaryWeaponBeam::update(const double& dt) {

}
SecondaryWeaponTorpedo::SecondaryWeaponTorpedo(Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc) : ship(_ship) {
    position = _position;
    forward = _forward;
    arc = _arc;
}
const glm::vec3 SecondaryWeaponTorpedo::calculatePredictedVector() {
    glm::vec3 ret = glm::vec3(0.0f);
    return ret;
}
bool SecondaryWeaponTorpedo::fire() {
    return false;
}
void SecondaryWeaponTorpedo::forceFire() {

}
void SecondaryWeaponTorpedo::update(const double& dt) {

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
        for (uint i = 0; i < m_SecondaryWeaponsTorpedos.size(); ++i) {
            auto res = m_SecondaryWeaponsTorpedos[i]->fire();
            if (res)
                secWeaponsTorpedosFired.push_back(i);
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