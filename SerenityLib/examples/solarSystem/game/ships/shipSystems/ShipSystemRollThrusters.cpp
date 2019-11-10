#include "ShipSystemRollThrusters.h"
#include "../../Ship.h"
#include "../../GameCamera.h"

#include <core/engine/events/Engine_Events.h>

ShipSystemRollThrusters::ShipSystemRollThrusters(Ship& _ship, const float additional_strength) :ShipSystem(ShipSystemType::ThrustersRoll, _ship) {
    m_AdditionalStrength = additional_strength;
}
ShipSystemRollThrusters::~ShipSystemRollThrusters() {

}
void ShipSystemRollThrusters::update(const double& dt) {
    if (!m_Ship.isDestroyed()) {
        if (isOnline()) {
            auto& rigidbody = *m_Ship.getComponent<ComponentBody>();
            if (m_Ship.IsPlayer()) {
                float amount = 1.0f / rigidbody.mass();
                if (Engine::isKeyDown(KeyboardKey::Q)) {
                    rigidbody.applyTorque(0, 0, static_cast<decimal>(amount) * static_cast<decimal>(m_AdditionalStrength));
                }
                if (Engine::isKeyDown(KeyboardKey::E)) {
                    rigidbody.applyTorque(0, 0, static_cast<decimal>(-amount) * static_cast<decimal>(m_AdditionalStrength));
                }
            }
        }
    }
    ShipSystem::update(dt);
}