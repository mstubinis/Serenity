#include "ShipSystemSensors.h"
#include "ShipSystemCloakingDevice.h"
#include "../../Ship.h"
#include "../../ResourceManifest.h"
#include "../../map/Map.h"
#include "../../Helper.h"
#include "../../Core.h"
#include "../../hud/HUD.h"
#include "../../GameCamera.h"
#include "../../networking/Packet.h"

#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/sounds/Engine_Sounds.h>
#include <glm/gtx/norm.hpp>

using namespace Engine;
using namespace std;

ShipSystemSensors::ShipSystemSensors(Ship& _ship, Map& map, const decimal& range, const double AntiCloakScanPingTime) :ShipSystem(ShipSystemType::Sensors, _ship),m_Map(map){
    m_RadarRange            = range;
    m_Target                = nullptr;
    m_AntiCloakScanTimer    = 0.0;
    m_AntiCloakScanTimerSound = 0.0;
    m_AntiCloakScanActive   = false;
    m_AntiCloakScanPingTime = AntiCloakScanPingTime;
}
ShipSystemSensors::~ShipSystemSensors() {

}

const double& ShipSystemSensors::getAntiCloakingScanTimer() const {
    return m_AntiCloakScanTimer;
}
const bool& ShipSystemSensors::isAntiCloakScanActive() const {
    return m_AntiCloakScanActive;
}
void ShipSystemSensors::sendAntiCloakScanStatusPacket() {
    PacketMessage pOut;
    pOut.PacketType = PacketType::Client_To_Server_Anti_Cloak_Status;
    pOut.r = m_AntiCloakScanTimer;
    pOut.name = m_Ship.getName();
    pOut.data = m_AntiCloakScanActive ? "1" : "0";
    m_Ship.m_Client.send(pOut);
}
const bool ShipSystemSensors::toggleAntiCloakScan(const bool sendPacket) {
    if (!m_AntiCloakScanActive) {
        if (!m_Ship.isCloaked()) {
            m_AntiCloakScanActive = true;
            m_AntiCloakScanTimer = 0.0;
            m_AntiCloakScanTimerSound = 0.0;
            if (sendPacket) {
                sendAntiCloakScanStatusPacket();
            }
            return true;
        }
    }else{
        m_AntiCloakScanTimer = 0.0;
        m_AntiCloakScanTimerSound = 0.0;
        m_AntiCloakScanActive = false;
        if (sendPacket) {
            sendAntiCloakScanStatusPacket();
        }
        return true;
    }
    return false;
}
const bool ShipSystemSensors::isShipDetectedByAntiCloak(Ship* ship) {
    if (m_DetectedAntiCloakedShips.size() == 0) {
        return false;
    }else{
        for (auto& ship_ptr : m_DetectedAntiCloakedShips) {
            if (ship_ptr.ship == ship) {
                return true;
            }
        }
    }
    return false;
}

EntityWrapper* ShipSystemSensors::getTarget() {
    return m_Target;
}
void ShipSystemSensors::setTarget(EntityWrapper* target, const bool sendPacket) {
    const auto is_player = m_Ship.IsPlayer();
    if (!target) {
        if (is_player && m_Ship.m_PlayerCamera) {
            m_Ship.m_PlayerCamera->setTarget(&m_Ship);
        }
    }
    Ship* ship = dynamic_cast<Ship*>(target);
    if (ship && ship->isFullyCloaked() && !ship->isAlly(m_Ship)) {
        if (!isShipDetectedByAntiCloak(ship)) {
            return;
        }
    }
    if (sendPacket) {
        PacketMessage pOut;
        pOut.PacketType = PacketType::Client_To_Server_Client_Changed_Target;
        pOut.name = m_Ship.getName();
        if (target) {
            auto* cName = target->getComponent<ComponentName>();
            if (cName) {
                pOut.data = cName->name();
            }
        }else{
            pOut.data = "";
        }
        m_Ship.m_Client.send(pOut);
    }
    m_Target = target;
    if (is_player) {
        m_Map.getHUD().setTarget(m_Target);
    }
}
void ShipSystemSensors::setTarget(const string& targetName, const bool sendPacket) {
    if (targetName.empty()) {
        setTarget(nullptr, sendPacket);
    }
    Map& map = static_cast<Map&>(m_Ship.entity().scene());
    for (auto& entity : map.m_Objects) {
        auto* componentName = entity->getComponent<ComponentName>();
        if (componentName) {
            if (componentName->name() == targetName) {
                setTarget(entity, sendPacket);
                return;
            }
        }
    }
}

