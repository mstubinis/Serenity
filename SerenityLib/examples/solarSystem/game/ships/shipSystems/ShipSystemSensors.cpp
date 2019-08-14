#include "ShipSystemSensors.h"
#include "../../Ship.h"

ShipSystemSensors::ShipSystemSensors(Ship& _ship) :ShipSystem(ShipSystemType::Sensors, _ship) {

}
ShipSystemSensors::~ShipSystemSensors() {

}
void ShipSystemSensors::update(const double& dt) {
    if (m_Ship.getTarget()) {
        Ship* target = dynamic_cast<Ship*>(m_Ship.getTarget());
        if (target) {
            if (target->isFullyCloaked()) {
                m_Ship.setTarget(nullptr, true);
            }
        }
    }
    ShipSystem::update(dt);
}