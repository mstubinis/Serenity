#include "ShipSystemWeapons.h"
#include "../../Ship.h"

#include <core/engine/events/Engine_Events.h>

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
bool PrimaryWeaponCannon::fire() {
    if (numRounds > 0) {
        --numRounds;
        return true;
    }
    return false;
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
bool PrimaryWeaponBeam::fire() {
    return false;
}
void PrimaryWeaponBeam::update(const double& dt) {

}
SecondaryWeaponTorpedo::SecondaryWeaponTorpedo(Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc) : ship(_ship) {
    position = _position;
    forward = _forward;
    arc = _arc;
}
bool SecondaryWeaponTorpedo::fire() {
    return false;
}
void SecondaryWeaponTorpedo::update(const double& dt) {

}



ShipSystemWeapons::ShipSystemWeapons(Ship& _ship) :ShipSystem(ShipSystemType::Weapons, _ship) {

}
ShipSystemWeapons::~ShipSystemWeapons() {
   SAFE_DELETE_VECTOR(m_PrimaryWeaponsCannons);
   SAFE_DELETE_VECTOR(m_PrimaryWeaponsBeams);
   SAFE_DELETE_VECTOR(m_SecondaryWeaponsTorpedos);
}
void ShipSystemWeapons::update(const double& dt) {
    const bool isCloaked = m_Ship.isCloaked();
    const bool isWarping = m_Ship.IsWarping();
    if (Engine::isMouseButtonDownOnce(MouseButton::Left) && !isCloaked && !isWarping) {
        for (auto& beam : m_PrimaryWeaponsBeams)
            beam->fire();
        for (auto& cannon : m_PrimaryWeaponsCannons)
            cannon->fire();
    }
    if (Engine::isMouseButtonDownOnce(MouseButton::Right) && !isCloaked && !isWarping) {
        for (auto& torpedo : m_SecondaryWeaponsTorpedos)
            torpedo->fire();
    }
    for (auto& beam : m_PrimaryWeaponsBeams)
        beam->update(dt);
    for (auto& cannon : m_PrimaryWeaponsCannons)
        cannon->update(dt);
    for (auto& torpedo : m_SecondaryWeaponsTorpedos)
        torpedo->update(dt);
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