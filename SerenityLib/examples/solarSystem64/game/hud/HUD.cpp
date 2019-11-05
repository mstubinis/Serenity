#include "HUD.h"

#include "../map/Map.h"
#include "../ResourceManifest.h"

#include "../hud/SensorStatusDisplay.h"
#include "../hud/ShipStatusDisplay.h"

#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/materials/MaterialLayer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/fonts/Font.h>

using namespace std;
using namespace Engine;


HUD::HUD(Map& map, Font& font):m_Map(map), m_Font(font){
    const auto& winSize = Resources::getWindowSize();

    auto& radarMat = *(Material*)ResourceManifest::RadarMaterial.get();
    const auto& radarTexture = *radarMat.getComponent(0).texture();
    const auto textureWidth = radarTexture.width();

    Texture& background = *(Texture*)((ResourceManifest::ShipStatusBackgroundHUDTexture).get());

    m_SensorDisplay = new SensorStatusDisplay(*this, map, glm::vec2(winSize.x / 2.0f, 0), glm::vec2(textureWidth, radarTexture.height()), glm::vec4(1, 1, 0, 1), Alignment::BottomCenter);
    m_ShipStatusDisplay = new ShipStatusDisplay(*this, glm::vec2((winSize.x / 2.0f) - textureWidth / 2.0f, 0), background.size(), glm::vec4(1,1,0,1), Alignment::BottomRight);
    m_ShipTargetStatusDisplay = new ShipStatusDisplay(*this, glm::vec2((winSize.x / 2.0f) + textureWidth / 2.0f, 0), background.size(), glm::vec4(1, 1, 0, 1), Alignment::BottomLeft);
}
HUD::~HUD() {
    SAFE_DELETE(m_SensorDisplay);
    SAFE_DELETE(m_ShipStatusDisplay);
    SAFE_DELETE(m_ShipTargetStatusDisplay);
}
Font& HUD::getFont() {
    return m_Font;
}
SensorStatusDisplay& HUD::getSensorDisplay() {
    return *m_SensorDisplay;
}
ShipStatusDisplay& HUD::getShipStatusDisplay() {
    return *m_ShipStatusDisplay;
}
ShipStatusDisplay& HUD::getShipTargetStatusDisplay() {
    return *m_ShipTargetStatusDisplay;
}
void HUD::onResize(const unsigned int& width, const unsigned int& height) {
    m_SensorDisplay->onResize(width, height);
    m_ShipStatusDisplay->setPosition((width / 2.0f) - (m_SensorDisplay->size().x / 2.0f), 0);
    m_ShipTargetStatusDisplay->setPosition((width / 2.0f) + (m_SensorDisplay->size().x / 2.0f), 0);
}
void HUD::setTarget(Planet* planet) {
    m_ShipTargetStatusDisplay->setTarget(planet);
}
void HUD::setTarget(Ship* ship) {
    m_ShipTargetStatusDisplay->setTarget(ship);
}
void HUD::setTarget(EntityWrapper* wrapper) {
    m_ShipTargetStatusDisplay->setTarget(wrapper);
}
void HUD::setTarget(Entity& entity) {
    m_ShipTargetStatusDisplay->setTarget(entity);
}
void HUD::update(const double& dt) {
    m_SensorDisplay->update(dt);
    m_ShipStatusDisplay->update(dt);
    m_ShipTargetStatusDisplay->update(dt);
}
void HUD::render() {
    m_SensorDisplay->render();
    m_ShipStatusDisplay->render();
    m_ShipTargetStatusDisplay->render();
}