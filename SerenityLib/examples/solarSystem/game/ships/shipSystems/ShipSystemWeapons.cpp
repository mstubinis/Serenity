#include "ShipSystemWeapons.h"
#include "../../Ship.h"

#include <core/engine/events/Engine_Events.h>

using namespace Engine;
using namespace std;

PrimaryWeaponCannon::PrimaryWeaponCannon(Ship& _ship, const glm::vec3& _position, const glm::vec3& _forward, const float& _arc):ship(_ship) {
    position = _position;
    forward = _forward;
    arc = _arc;
}
bool PrimaryWeaponCannon::fire() {
    return false;
}
void PrimaryWeaponCannon::update(const double& dt) {

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
    if (Engine::isMouseButtonDownOnce(MouseButton::Left) && !isCloaked) {
        for (auto& beam : m_PrimaryWeaponsBeams)
            beam->fire();
        for (auto& cannon : m_PrimaryWeaponsCannons)
            cannon->fire();
    }
    if (Engine::isMouseButtonDownOnce(MouseButton::Right) && !isCloaked) {
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
    this->m_PrimaryWeaponsBeams.push_back(&beam);
}
void ShipSystemWeapons::addPrimaryWeaponCannon(PrimaryWeaponCannon& cannon) {
    this->m_PrimaryWeaponsCannons.push_back(&cannon);
}
void ShipSystemWeapons::addSecondaryWeaponTorpedo(SecondaryWeaponTorpedo& torpedo) {
    this->m_SecondaryWeaponsTorpedos.push_back(&torpedo);
}