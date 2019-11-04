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
#include <glm/gtx/norm.hpp>

using namespace Engine;
using namespace std;

ShipSystemSensors::ShipSystemSensors(Ship& _ship, Map& map, const decimal& range) :ShipSystem(ShipSystemType::Sensors, _ship),m_Map(map){
    m_RadarRange = range;
    m_Target     = nullptr;
}
ShipSystemSensors::~ShipSystemSensors() {

}

EntityWrapper* ShipSystemSensors::getTarget() {
    return m_Target;
}
void ShipSystemSensors::setTarget(EntityWrapper* target, const bool sendPacket) {
    if (!target) {
        if (m_Ship.IsPlayer() && m_Ship.m_PlayerCamera) {
            m_Ship.m_PlayerCamera->setTarget(&m_Ship);
        }
    }
    Ship* ship = dynamic_cast<Ship*>(target);
    if (ship && ship->isFullyCloaked() && !ship->isAlly(m_Ship)) {
        return;
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
    m_Map.getHUD().setTarget(m_Target);
}
void ShipSystemSensors::setTarget(const string& target, const bool sendPacket) {
    if (target.empty()) {
        setTarget(nullptr, sendPacket);
    }
    Map& map = static_cast<Map&>(m_Ship.entity().scene());
    for (auto& entity : map.m_Objects) {
        auto* componentName = entity->getComponent<ComponentName>();
        if (componentName) {
            if (componentName->name() == target) {
                setTarget(entity, sendPacket);
            }
        }
    }
}

const bool ShipSystemSensors::validateDetection(Ship& othership, const glm_vec3& thisShipPos) {
    bool ret = false;
    const auto dist2 = glm::distance2(othership.getPosition(), thisShipPos);
    if (dist2 <= m_RadarRange * m_RadarRange) {
        if (  (!othership.isFullyCloaked() && !othership.isAlly(m_Ship)) || (othership.isAlly(m_Ship))    ) {
            ret = true;
        }
    }
    return ret;
}

void ShipSystemSensors::update(const double& dt) {
    auto* thisShipTarget = m_Ship.getTarget();
    if (thisShipTarget) {
        Ship* target = dynamic_cast<Ship*>(thisShipTarget);
        if (target) {
            if (target->isFullyCloaked() && !target->isAlly(m_Ship)) {
                m_Ship.setTarget(nullptr, true);
            }
        }
    }

    //populate ship list
    m_DetectedShips.clear();
    m_DetectedEnemyShips.clear();
    m_DetectedAlliedShips.clear();
    m_DetectedNeutralShips.clear();
    for (auto& shipItr : m_Map.getShips()) {
        auto& ship = *shipItr.second;
        if (&ship != &m_Ship) {
            const bool res = validateDetection(ship, m_Ship.getPosition());
            if (res) {
                m_DetectedShips.push_back(&ship);

                if (ship.isAlly(m_Ship)) {
                    m_DetectedAlliedShips.push_back(&ship);
                }else if (ship.isEnemy(m_Ship)) {
                    m_DetectedEnemyShips.push_back(&ship);
                }else if (ship.isNeutral(m_Ship)) {
                    m_DetectedNeutralShips.push_back(&ship);
                }
            }
        }
    }
    ShipSystem::update(dt);
}
const decimal& ShipSystemSensors::getRadarRange() const {
    return m_RadarRange;
}

DetectedShip ShipSystemSensors::getClosestAlliedShip() {
    DetectedShip ret;
    if (m_DetectedAlliedShips.size() > 0) {
        auto& thisShipPos = m_Ship.getPosition();
        for (auto& ship : m_DetectedAlliedShips) {
            const auto dist2 = glm::distance2(ship->getPosition(), thisShipPos);
            if (dist2 <= ret.distanceAway2 || ret.distanceAway2 < static_cast<decimal>(0.0)) {
                ret.ship = ship;
                ret.distanceAway2 = dist2;
            }
        }
    }
    return ret;
}
DetectedShip ShipSystemSensors::getClosestNeutralShip() {
    DetectedShip ret;
    if (m_DetectedNeutralShips.size() > 0) {
        auto& thisShipPos = m_Ship.getPosition();
        for (auto& ship : m_DetectedNeutralShips) {
            const auto dist2 = glm::distance2(ship->getPosition(), thisShipPos);
            if (dist2 <= ret.distanceAway2 || ret.distanceAway2 < static_cast<decimal>(0.0)) {
                ret.ship = ship;
                ret.distanceAway2 = dist2;
            }
        }
    }
    return ret;
}
DetectedShip ShipSystemSensors::getClosestEnemyShip() {
    DetectedShip ret;
    if (m_DetectedEnemyShips.size() > 0) {
        auto& thisShipPos = m_Ship.getPosition();
        for (auto& ship : m_DetectedEnemyShips) {
            const auto dist2 = glm::distance2(ship->getPosition(), thisShipPos);
            if (dist2 <= ret.distanceAway2 || ret.distanceAway2 < static_cast<decimal>(0.0)) {
                ret.ship = ship;
                ret.distanceAway2 = dist2;
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
            const auto dist2 = glm::distance2(ship->getPosition(), thisShipPos);
            if (dist2 <= ret.distanceAway2 || ret.distanceAway2 < static_cast<decimal>(0.0)) {
                ret.ship = ship;
                ret.distanceAway2 = dist2;
            }
        }
    }
    return ret;
}
vector<Ship*>& ShipSystemSensors::getEnemyShips() {
    return m_DetectedEnemyShips;
}
vector<Ship*>& ShipSystemSensors::getShips() {
    return m_DetectedShips;
}
vector<Ship*>& ShipSystemSensors::getAlliedShips() {
    return m_DetectedAlliedShips;
}
vector<Ship*>& ShipSystemSensors::getNeutralShips() {
    return m_DetectedNeutralShips;
}

void ShipSystemSensors::render() {

}