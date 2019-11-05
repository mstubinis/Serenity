#include "ShipSystemMainThrusters.h"
#include "../../Ship.h"

#include <ecs/ComponentBody.h>
#include <core/engine/events/Engine_Events.h>

ShipSystemMainThrusters::ShipSystemMainThrusters(Ship& _ship, const float additional_force_strength) :ShipSystem(ShipSystemType::ThrustersMain, _ship) {
    m_AdditionalForceStrength = additional_force_strength;
}
ShipSystemMainThrusters::~ShipSystemMainThrusters() {

}
void ShipSystemMainThrusters::update(const double& dt) {
    auto& rigidbody = *m_Ship.getComponent<ComponentBody>();
    if (isOnline()) {
        if (m_Ship.IsPlayer()) {
            bool ismoving = false;
            if (!m_Ship.IsWarping()) {
                const float& amount = 1.3f / ((rigidbody.mass() * 0.15f) + 1.0f);

                const decimal final_amount = static_cast<decimal>(amount) * static_cast<decimal>(m_AdditionalForceStrength);

                if (Engine::isKeyDown(KeyboardKey::W)) {
                    rigidbody.applyForce(0, 0, -final_amount);
                    ismoving = true;
                }
                if (Engine::isKeyDown(KeyboardKey::S)) {
                    rigidbody.applyForce(0, 0, final_amount);
                    ismoving = true;
                }
                if (Engine::isKeyDown(KeyboardKey::A)) {
                    rigidbody.applyForce(-final_amount, 0, 0);
                    ismoving = true;
                }
                if (Engine::isKeyDown(KeyboardKey::D)) {
                    rigidbody.applyForce(final_amount, 0, 0);
                    ismoving = true;
                }
                if (Engine::isKeyDown(KeyboardKey::F)) {
                    rigidbody.applyForce(0, -final_amount, 0);
                    ismoving = true;
                }
                if (Engine::isKeyDown(KeyboardKey::R)) {
                    rigidbody.applyForce(0, final_amount, 0);
                    ismoving = true;
                }
            }
            if (!ismoving) {
                auto current = rigidbody.getLinearVelocity();
                rigidbody.setDamping(static_cast<decimal>(0.05), static_cast<decimal>(0.2));
            }else{
                rigidbody.setDamping(static_cast<decimal>(0.0), static_cast<decimal>(0.2));
            }
        }
    }else{
        rigidbody.setDamping(static_cast<decimal>(0.0), static_cast<decimal>(0.2));
    }
    ShipSystem::update(dt);
}