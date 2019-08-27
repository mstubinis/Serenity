#include "ShipSystemMainThrusters.h"
#include "../../Ship.h"

#include <ecs/ComponentBody.h>
#include <core/engine/events/Engine_Events.h>

ShipSystemMainThrusters::ShipSystemMainThrusters(Ship& _ship) :ShipSystem(ShipSystemType::ThrustersMain, _ship) {

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
                if (Engine::isKeyDown(KeyboardKey::W)) {
                    rigidbody.applyForce(0, 0, -amount);
                    ismoving = true;
                }
                if (Engine::isKeyDown(KeyboardKey::S)) {
                    rigidbody.applyForce(0, 0, amount);
                    ismoving = true;
                }
                if (Engine::isKeyDown(KeyboardKey::A)) {
                    rigidbody.applyForce(-amount, 0, 0);
                    ismoving = true;
                }
                if (Engine::isKeyDown(KeyboardKey::D)) {
                    rigidbody.applyForce(amount, 0, 0);
                    ismoving = true;
                }
                if (Engine::isKeyDown(KeyboardKey::F)) {
                    rigidbody.applyForce(0, -amount, 0);
                    ismoving = true;
                }
                if (Engine::isKeyDown(KeyboardKey::R)) {
                    rigidbody.applyForce(0, amount, 0);
                    ismoving = true;
                }
            }
            if (!ismoving) {
                auto current = rigidbody.getLinearVelocity();
                const_cast<btRigidBody&>(rigidbody.getBtBody()).setDamping(0.05f, 0.2f);
            }else{
                const_cast<btRigidBody&>(rigidbody.getBtBody()).setDamping(0.0f, 0.2f);
            }
        }
    }else{
        const_cast<btRigidBody&>(rigidbody.getBtBody()).setDamping(0.0f, 0.2f);
    }
    ShipSystem::update(dt);
}