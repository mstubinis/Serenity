#include "ShipSystemWarpDrive.h"
#include "../../Ship.h"
#include "../../config/Keybinds.h"

#include <core/engine/events/Engine_Events.h>

ShipSystemWarpDrive::ShipSystemWarpDrive(Ship& _ship) : ShipSystem(ShipSystemType::WarpDrive, _ship) {

}
ShipSystemWarpDrive::~ShipSystemWarpDrive() {

}
void ShipSystemWarpDrive::update(const double& dt) {
    if (!m_Ship.isDestroyed()) {
        if (isOnline()) {
            if (m_Ship.IsPlayer()) {
                if (Keybinds::isPressedDownOnce(KeybindEnum::ToggleWarpDrive)) {
                    m_Ship.toggleWarp();
                }
                if (m_Ship.IsWarping()) {
                    if (Keybinds::isPressedDown(KeybindEnum::MoveForward)) {
                        m_Ship.translateWarp(0.1, dt);
                    }else if (Keybinds::isPressedDown(KeybindEnum::MoveBackward)) {
                        m_Ship.translateWarp(-0.1, dt);
                    }
                }
            }
        }
    }
    ShipSystem::update(dt);
}