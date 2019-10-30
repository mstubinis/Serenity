#include "ShipSystemSensors.h"
#include "ShipSystemCloakingDevice.h"
#include "../../Ship.h"
#include "../../ResourceManifest.h"
#include "../../map/Map.h"
#include "../../Helper.h"
#include "../../Core.h"
#include "../../HUD.h"
#include "../../GameCamera.h"
#include "../../Packet.h"

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
    auto& radarMat = *(Material*)ResourceManifest::RadarMaterial.get();
    const auto& radarTexture = *radarMat.getComponent(0).texture();
    const auto& winSize = Resources::getWindowSize();

    const auto halfWinWidth = winSize.x / 2.0f;
    const auto halfTextureWidth = radarTexture.width() / 2.0f;


    const float aspect = static_cast<float>(winSize.x) / static_cast<float>(winSize.y);
    m_Camera = new Camera(-1.0f, 1.0f, -1.0f, 1.0f, 0.0005f, 100.0f, &m_Map);


    m_ViewportObject = &m_Map.addViewport(halfWinWidth - halfTextureWidth, 0, radarTexture.width(), radarTexture.height(), *m_Camera);
    m_Viewport = glm::vec4(halfWinWidth - halfTextureWidth, 0, radarTexture.width(), radarTexture.height());

    m_ViewportObject->activate();
    m_ViewportObject->removeRenderFlag(ViewportRenderingFlag::API2D);
    //m_ViewportObject->setSkyboxVisible(false);
    m_ViewportObject->activateDepthMask();
    m_ViewportObject->setDepthMaskValue(99.8f);
    m_ViewportObject->removeRenderFlag(ViewportRenderingFlag::GodRays);

    m_RadarRingEntity = map.createEntity();
    m_RadarRingEntity.addComponent<ComponentBody>();
    auto& radarModel = *m_RadarRingEntity.addComponent<ComponentModel>(ResourceManifest::RadarDiscMesh,Material::WhiteShadeless,ShaderProgram::Deferred,RenderStage::GeometryOpaque);
    radarModel.getModel().setColor(1, 1, 0, 1);
    radarModel.getModel().setViewportFlag(ViewportFlag::_2);
    radarModel.getModel().setScale(0.95f);

    m_RadarRange = range;
    m_Target = nullptr;
}
ShipSystemSensors::~ShipSystemSensors() {

}
const Entity& ShipSystemSensors::radarRingEntity() const {
    return m_RadarRingEntity;
}
const Entity& ShipSystemSensors::radarCameraEntity() const {
    return m_Camera->entity();
}
void ShipSystemSensors::onResize(const uint& width, const uint& height) {
    auto& radarMat = *(Material*)ResourceManifest::RadarMaterial.get();
    const auto& radarTexture = *radarMat.getComponent(0).texture();

    const auto halfWinWidth = width / 2.0f;
    const auto halfTextureWidth = radarTexture.width() / 2.0f;

    m_ViewportObject->setViewportDimensions(halfWinWidth - halfTextureWidth, 0, radarTexture.width(), radarTexture.height());
    m_Viewport = glm::vec4(halfWinWidth - halfTextureWidth, 0, radarTexture.width(), radarTexture.height());
}
EntityWrapper* ShipSystemSensors::getTarget() {
    return m_Target;
}
void ShipSystemSensors::setTarget(EntityWrapper* target, const bool sendPacket) {
    if (!target) {
        if (m_Ship.m_IsPlayer && m_Ship.m_PlayerCamera) {
            m_Ship.m_PlayerCamera->follow(&m_Ship);
        }
    }
    Ship* ship = dynamic_cast<Ship*>(target);
    if (ship && ship->isFullyCloaked()) {
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
}
void ShipSystemSensors::setTarget(const string& target, const bool sendPacket) {
    if (target.empty()) {
        m_Ship.setTarget(nullptr, sendPacket);
    }
    Map& map = static_cast<Map&>(m_Ship.entity().scene());
    for (auto& entity : map.m_Objects) {
        auto* componentName = entity->getComponent<ComponentName>();
        if (componentName) {
            if (componentName->name() == target) {
                m_Ship.setTarget(entity, sendPacket);
            }
        }
    }
}

const bool ShipSystemSensors::validateDetection(Ship& othership, const glm_vec3& thisShipPos) {
    bool ret = false;
    const auto dist2 = glm::distance2(othership.getPosition(), thisShipPos);
    if (dist2 <= m_RadarRange * m_RadarRange) {
        if (!othership.isFullyCloaked()) {
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
            if (target->isFullyCloaked()) {
                m_Ship.setTarget(nullptr, true);
            }
        }
    }

    auto& thisShipPos = m_Ship.getPosition();
    auto& camBody = *m_Camera->getComponent<ComponentBody>();
    const auto shipForward = m_Ship.forward();
    const auto extendedForward = shipForward * static_cast<decimal>(100000.0);
    camBody.setPosition(thisShipPos - extendedForward);
    camBody.setRotation(m_Ship.getRotation());
    const auto camPos = camBody.position();
    m_Camera->lookAt(camPos, camPos - extendedForward, m_Ship.up());

    auto& radarBody = *m_RadarRingEntity.getComponent<ComponentBody>();
    auto& radarModel = *m_RadarRingEntity.getComponent<ComponentModel>();
    radarBody.setPosition(camPos - (shipForward * glm_vec3(2.05)));

    ShipSystem::update(dt);

    if (!m_Ship.m_Client.m_Core.m_HUD->isActive()) {
        radarModel.hide();
    }else{
        radarModel.show();
    }

    //populate ship list
    m_DetectedShips.clear();
    m_DetectedEnemyShips.clear();
    m_DetectedAlliedShips.clear();
    m_DetectedNeutralShips.clear();
    for (auto& shipItr : m_Map.getShips()) {
        auto& ship = *shipItr.second;
        if (&ship != &m_Ship) {
            bool res = validateDetection(ship, thisShipPos);
            if (res) {
                //TODO: place ship in enemy / ally / neutral position via teams
                m_DetectedShips.push_back(&ship);
            }
        }
    }
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
    if (!m_Ship.m_Client.m_Core.m_HUD->isActive()) {
        return;
    }

    const auto& winSize = Resources::getWindowSize();

    auto& radarMat = *(Material*)ResourceManifest::RadarMaterial.get();
    auto& radarEdgeMat = *(Material*)ResourceManifest::RadarEdgeMaterial.get();
    auto& radarTokenMat = *(Material*)ResourceManifest::RadarTokenMaterial.get();

    const auto& radarTexture      = *radarMat.getComponent(0).texture();
    const auto& radarEdgeTexture  = *radarEdgeMat.getComponent(0).texture();
    const auto& radarTokenTexture = *radarTokenMat.getComponent(0).texture();
    auto radarPos = glm::vec2(winSize.x / 2, 0);
    //render radar 2d graphic
    Renderer::renderTexture(radarTexture, radarPos,glm::vec4(1, 1, 1, 1), 0, glm::vec2(1.0f), 0.2f, Alignment::BottomCenter);
    //render radar edge
    Renderer::renderTexture(radarEdgeTexture, glm::vec2(winSize.x / 2, 0), glm::vec4(0.11f, 0.16f, 0.19f, 1), 0, glm::vec2(1.0f), 0.17f, Alignment::BottomCenter);

    const auto& radarBodyPosition = m_RadarRingEntity.getComponent<ComponentBody>()->position();
    const auto& myPos = m_Ship.getComponent<ComponentBody>()->position();
    auto* myTarget = m_Ship.getTarget();
    for (auto& ship : m_DetectedShips) {
        auto color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); //red first (enemy)
        auto otherVector = ship->getPosition() - myPos;
        auto* cloakingDevice = static_cast<ShipSystemCloakingDevice*>(ship->getShipSystem(ShipSystemType::CloakingDevice));
        auto modByCloak = false;
        if (cloakingDevice) {
            auto timer = cloakingDevice->getCloakTimer();
            //1.0 - see, 0.0 or below is invisible
            if (timer < 1.0f) {
                timer = glm::max(0.0f, timer);
                auto invTimer = 1.0f - timer;
                const auto _small = m_RadarRange * 0.03f;
                const auto _large = m_RadarRange * 0.22f;
                auto randX2 = Helper::GetRandomFloatFromTo(-_small, _small);
                auto randY2 = Helper::GetRandomFloatFromTo(-_small, _small);
                auto randZ2 = Helper::GetRandomFloatFromTo(-_small, _small);
                auto randX = Helper::GetRandomFloatFromTo((-_large + randX2) * invTimer, (_large + randX2) * invTimer);
                auto randY = Helper::GetRandomFloatFromTo((-_large + randY2) * invTimer, (_large + randY2) * invTimer);
                auto randZ = Helper::GetRandomFloatFromTo((-_large + randZ2) * invTimer, (_large + randZ2) * invTimer);
                color.r = 1.0f; //cannot tell if cloaked vessel is enemy or not
                color.g = 1.0f;
                color.b = 0.0f;
                color.a = glm::max(0.35f, 1.0f * timer);
                modByCloak = true;
                otherVector += glm_vec3(randX, randY, randZ) * ((glm::length(otherVector) / m_RadarRange) + 0.01f);
            }
        }
            
        //scale otherPos down to the range
        const auto otherLen = glm::length(otherVector);
        otherVector /= otherLen;
        otherVector *= glm::min((otherLen / m_RadarRange + static_cast<decimal>(0.01)), static_cast<decimal>(1.0));
        otherVector = radarBodyPosition + otherVector;

        const auto pos = Math::getScreenCoordinates(otherVector, *m_Camera, m_Viewport, false);

        const auto pos2D = glm::vec2(pos.x, pos.y);
        const auto dotproduct = glm::dot(radarBodyPosition + m_Ship.forward(), otherVector - radarBodyPosition);
        if (dotproduct <= 0 /*&& !modByCloak*/) {
            color.a = 0.5f;
        }
        //render radar token
        Renderer::renderTexture(radarTokenTexture, pos2D, color, 0, glm::vec2(1.0f), 0.14f, Alignment::Center);

        if (myTarget && myTarget->getComponent<ComponentName>()->name() == ship->getName()) {
            Renderer::renderTexture(radarTokenTexture, pos2D, glm::vec4(1, 1, 1, 1), 0, glm::vec2(1.2f), 0.16f, Alignment::Center);
        }
    }
}