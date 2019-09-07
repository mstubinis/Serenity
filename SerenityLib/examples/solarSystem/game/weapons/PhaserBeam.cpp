#include "PhaserBeam.h"

PhaserBeam::PhaserBeam(Ship& ship, Map& map, const glm::vec3& position, const glm::vec3& forward, const float& arc, std::vector<glm::vec3>& windupPts, const uint& damage, const float& _impactRadius, const float& _impactTime, const float& _volume) : PrimaryWeaponBeam(ship, position, forward, arc, damage, _impactRadius, _impactTime, _volume, windupPts), m_Map(map) {

}
PhaserBeam::~PhaserBeam() {

}
const bool PhaserBeam::fire() {
    return true;
}
void PhaserBeam::forceFire() {

}
void PhaserBeam::update(const double& dt) {

}