const ShipSystemSensors::Detection ShipSystemSensors::validateDetection(Ship& othership, const glm_vec3& thisShipPos) {
    const auto dist2 = glm::distance2(othership.getPosition(), thisShipPos);

    Detection d;
    d.valid = false;
    d.distanceSquared = dist2;
    if (dist2 <= m_RadarRange * m_RadarRange) {
        if (  (!othership.isFullyCloaked() && !othership.isAlly(m_Ship)) || (othership.isAlly(m_Ship))  /*   or is it an enemy ship detected by anti-cloak scan?    */   ) {
            d.valid = true;
        }
    }
    return d;
}


void ShipSystemSensors::internal_update_anti_cloak_scan(const double& dt) {
    m_AntiCloakScanTimer += dt;
    m_AntiCloakScanTimerSound += dt;
    if (m_AntiCloakScanTimerSound > m_AntiCloakScanPingTime - 1.0) {
        auto* sound = Sound::playEffect(ResourceManifest::SoundAntiCloakScan);
        if (sound) {
            sound->setPosition(m_Ship.getPosition());
            sound->setAttenuation(0.2f);
        }
        m_AntiCloakScanTimerSound = -100.0;
    }
    if (m_AntiCloakScanTimer > m_AntiCloakScanPingTime) {
        //ping
        auto maxRange = m_RadarRange * m_RadarRange;
        float rand;
        double percent, dist2, factor;
        for (auto& ship_itr : m_Map.getShips()) {
            auto& ship = *ship_itr.second;
            if (!ship.isAlly(m_Ship) && ship.isFullyCloaked()) {
                dist2 = glm::distance2(ship.getPosition(), m_Ship.getPosition());
                if (dist2 <= maxRange) {
                    factor = ((maxRange - dist2) / maxRange);
                    percent = (factor * 25.0); //0 to 25, modify by signature radius, ship perks, etc
                    percent = glm::clamp(percent, 0.1, 25.0);
                    rand = Helper::GetRandomFloatFromTo(0.0f, 100.0f);
                    if (rand <= percent) {
                        //success

                        auto* sound = Sound::playEffect(ResourceManifest::SoundAntiCloakScanDetection);
                        if (sound) {
                            //sound->setPosition(m_Ship.getPosition());
                            //sound->setAttenuation(0.0f);
                        }
                        AntiCloakDetection d;

                        d.ship = &ship;
                        d.detection_timer_current = 0.0;
                        d.detection_timer_max = 4.5; //this should probably be modified by signature radius and other stuff

                        m_DetectedAntiCloakedShips.push_back(std::move(d));
                    }
                }

            }
        }
        m_AntiCloakScanTimerSound = 0.0;
        m_AntiCloakScanTimer = 0.0;
    }
}
void ShipSystemSensors::internal_update_anti_cloak_scan_detected_ships(const double& dt) {
    if (m_DetectedAntiCloakedShips.size() > 0) {
        for (size_t i = 0; i < m_DetectedAntiCloakedShips.size(); ++i) {
            auto& detection = m_DetectedAntiCloakedShips[i];
            detection.detection_timer_current += dt;
            if (detection.detection_timer_current > detection.detection_timer_max) {
                m_DetectedAntiCloakedShips.erase(m_DetectedAntiCloakedShips.begin() + i);
            }
        }
    }
}
void ShipSystemSensors::internal_update_populate_detected_ships(const double& dt) {
    m_DetectedShips.clear();
    m_DetectedEnemyShips.clear();
    m_DetectedAlliedShips.clear();
    m_DetectedNeutralShips.clear();
    for (auto& shipItr : m_Map.getShips()) {
        auto& ship = *shipItr.second;
        if (&ship != &m_Ship) {
            const auto& res = validateDetection(ship, m_Ship.getPosition());
            if (res.valid) {
                DetectedShip detected;
                detected.ship = &ship;
                detected.distanceAway2 = res.distanceSquared;

                m_DetectedShips.push_back(detected);//std::move() here?
                if (ship.isAlly(m_Ship)) {
                    m_DetectedAlliedShips.push_back(std::move(detected));
                }else if (ship.isEnemy(m_Ship)) {
                    m_DetectedEnemyShips.push_back(std::move(detected));
                }else if (ship.isNeutral(m_Ship)) {
                    m_DetectedNeutralShips.push_back(std::move(detected));
                }
            }
        }
    }
}
void ShipSystemSensors::internal_update_clear_target_automatically_if_applicable(const double& dt) {
    auto* thisShipTarget = m_Ship.getTarget();
    if (thisShipTarget) {
        Ship* target = dynamic_cast<Ship*>(thisShipTarget);
        if (target) {
            if (target->isFullyCloaked() && !target->isAlly(m_Ship)) {
                if (!isShipDetectedByAntiCloak(target)) {
                    m_Ship.setTarget(nullptr, true);
                }
            }
        }
    }
}
void ShipSystemSensors::update(const double& dt) {

    if (m_Ship.IsPlayer() && Engine::isKeyDownOnce(KeyboardKey::I)) {
        toggleAntiCloakScan(true);
    }

    internal_update_anti_cloak_scan_detected_ships(dt);
    internal_update_clear_target_automatically_if_applicable(dt);

    if (m_AntiCloakScanActive) {
        internal_update_anti_cloak_scan(dt);
    }

    internal_update_populate_detected_ships(dt);

    ShipSystem::update(dt);
}
const decimal& ShipSystemSensors::getRadarRange() const {
    return m_RadarRange;
}

