#include "ShipSystemWarpDrive.h"
#include "../../Ship.h"

#include <core/engine/events/Engine_Events.h>

ShipSystemWarpDrive::ShipSystemWarpDrive(Ship& _ship) :ShipSystem(ShipSystemType::WarpDrive, _ship) {

}
ShipSystemWarpDrive::~ShipSystemWarpDrive() {

}
void ShipSystemWarpDrive::update(const double& dt) {
    if (isOnline()) {
        if (!Engine::paused()) {
            if (m_Ship.IsPlayer()) {
                if (Engine::isKeyDownOnce(KeyboardKey::L)) {
                    m_Ship.toggleWarp();
                }
                if (m_Ship.IsWarping()) {
                    if (Engine::isKeyDown(KeyboardKey::W)) {
                        m_Ship.translateWarp(0.1, dt);
                    }else if (Engine::isKeyDown(KeyboardKey::S)) {
                        m_Ship.translateWarp(-0.1, dt);
                    }
                }
            }
        }
    }
    ShipSystem::update(dt);
}