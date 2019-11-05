#include "ShipSystemReactor.h"

ShipSystemReactor::ShipSystemReactor(Ship& _ship, const float maxPower, const float currentPower) :ShipSystem(ShipSystemType::Reactor, _ship) {
    if (currentPower == -1) {
        m_TotalPower = maxPower;
    }else{
        m_TotalPower = currentPower;
    }
    m_TotalPowerMax = maxPower;
}
ShipSystemReactor::~ShipSystemReactor() {

}
void ShipSystemReactor::update(const double& dt) {
    ShipSystem::update(dt);
}