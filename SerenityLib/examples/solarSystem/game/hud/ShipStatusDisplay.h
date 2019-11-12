#pragma once
#ifndef GAME_SHIP_STATUS_DISPLAY_H
#define GAME_SHIP_STATUS_DISPLAY_H

#include <ecs/EntityWrapper.h>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

class  Ship;
class  Planet;
class  ShipSystemShields;
class  HUD;
class  Texture;
class ShipStatusDisplay final {
    private:
        HUD&             m_HUD;
        Ship*            m_TargetAsShip;
        EntityWrapper*   m_TargetAsWrapper;
        Entity           m_Target;

        glm::vec2        m_Position;
        glm::vec2        m_Size;
        glm::vec4        m_Color;

        Alignment::Type  m_Alignment;


        const glm::vec4 getPercentColor(const float percent);

        void renderShipHullStatus(const glm::vec2& bottomLeftCorner);
        void renderForwardShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder);
        void renderAftShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder);
        void renderPortShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder);
        void renderStarboardShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder);
        void renderDorsalShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder);
        void renderVentralShieldStatus(const glm::vec2& centerHullIcon, ShipSystemShields& shields, Texture& icon, Texture& iconBorder);

    public:
        ShipStatusDisplay(HUD&, const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const Alignment::Type& alignment);
        ~ShipStatusDisplay();

        void onResize(const unsigned int& width, const unsigned int& height);

        void setTarget(Planet*);
        void setTarget(Ship*);
        void setTarget(EntityWrapper*);
        void setTarget(Entity&);

        void setPosition(const float x, const float y);
        void update(const double& dt);
        void render();
};

#endif