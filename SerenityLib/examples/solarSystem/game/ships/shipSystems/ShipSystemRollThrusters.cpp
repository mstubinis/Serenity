#include "ShipSystemRollThrusters.h"
#include "../../Ship.h"
#include "../../GameCamera.h"

#include <core/engine/events/Engine_Events.h>

ShipSystemRollThrusters::ShipSystemRollThrusters(Ship& _ship) :ShipSystem(ShipSystemType::ThrustersRoll, _ship) {

}
ShipSystemRollThrusters::~ShipSystemRollThrusters() {

}
void ShipSystemRollThrusters::update(const double& dt) {
    if (isOnline()) {
        auto& rigidbody = *m_Ship.getComponent<ComponentBody>();
        if (m_Ship.IsPlayer()) {
            float amount = 1.0f / rigidbody.mass();
            if (Engine::isKeyDown(KeyboardKey::Q)) {
                rigidbody.applyTorque(0, 0, amount);
            }
            if (Engine::isKeyDown(KeyboardKey::E)) {
                rigidbody.applyTorque(0, 0, -amount);
            }
        }
    }
    ShipSystem::update(dt);
}