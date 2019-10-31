#include "ShipStatusDisplay.h"

#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/textures/Texture.h>
#include "../Ship.h"
#include "../Planet.h"
#include "../ships/Ships.h"

#include "../ships/shipSystems/ShipSystemHull.h"
#include "../ships/shipSystems/ShipSystemShields.h"

using namespace Engine;
using namespace std;

ShipStatusDisplay::ShipStatusDisplay(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const Alignment::Type& alignment) {
    m_TargetAsShip      = nullptr;
    m_TargetAsWrapper   = nullptr;
    m_Target            = Entity::_null;

    //330 x 256
    setPosition(position.x, position.y);
    m_Size      = size;
    m_Color     = color;
    m_Alignment = alignment;
}
ShipStatusDisplay::~ShipStatusDisplay() {

}
void ShipStatusDisplay::setTarget(Planet* planet) {
    m_TargetAsShip    = nullptr;
    m_TargetAsWrapper = planet;
    m_Target          = planet->entity();
}
void ShipStatusDisplay::setTarget(Ship* ship) {
    m_TargetAsShip    = ship;
    m_TargetAsWrapper = ship;
    m_Target          = ship->entity();
}
void ShipStatusDisplay::setTarget(EntityWrapper* wrapper) {
    Ship* ship = dynamic_cast<Ship*>(wrapper);
    //TODO: test if we need these seperate assignments (dynamic_cast should be nullptr if it fails?)
    if (ship) {
        m_TargetAsShip = ship;
    }else{
        m_TargetAsShip = nullptr;
    }
    m_TargetAsWrapper = wrapper;
    m_Target = wrapper->entity();
}
void ShipStatusDisplay::setTarget(Entity& entity) {
    m_TargetAsShip    = nullptr;
    m_TargetAsWrapper = nullptr;
    m_Target          = entity;
}
void ShipStatusDisplay::setPosition(const float x, const float y) {
    m_Position.x = x;
    m_Position.y = y;
}
void ShipStatusDisplay::onResize(const unsigned int& width, const unsigned int& height) {

}
void ShipStatusDisplay::update(const double& dt) {

}

void ShipStatusDisplay::renderShipHullStatus() {
    Texture& texture = *((Texture*)(Ships::Database.at(m_TargetAsShip->getClass()).IconTextureHandle).get());
    Texture& textureBorder = *((Texture*)(Ships::Database.at(m_TargetAsShip->getClass()).IconBorderTextureHandle).get());

    glm::vec2 bottomLeftCorner;
    if (m_Alignment == Alignment::BottomRight) {
        bottomLeftCorner = glm::vec2(m_Position.x - m_Size.x, m_Position.y);
    }else if (m_Alignment == Alignment::BottomLeft) {
        bottomLeftCorner = glm::vec2(m_Position.x, m_Position.y);
    }

    auto* hull = static_cast<ShipSystemHull*>(m_TargetAsShip->getShipSystem(ShipSystemType::Hull));

    const auto padding = glm::vec2(75.0f);
    const auto bottomLeftCornerIcon = bottomLeftCorner + padding;
    const auto wantedSize = m_Size.y - (padding.y * 2.0f);
    const auto wantedScale = wantedSize / texture.height();

    Renderer::renderRectangle(bottomLeftCorner, glm::vec4(0.11f, 0.16f, 0.19f, 1), m_Size.x, m_Size.y, 0, 0.17f, Alignment::BottomLeft);
    Renderer::renderTexture(textureBorder, bottomLeftCornerIcon, m_Color, 0, glm::vec2(wantedScale), 0.15f, Alignment::BottomLeft); //border

    auto percent = hull->getHealthPercent();
    auto green = percent * 2.0f; //0 to 2
    auto red = 2.0f - green;

    green = glm::clamp(green, 0.0f, 1.0f);
    red = glm::clamp(red, 0.0f, 1.0f);
    glm::vec4 hullColor = glm::vec4(red, green, 0.0f, 1.0f);

    //scissor
    const auto scissor = glm::vec4(bottomLeftCornerIcon.x, bottomLeftCornerIcon.y, wantedSize, wantedSize * percent);
    Renderer::renderTexture(texture, bottomLeftCornerIcon, hullColor, 0, glm::vec2(wantedScale), 0.16f, Alignment::BottomLeft, scissor); //hull
}

void ShipStatusDisplay::render() {
    if (m_TargetAsShip) {
        renderShipHullStatus();
    }
}