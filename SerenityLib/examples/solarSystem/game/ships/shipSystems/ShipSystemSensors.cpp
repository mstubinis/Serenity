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
    const auto is_player = m_Ship.IsPlayer();
    if (!target) {
        if (is_player && m_Ship.m_PlayerCamera) {
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
    if (m_DetectedEnemyShips.size() > 0) {
        for (auto& ship : m_DetectedEnemyShips) {
            if ((ship.ship->isCloaked() /* should we only check for fully cloaked ships? */ || ship.ship->isFullyCloaked()) && ship.distanceAway2 <= ret.distanceAway2 || ret.distanceAway2 < static_cast<decimal>(0.0)) {
                ret = ship;
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

void ShipSystemSensors::render() {

}