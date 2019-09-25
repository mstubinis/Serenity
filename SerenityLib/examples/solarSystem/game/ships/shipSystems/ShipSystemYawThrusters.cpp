#include "ShipSystemYawThrusters.h"
#include "../../Ship.h"
#include "../../GameCamera.h"

#include <core/engine/events/Engine_Events.h>

ShipSystemYawThrusters::ShipSystemYawThrusters(Ship& _ship) :ShipSystem(ShipSystemType::ThrustersYaw, _ship) {

}
ShipSystemYawThrusters::~ShipSystemYawThrusters() {

}
void ShipSystemYawThrusters::update(const double& dt) {
    if (isOnline()) {
        auto& rigidbody = *m_Ship.getComponent<ComponentBody>();
        if (m_Ship.IsPlayer()) {
            if (m_Ship.getPlayerCamera()->getState() != CameraState::Orbit) {
                const auto& diff = -Engine::getMouseDifference().x;
                m_Ship.m_MouseFactor.x += diff * 0.00065;
                const float massFactor = 1.0f / (rigidbody.mass() * 5.0f);
                const float amount = m_Ship.m_MouseFactor.x * massFactor;
                rigidbody.applyTorque(0, amount, 0);
                const double step = (1.0 - dt);
                m_Ship.m_MouseFactor.x *= (step * step);
            }
        }
    }
    ShipSystem::update(dt);
}