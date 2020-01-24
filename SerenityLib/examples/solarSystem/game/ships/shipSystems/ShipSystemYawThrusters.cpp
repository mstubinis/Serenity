#include "ShipSystemYawThrusters.h"
#include "../../Ship.h"
#include "../../GameCamera.h"

#include <core/engine/events/Engine_Events.h>

ShipSystemYawThrusters::ShipSystemYawThrusters(Ship& _ship, const float additional_strength) :ShipSystem(ShipSystemType::ThrustersYaw, _ship) {
    m_AdditionalStrength = additional_strength;
}
ShipSystemYawThrusters::~ShipSystemYawThrusters() {

}
void ShipSystemYawThrusters::update(const double& dt) {
    if (!m_Ship.isDestroyed()) {
        if (isOnline()) {
            auto& rigidbody = *m_Ship.getComponent<ComponentBody>();
            if (m_Ship.IsPlayer()) {
                if (m_Ship.getPlayerCamera()->getState() != CameraState::Orbit) {
                    const auto& diff = -Engine::getMouseDifference().x;
                    m_Ship.m_MouseFactor.x += diff * 0.00065;
                    const auto massFactor = 1.0f / (rigidbody.mass() * 5.0f);
                    const auto amount = m_Ship.m_MouseFactor.x * massFactor;
                    rigidbody.applyTorque(0, static_cast<decimal>(amount) * static_cast<decimal>(m_AdditionalStrength), 0);
                    const auto step = (1.0 - dt);
                    m_Ship.m_MouseFactor.x *= (step * step);
                }
            }
        }
    }
    ShipSystem::update(dt);
}