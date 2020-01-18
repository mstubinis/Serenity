#include "ShipSystemCloakingDevice.h"
#include "ShipSystemShields.h"
#include "ShipSystemSensors.h"

#include "../../Ship.h"
#include "../../ships/Ships.h"
#include "../../networking/packets/PacketCloakUpdate.h"
#include "../../ResourceManifest.h"
#include "../../map/Map.h"
#include "../../config/Keybinds.h"

#include <core/engine/system/Engine.h>

using namespace Engine;

ShipSystemCloakingDevice::ShipSystemCloakingDevice(Ship& _ship, const float cloakTimerSpeed, const float maxAlphaWhileCloaked) : ShipSystem(ShipSystemType::CloakingDevice, _ship) {
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
const bool ShipSystemCloakingDevice::cloak(bool sendPacket) {
    return cloak(*m_Ship.getComponent<ComponentModel>(), sendPacket);
}
const bool ShipSystemCloakingDevice::cloak(ComponentModel& model, bool sendPacket) {
    if (m_Ship.isDestroyed())
        return false;
    if (m_CloakTimer >= 1.0f) {
        for (unsigned int i = 0; i < model.getNumModels(); ++i) {
            model.setModelShaderProgram(ResourceManifest::ShipShaderProgramForward, i, RenderStage::ForwardTransparentTrianglesSorted);
        }
        m_Active = true;

        auto* shields = static_cast<ShipSystemShields*>(m_Ship.getShipSystem(ShipSystemType::Shields));
        if (shields) {
            shields->turnOffShields();
        }
        SoundEffect* effect = nullptr;
        if (Ships::Database[m_Ship.getClass()].Faction == FactionEnum::Klingon) {
            effect = Sound::playEffect(ResourceManifest::SoundKlingonCloakingActivated);
        }else{
            effect = Sound::playEffect(ResourceManifest::SoundRomulanCloakingActivated);
        }
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

const bool ShipSystemCloakingDevice::decloak(bool sendPacket) {
    return decloak(*m_Ship.getComponent<ComponentModel>(), sendPacket);
}
const bool ShipSystemCloakingDevice::decloak(ComponentModel& model, bool sendPacket) {
    if (m_Ship.isDestroyed()) {
        forceCloakOff(model, sendPacket);
        return true;
    }
    if (m_CloakTimer <= 0.0f) {
        forceCloakOff(model, sendPacket);
        return true;
    }
    return false;
}
void ShipSystemCloakingDevice::forceCloakOff(bool sendPacket) {
    forceCloakOff(*m_Ship.getComponent<ComponentModel>(), sendPacket);
}
void ShipSystemCloakingDevice::forceCloakOff(ComponentModel& model, bool sendPacket) {
    if (m_CloakTimer < 1.0f) {
        m_Active = false;
        m_CloakTimer = 0.0f;
        model.show();
        SoundEffect* effect = nullptr;
        if (Ships::Database[m_Ship.getClass()].Faction == FactionEnum::Klingon) {
            effect = Sound::playEffect(ResourceManifest::SoundKlingonCloakingDeactivated);
        }else{
            effect = Sound::playEffect(ResourceManifest::SoundRomulanCloakingDeactivated);
        }
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
    }
}

void ShipSystemCloakingDevice::update(const double& dt) {
    auto _fdt = (static_cast<float>(dt) * 0.37f) * m_CloakSpeed;
    auto& model = *m_Ship.getComponent<ComponentModel>();
    if (isOnline()) {
        if (m_Ship.IsPlayer()) {
            if (Keybinds::isPressedDownOnce(KeybindEnum::ToggleCloak)){
                if (!m_Active) {
                    m_Ship.cloak(true);
                }else{
                    m_Ship.decloak(true);
                }
            }
        }
        if (m_Active) {
            Map& map = static_cast<Map&>(m_Ship.entity().scene());
            Ship* playerShip = map.getPlayer();
            if (m_Ship.isAlly(*playerShip) || &m_Ship == playerShip) {
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
        }else{//if not cloaking device is active...
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
    }else{//if cloak is offline...
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