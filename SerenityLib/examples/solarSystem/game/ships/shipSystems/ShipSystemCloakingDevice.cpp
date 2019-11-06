#include "ShipSystemCloakingDevice.h"
#include "ShipSystemShields.h"
#include "ShipSystemSensors.h"

#include "../../Ship.h"
#include "../../networking/Packet.h"
#include "../../ResourceManifest.h"
#include "../../map/Map.h"

#include <core/engine/sounds/Engine_Sounds.h>

using namespace Engine;

ShipSystemCloakingDevice::ShipSystemCloakingDevice(Ship& _ship, const float cloakTimerSpeed, const float maxAlphaWhileCloaked) :ShipSystem(ShipSystemType::CloakingDevice, _ship) {
    m_Active = false;
    m_CloakTimer = 1.0f;
    m_CloakSpeed = cloakTimerSpeed;
    m_MaxAlphaWhileCloaked = maxAlphaWhileCloaked;
}
ShipSystemCloakingDevice::~ShipSystemCloakingDevice() {

}
const bool  ShipSystemCloakingDevice::isCloakActive() const {
    return m_Active;
}
const float ShipSystemCloakingDevice::getCloakTimer() const {
    return m_CloakTimer;
}
const float ShipSystemCloakingDevice::getMaxAlphaWhileCloaked() const {
    return m_MaxAlphaWhileCloaked;
}
const bool ShipSystemCloakingDevice::cloak(ComponentModel& model, bool sendPacket) {
    if (m_CloakTimer >= 1.0f) {
        for (unsigned int i = 0; i < model.getNumModels(); ++i) {
            model.setModelShaderProgram(ResourceManifest::ShipShaderProgramForward, i, RenderStage::ForwardTransparentTrianglesSorted);
        }
        m_Active = true;

        auto* shields = static_cast<ShipSystemShields*>(m_Ship.getShipSystem(ShipSystemType::Shields));
        if (shields) {
            shields->turnOffShields();
        }

        auto effect = Sound::playEffect(ResourceManifest::SoundCloakingActivated);
        if (effect) {
            auto& body = *m_Ship.getComponent<ComponentBody>();
            effect->setPosition(body.position());
            effect->setAttenuation(0.15f);
        }
        if (sendPacket) {
            PacketCloakUpdate pOut(m_Ship);
            pOut.PacketType = PacketType::Client_To_Server_Ship_Cloak_Update;
            pOut.justTurnedOn = true;
            m_Ship.m_Client.send(pOut);
        }
        return true;
    }
    return false;
}
const bool ShipSystemCloakingDevice::decloak(ComponentModel& model, bool sendPacket) {
    if (m_CloakTimer <= 0.0f) {
        m_Active = false;
        m_CloakTimer = 0.0f;
        model.show();
        auto effect = Sound::playEffect(ResourceManifest::SoundCloakingDeactivated);
        if (effect) {
            auto& body = *m_Ship.getComponent<ComponentBody>();
            effect->setPosition(body.position());
            effect->setAttenuation(0.15f);
        }
        if (sendPacket) {
            PacketCloakUpdate pOut(m_Ship);
            pOut.PacketType = PacketType::Client_To_Server_Ship_Cloak_Update;
            pOut.justTurnedOff = true;
            m_Ship.m_Client.send(pOut);
        }
        return true;
    }
    return false;
}
void ShipSystemCloakingDevice::update(const double& dt) {
    auto _fdt = (static_cast<float>(dt) * 0.37f) * m_CloakSpeed;
    auto& model = *m_Ship.getComponent<ComponentModel>();
    if (isOnline()) {
        if (m_Ship.IsPlayer()) {
            if (Engine::isKeyDownOnce(KeyboardKey::C)) {
                if (!m_Active) {
                    ShipSystemCloakingDevice::cloak(model);
                }else{
                    ShipSystemCloakingDevice::decloak(model);
                }
            }
        }
        if (m_Active) {
            Map& map = static_cast<Map&>(m_Ship.entity().scene());
            Ship* playerShip = map.getPlayer();
            if (m_Ship.canSeeCloak(playerShip)) {
                if (m_CloakTimer > -m_MaxAlphaWhileCloaked) {
                    if (m_CloakTimer > 0.0f) {
                        m_CloakTimer -= _fdt;
                        m_Ship.updateCloakVisuals(1, 1, 1, glm::abs(m_CloakTimer), model);
                    }else{
                        m_CloakTimer -= _fdt * 0.35f;
                        if (m_CloakTimer < -m_MaxAlphaWhileCloaked) {
                            m_CloakTimer = -m_MaxAlphaWhileCloaked;
                        }
                        m_Ship.updateCloakVisuals(0.369f, 0.912f, 1, glm::abs(m_CloakTimer), model);
                    }
                }
            }else{
                bool isAntiCloakDetected = false;
                auto* sensors = static_cast<ShipSystemSensors*>(playerShip->getShipSystem(ShipSystemType::Sensors));
                if (sensors) {
                    for (auto& detection : sensors->getAntiCloakDetectedShips()) {
                        if (detection.ship == &m_Ship) {
                            auto factor = static_cast<float>(detection.detection_timer_max - detection.detection_timer_current) * 0.08f;
                            factor = glm::min(factor, m_MaxAlphaWhileCloaked);
                            model.show();
                            m_Ship.updateCloakVisuals(0.369f, 0.912f, 1, factor, model);
                            isAntiCloakDetected = true;
                            break;
                        }
                    }
                }
                if (!isAntiCloakDetected) {
                    if (m_CloakTimer > 0.0f) {
                        m_CloakTimer -= _fdt;
                        if (m_CloakTimer < 0.0f) {
                            m_CloakTimer = 0.0f;
                            model.hide();
                        }
                        m_Ship.updateCloakVisuals(1, 1, 1, glm::abs(m_CloakTimer), model);
                    }
                }
            }
        }else{
            if (m_CloakTimer < 1.0f) {
                model.show();
                m_CloakTimer += _fdt;
                if (m_CloakTimer > 1.0f) {
                    m_CloakTimer = 1.0f;

                    for (unsigned int i = 0; i < model.getNumModels(); ++i) {
                        model.setModelShaderProgram(ResourceManifest::ShipShaderProgramDeferred, i, RenderStage::GeometryOpaque);
                    }

                    auto* shields = static_cast<ShipSystemShields*>(m_Ship.getShipSystem(ShipSystemType::Shields));
                    if (shields) {
                        shields->turnOnShields();
                    }
                }
            }
            m_Ship.updateCloakVisuals(1, 1, 1, glm::abs(m_CloakTimer), model);
        }
    }else{
        if (m_CloakTimer <= 0.0f) {
            ShipSystemCloakingDevice::decloak(model);
        }
        m_Active = false;
        if (m_CloakTimer < 1.0f) {
            m_CloakTimer += _fdt;
            if (m_CloakTimer > 1.0f) {
                m_CloakTimer = 1.0f;

                for (unsigned int i = 0; i < model.getNumModels(); ++i) {
                    model.setModelShaderProgram(ResourceManifest::ShipShaderProgramDeferred, i, RenderStage::GeometryOpaque);
                }
                auto* shields = static_cast<ShipSystemShields*>(m_Ship.getShipSystem(ShipSystemType::Shields));
                if (shields) {
                    shields->turnOnShields();
                }
            }
        }
        m_Ship.updateCloakVisuals(1, 1, 1, glm::abs(m_CloakTimer), model);
    }
    ShipSystem::update(dt);
}