DetectedShip ShipSystemSensors::getClosestAlliedShip() {
    DetectedShip ret;
    if (m_DetectedAlliedShips.size() > 0) {
        for (auto& ship : m_DetectedAlliedShips) {
            if (ship.distanceAway2 <= ret.distanceAway2 || ret.distanceAway2 < static_cast<decimal>(0.0)) {
                ret = ship;
            }
        }
    }
    return ret;
}
DetectedShip ShipSystemSensors::getClosestNeutralShip() {
    DetectedShip ret;
    if (m_DetectedNeutralShips.size() > 0) {
        for (auto& ship : m_DetectedNeutralShips) {
            if (ship.distanceAway2 <= ret.distanceAway2 || ret.distanceAway2 < static_cast<decimal>(0.0)) {
                ret = ship;
            }
        }
    }
    return ret;
}
DetectedShip ShipSystemSensors::getClosestEnemyShip() {
    DetectedShip ret;
    if (m_DetectedEnemyShips.size() > 0) {
        for (auto& ship : m_DetectedEnemyShips) {
            if (ship.distanceAway2 <= ret.distanceAway2 || ret.distanceAway2 < static_cast<decimal>(0.0)) {
                ret = ship;
            }
        }
    }
    return ret;
}
DetectedShip ShipSystemSensors::getClosestEnemyCloakedShip() {
    DetectedShip ret;
    if (m_DetectedAntiCloakedShips.size() > 0) {
        for (auto& ship : m_DetectedAntiCloakedShips) {
            const auto dist2 = glm::distance2(ship.ship->getPosition(), m_Ship.getPosition());
            if (dist2 < ret.distanceAway2 || ret.distanceAway2 < static_cast<decimal>(0.0)) {
                ret.distanceAway2 = dist2;
                ret.ship = ship.ship;
            }
        }
    }
    return ret;
}

DetectedShip ShipSystemSensors::getClosestShip() {
    DetectedShip ret;
    if (m_DetectedShips.size() > 0) {
        auto& thisShipPos = m_Ship.getPosition();
        for (auto& ship : m_DetectedShips) {
            if (ship.distanceAway2 <= ret.distanceAway2 || ret.distanceAway2 < static_cast<decimal>(0.0)) {
                ret = ship;
            }
        }
    }
    return ret;
}
vector<DetectedShip>& ShipSystemSensors::getEnemyShips() {
    return m_DetectedEnemyShips;
}
vector<DetectedShip>& ShipSystemSensors::getShips() {
    return m_DetectedShips;
}
vector<DetectedShip>& ShipSystemSensors::getAlliedShips() {
    return m_DetectedAlliedShips;
}
vector<DetectedShip>& ShipSystemSensors::getNeutralShips() {
    return m_DetectedNeutralShips;
}
vector<ShipSystemSensors::AntiCloakDetection>& ShipSystemSensors::getAntiCloakDetectedShips() {
    return m_DetectedAntiCloakedShips;
}
void ShipSystemSensors::render() {

}