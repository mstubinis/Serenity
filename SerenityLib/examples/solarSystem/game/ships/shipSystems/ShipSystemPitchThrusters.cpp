#include "ShipSystemPitchThrusters.h"
#include "../../Ship.h"
#include "../../GameCamera.h"

#include <core/engine/events/Engine_Events.h>

ShipSystemPitchThrusters::ShipSystemPitchThrusters(Ship& _ship, const float additional_strength) :ShipSystem(ShipSystemType::ThrustersPitch, _ship) {
    m_AdditionalStrength = additional_strength;
}
ShipSystemPitchThrusters::~ShipSystemPitchThrusters() {

}
void ShipSystemPitchThrusters::update(const double& dt) {
    if (!m_Ship.isDestroyed()) {
        if (isOnline()) {
            auto& rigidbody = *m_Ship.getComponent<ComponentBody>();
            if (m_Ship.IsPlayer()) {
                 if (m_Ship.getPlayerCamera()->getState() != CameraState::Orbit) {
                    const auto& diff = Engine::getMouseDifference().y;
                    m_Ship.m_MouseFactor.y += diff * 0.00065;
                    const auto massFactor = 1.0f / (rigidbody.mass() * 5.0f);
                    const auto amount = m_Ship.m_MouseFactor.y * massFactor;
                    rigidbody.applyTorque(static_cast<decimal>(amount) * static_cast<decimal>(m_AdditionalStrength), 0, 0);
                    const auto step = (1.0 - dt);
                    m_Ship.m_MouseFactor.y *= (step * step);
                }
            }
        }
    }
    ShipSystem::update(dt);
}