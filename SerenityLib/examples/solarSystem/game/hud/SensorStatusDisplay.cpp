#include "SensorStatusDisplay.h"
#include "../ships/shipSystems/ShipSystemSensors.h"
#include "../ships/shipSystems/ShipSystemCloakingDevice.h"

#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/scene/Camera.h>
#include "../Ship.h"
#include "../map/Map.h"
#include "../ResourceManifest.h"
#include "../Core.h"
#include "../networking/Client.h"
#include "../Menu.h"
#include "../Helper.h"
#include "../hud/HUD.h"

using namespace std;
using namespace Engine;

SensorStatusDisplay::SensorStatusDisplay(HUD& hud, Map& map, const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color, const Alignment::Type& alignment):m_HUD(hud),m_Map(map) {
    m_Ship      = nullptr;
    m_Sensors   = nullptr;
    setPosition(pos.x, pos.y);
    m_Size      = size;
    m_Color     = color;
    m_Alignment = alignment;

    m_Camera = new Camera(-1.0f, 1.0f, -1.0f, 1.0f, 0.0005f, 100.0f, &m_Map);

    m_ViewportObject = &m_Map.addViewport(pos.x - (size.x / 2.0f), pos.y + (size.y - size.x), size.x, size.x, *m_Camera);
    m_Viewport = glm::vec4(pos.x - (size.x / 2.0f), pos.y + (size.y - size.x), size.x, size.x);

    m_ViewportObject->activate();
    m_ViewportObject->removeRenderFlag(ViewportRenderingFlag::API2D);
    //m_ViewportObject->setSkyboxVisible(false);
    m_ViewportObject->activateDepthMask();
    m_ViewportObject->setDepthMaskValue(99.8f);
    m_ViewportObject->removeRenderFlag(ViewportRenderingFlag::GodRays);

    m_RadarRingEntity = map.createEntity();
    m_RadarRingEntity.addComponent<ComponentBody>();
    auto& radarModel = *m_RadarRingEntity.addComponent<ComponentModel>(ResourceManifest::RadarDiscMesh, Material::WhiteShadeless, ShaderProgram::Deferred, RenderStage::GeometryOpaque);
    auto& radarModelInstance = radarModel.getModel();
    radarModelInstance.setColor(1, 1, 0, 1);
    radarModelInstance.setViewportFlag(ViewportFlag::_2); //only render in the 2nd viewport
    radarModelInstance.setScale(0.95f);
}
SensorStatusDisplay::~SensorStatusDisplay() {

}
const Entity& SensorStatusDisplay::radarRingEntity() const {
    return m_RadarRingEntity;
}
const Entity& SensorStatusDisplay::radarCameraEntity() const {
    return m_Camera->entity();
}
void SensorStatusDisplay::onResize(const unsigned int& width, const unsigned int& height) {
    m_Position.x = static_cast<float>(width) / 2.0f;

    m_ViewportObject->setViewportDimensions(m_Position.x - (m_Size.x / 2.0f), m_Position.y +(m_Size.y - m_Size.x), m_Size.x, m_Size.x);
    m_Viewport = glm::vec4(m_Position.x - (m_Size.x / 2.0f), m_Position.y +(m_Size.y - m_Size.x), m_Size.x, m_Size.x);
}
void SensorStatusDisplay::setShip(Ship* ship) {
    if (!ship) {
        m_Ship = m_Map.getPlayer();
        m_Sensors = static_cast<ShipSystemSensors*>(m_Map.getPlayer()->getShipSystem(ShipSystemType::Sensors));
        return;
    }
    m_Ship = ship;
    m_Sensors = static_cast<ShipSystemSensors*>(ship->getShipSystem(ShipSystemType::Sensors));
}
void SensorStatusDisplay::setPosition(const float x, const float y) {
    m_Position.x = x;
    m_Position.y = y;
}
const glm::vec2& SensorStatusDisplay::size() const {
    return m_Size;
}
void SensorStatusDisplay::update(const double& dt) {
    Ship& ship = *m_Ship;
    auto& thisShipPos = ship.getPosition();
    auto& camBody = *m_Camera->getComponent<ComponentBody>();
    const auto shipForward = ship.forward();
    const auto extendedForward = shipForward * static_cast<decimal>(1000.0);
    camBody.setPosition(thisShipPos - extendedForward);
    camBody.setRotation(ship.getRotation());
    const auto camPos = camBody.position();
    m_Camera->lookAt(camPos, camPos - extendedForward, ship.up());

    auto& radarBody = *m_RadarRingEntity.getComponent<ComponentBody>();
    auto& radarModel = *m_RadarRingEntity.getComponent<ComponentModel>();
    radarBody.setPosition(camPos - (shipForward * glm_vec3(2.05)));

    if (m_Map.getHUD().isHidden()) {
        radarModel.hide();
    }else{
        radarModel.show();
    }
}
void SensorStatusDisplay::render() {
    if (!m_Ship)
        return;
    Ship& ship = *m_Ship;
    const auto& radarRange = m_Sensors->getRadarRange();
    const auto& winSize = Resources::getWindowSize();

    auto& radarMat = *(Material*)ResourceManifest::RadarMaterial.get();
    auto& radarEdgeMat = *(Material*)ResourceManifest::RadarEdgeMaterial.get();
    auto& radarTokenMat = *(Material*)ResourceManifest::RadarTokenMaterial.get();

    const auto& radarTexture = *radarMat.getComponent(0).texture();
    const auto& radarEdgeTexture = *radarEdgeMat.getComponent(0).texture();
    const auto& radarTokenTexture = *radarTokenMat.getComponent(0).texture();
    //render radar 2d graphic
    Renderer::renderTexture(radarTexture, m_Position, glm::vec4(1, 1, 1, 1), 180.0f, glm::vec2(1.0f), 0.2f, Alignment::BottomCenter);
    //render radar edge
    Renderer::renderTexture(radarEdgeTexture, m_Position, glm::vec4(0.11f, 0.16f, 0.19f, 1), 180.0f, glm::vec2(1.0f), 0.17f, Alignment::BottomCenter);

    const auto& radarBodyPosition = m_RadarRingEntity.getComponent<ComponentBody>()->position();
    const auto& myPos = ship.getComponent<ComponentBody>()->position();
    auto* myTarget = ship.getTarget();


    //radar anti cloak bar
    auto& anti_cloak_bar_bg_mat = *(Material*)ResourceManifest::RadarAntiCloakBarBackgroundMaterial.get();
    auto& anti_cloak_bar_mat = *(Material*)ResourceManifest::RadarAntiCloakBarMaterial.get();
    const auto& anti_cloak_bar_bg = *anti_cloak_bar_bg_mat.getComponent(0).texture();
    const auto& anti_cloak_bar = *anti_cloak_bar_mat.getComponent(0).texture();
    const auto height_above_bottom = 20;
    Renderer::renderTexture(anti_cloak_bar_bg, glm::vec2(m_Position.x - (anti_cloak_bar_bg.size().x / 2), m_Position.y + height_above_bottom), glm::vec4(1, 1, 1, 1), 180.0f, glm::vec2(1.0f), 0.19f, Alignment::BottomLeft);
    const auto anti_cloak_bar_width = (m_Sensors->getAntiCloakingScanTimer() / m_Sensors->getAntiCloakingScanTimerMax()) * anti_cloak_bar.width();
    glm::vec4 scissor = glm::vec4(m_Position.x - (anti_cloak_bar_bg.size().x / 2), m_Position.y + height_above_bottom, anti_cloak_bar_width, anti_cloak_bar.height());
    Renderer::renderTexture(anti_cloak_bar, glm::vec2(m_Position.x - (anti_cloak_bar_bg.size().x / 2), m_Position.y + height_above_bottom), glm::vec4(1, 1, 1, 1), 180.0f, glm::vec2(1.0f), 0.19f, Alignment::BottomLeft, scissor);

    auto lambda = [&](Ship& other_ship) {
        glm::vec4 color;
        if (ship.isEnemy(other_ship)) {
            color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); //red (enemy)
        }else if (ship.isAlly(other_ship)) {
            color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); //green (ally)
        }else {
            color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); //yellow (neutral)
        }
        auto otherVector = other_ship.getPosition() - myPos;
        //scale otherPos down to the range
        const auto otherLen = glm::length(otherVector);
        otherVector /= otherLen;
        otherVector *= glm::min((otherLen / 1.0), static_cast<decimal>(1.0));
        otherVector = radarBodyPosition + otherVector;

        const auto pos = Math::getScreenCoordinates(otherVector, *m_Camera, m_Viewport, false);

        const auto pos2D = glm::vec2(pos.x, pos.y);
        const auto dotproduct = glm::dot(radarBodyPosition + ship.forward(), otherVector - radarBodyPosition);
        if (dotproduct <= 0 /*&& !modByCloak*/) {
            color.a = 0.5f;
        }
        //render radar token
        Renderer::renderTexture(radarTokenTexture, pos2D, color, 0, glm::vec2(1.0f), 0.14f, Alignment::Center);

        if (myTarget) {
            Ship* myTargetAsShip = dynamic_cast<Ship*>(myTarget);
            if (myTargetAsShip && myTargetAsShip->getMapKey() == other_ship.getMapKey()) {
                Renderer::renderTexture(radarTokenTexture, pos2D, glm::vec4(1, 1, 1, 1), 0, glm::vec2(1.2f), 0.16f, Alignment::Center);
            }//else{
             //   if (myTarget->getComponent<ComponentName>()->name() == other_ship.getName()) {
             //       Renderer::renderTexture(radarTokenTexture, pos2D, glm::vec4(1, 1, 1, 1), 0, glm::vec2(1.2f), 0.16f, Alignment::Center);
             //   }
            //}
        }
    };
    if (!m_Ship->isFullyDestroyed()) {
        for (auto& other_ship_ptr : m_Sensors->getShips()) {
            Ship& other_ship = *other_ship_ptr.ship;
            lambda(other_ship);
        }
        for (auto& other_ship_ptr : m_Sensors->getAntiCloakDetectedShips()) {
            Ship& other_ship = *other_ship_ptr.ship;
            lambda(other_ship);
        }
    }
}