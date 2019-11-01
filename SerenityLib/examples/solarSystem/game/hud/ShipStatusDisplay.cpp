#include "ShipStatusDisplay.h"

#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/fonts/Font.h>

#include "../Ship.h"
#include "../Planet.h"
#include "../ships/Ships.h"
#include "../ResourceManifest.h"
#include "../hud/HUD.h"

#include "../ships/shipSystems/ShipSystemHull.h"
#include "../ships/shipSystems/ShipSystemShields.h"

using namespace Engine;
using namespace std;

ShipStatusDisplay::ShipStatusDisplay(HUD& hud, const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const Alignment::Type& alignment):m_HUD(hud) {
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
const glm::vec4 ShipStatusDisplay::getPercentColor(const float percent) {
    auto green = percent * 2.0f; //0 to 2
    auto red = 2.0f - green;

    green = glm::clamp(green, 0.0f, 1.0f);
    red = glm::clamp(red, 0.0f, 1.0f);
    return glm::vec4(red, green, 0.0f, 1.0f);
}


void ShipStatusDisplay::renderShipHullStatus(const glm::vec2& bottomLeftCorner) {
    Texture& texture = *((Texture*)(Ships::Database.at(m_TargetAsShip->getClass()).IconTextureHandle).get());
    Texture& textureBorder = *((Texture*)(Ships::Database.at(m_TargetAsShip->getClass()).IconBorderTextureHandle).get());

    auto* hull = static_cast<ShipSystemHull*>(m_TargetAsShip->getShipSystem(ShipSystemType::Hull));

    const auto pos = bottomLeftCorner + glm::vec2((m_Size.y / 2.0f) - (27.0f / 2.0f));

    Renderer::renderTexture(textureBorder, pos, m_Color, 180, glm::vec2(1.0f), 0.15f, Alignment::Center); //border

    const auto percent = hull->getHealthPercent();

    //scissor
    const auto scissor = glm::vec4(pos.x - texture.width() / 2.0f, pos.y - texture.height() / 2.0f, texture.width(), texture.height() * percent);
    Renderer::renderTexture(texture, pos, getPercentColor(percent), 180, glm::vec2(1.0f), 0.16f, Alignment::Center, scissor); //hull
}
void ShipStatusDisplay::renderForwardShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder) {
    const auto percent = shields.getHealthPercentForward();
    const auto pos = centerHullIcon + glm::vec2(0, 50);

    //scissor
    const auto width_with_percent = 74.0f * percent; //the graphic is 74.0f pixels wide
    const auto scissor = glm::vec4(pos.x - (width_with_percent / 2.0f), pos.y, width_with_percent, icon.height());

    Renderer::renderTexture(iconBorder, pos, m_Color, 180, glm::vec2(1.0f), 0.15f, Alignment::BottomCenter); //border
    Renderer::renderTexture(icon, pos, getPercentColor(percent), 180, glm::vec2(1.0f), 0.16f, Alignment::BottomCenter, scissor); //shield
}
void ShipStatusDisplay::renderAftShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder) {
    const auto percent = shields.getHealthPercentAft();

    const auto pos = centerHullIcon - glm::vec2(0, 50);

    //scissor
    const auto width_with_percent = 74.0f * percent; //the graphic is 74.0f pixels wide
    const auto scissor = glm::vec4(pos.x - (width_with_percent / 2.0f), pos.y - icon.height(), width_with_percent, icon.height());

    Renderer::renderTexture(iconBorder, pos, m_Color, 0, glm::vec2(1.0f), 0.15f, Alignment::TopCenter); //border
    Renderer::renderTexture(icon, pos, getPercentColor(percent), 0, glm::vec2(1.0f), 0.16f, Alignment::TopCenter, scissor); //shield
}
void ShipStatusDisplay::renderPortShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder) {
    const auto percent = shields.getHealthPercentPort();
    const auto pos = centerHullIcon - glm::vec2(70, 0);
    //scissor
    const auto width_with_percent = 74.0f * percent; //the graphic is 74.0f pixels wide
    const auto scissor = glm::vec4(pos.x - icon.height() / 2.0f, pos.y - (width_with_percent / 2.0f), icon.height(), width_with_percent);

    Renderer::renderTexture(iconBorder, pos, m_Color, 270, glm::vec2(1.0f), 0.15f, Alignment::Center); //border
    Renderer::renderTexture(icon, pos, getPercentColor(percent), 270, glm::vec2(1.0f), 0.16f, Alignment::Center, scissor); //shield
}
void ShipStatusDisplay::renderStarboardShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder) {
    const auto percent = shields.getHealthPercentStarboard();
    const auto pos = centerHullIcon + glm::vec2(70, 0);
    //scissor
    const auto width_with_percent = 74.0f * percent; //the graphic is 74.0f pixels wide
    const auto scissor = glm::vec4(pos.x - icon.height() / 2.0f, pos.y - (width_with_percent / 2.0f), icon.height(), width_with_percent);

    Renderer::renderTexture(iconBorder, pos, m_Color, 90, glm::vec2(1.0f), 0.15f, Alignment::Center); //border
    Renderer::renderTexture(icon, pos, getPercentColor(percent), 90, glm::vec2(1.0f), 0.16f, Alignment::Center, scissor); //shield
}
void ShipStatusDisplay::renderDorsalShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder) {
    const auto percent = shields.getHealthPercentDorsal();

    const auto pos = centerHullIcon + glm::vec2(100 + (icon.width() / 2) + 2, 74);

    //scissor
    const auto width_with_percent = 64.0f * percent; //the graphic is 64.0f pixels wide
    const auto scissor = glm::vec4(pos.x - (width_with_percent / 2.0f), pos.y - icon.height() / 2.0f, width_with_percent, icon.height());

    Renderer::renderTexture(iconBorder, pos, m_Color, 0, glm::vec2(1.0f), 0.15f, Alignment::Center); //border
    Renderer::renderTexture(icon, pos, getPercentColor(percent), 0, glm::vec2(1.0f), 0.16f, Alignment::Center, scissor); //shield
}
void ShipStatusDisplay::renderVentralShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder) {
    const auto percent = shields.getHealthPercentVentral();

    const auto pos = centerHullIcon + glm::vec2(100 + (icon.width() / 2) + 2, 74 - (icon.height() + 5));

    //scissor
    const auto width_with_percent = 64.0f * percent; //the graphic is 64.0f pixels wide
    const auto scissor = glm::vec4(pos.x - (width_with_percent / 2.0f), pos.y - icon.height() / 2.0f, width_with_percent, icon.height());

    Renderer::renderTexture(iconBorder, pos, m_Color, 0, glm::vec2(1.0f), 0.15f, Alignment::Center); //border
    Renderer::renderTexture(icon, pos, getPercentColor(percent), 0, glm::vec2(1.0f), 0.16f, Alignment::Center, scissor); //shield
}
void ShipStatusDisplay::render() {
    glm::vec2 bottomLeftCorner;
    if (m_Alignment == Alignment::BottomRight) {
        bottomLeftCorner = glm::vec2(m_Position.x - m_Size.x - 6, m_Position.y);
    }else if (m_Alignment == Alignment::BottomLeft) {
        bottomLeftCorner = glm::vec2(m_Position.x + 6, m_Position.y);
    }
    Texture& background = *(Texture*)((ResourceManifest::ShipStatusBackgroundHUDTexture).get());
    Texture& backgroundBorder = *(Texture*)((ResourceManifest::ShipStatusBackgroundBorderHUDTexture).get());
    Renderer::renderTexture(background, bottomLeftCorner, glm::vec4(1.0f), 180, glm::vec2(1.0f), 0.17f, Alignment::BottomLeft);
    Renderer::renderTexture(backgroundBorder, bottomLeftCorner, m_Color, 180, glm::vec2(1.0f), 0.169f, Alignment::BottomLeft);

    const auto centerHullIcon = bottomLeftCorner + glm::vec2((m_Size.y / 2.0f) - (27.0f / 2.0f));
    if (m_TargetAsShip) {
        renderShipHullStatus(bottomLeftCorner);
        auto* shields_ptr = static_cast<ShipSystemShields*>(m_TargetAsShip->getShipSystem(ShipSystemType::Shields));
        if (shields_ptr) {
            auto& shields = *shields_ptr;

            Texture& texture = *((Texture*)(ResourceManifest::ShieldRingHUDTexture).get());
            Texture& textureBorder = *((Texture*)(ResourceManifest::ShieldRingBorderHUDTexture).get());
            Texture& texture2 = *((Texture*)(ResourceManifest::ShieldRing2HUDTexture).get());
            Texture& textureBorder2 = *((Texture*)(ResourceManifest::ShieldRingBorder2HUDTexture).get());

            renderForwardShieldStatus(centerHullIcon, shields, texture, textureBorder);
            renderAftShieldStatus(centerHullIcon, shields, texture, textureBorder);
            renderPortShieldStatus(centerHullIcon, shields, texture, textureBorder);
            renderStarboardShieldStatus(centerHullIcon, shields, texture, textureBorder);
            renderDorsalShieldStatus(centerHullIcon, shields, texture2, textureBorder2);
            renderVentralShieldStatus(centerHullIcon, shields, texture2, textureBorder2);
        }
        //finally, render name & ship class
        Renderer::renderText(
            m_TargetAsShip->getName() + " - " + m_TargetAsShip->getClass(),
            m_HUD.getFont(),
            bottomLeftCorner + glm::vec2(m_Size.x / 2, m_Size.y - 4),
            m_Color, 
            0,
            glm::vec2(0.6f),
            0.168f,
            TextAlignment::Center
        );
    }
}