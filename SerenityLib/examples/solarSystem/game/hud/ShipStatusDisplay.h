#pragma once
#ifndef GAME_SHIP_STATUS_DISPLAY_H
#define GAME_SHIP_STATUS_DISPLAY_H

#include <ecs/Entity.h>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

class  Ship;
class  Planet;
class ShipStatusDisplay final {
    private:

        Ship*            m_TargetAsShip;
        EntityWrapper*   m_TargetAsWrapper;
        Entity           m_Target;

        glm::vec2        m_Position;
        glm::vec2        m_Size;
        glm::vec4        m_Color;

        Alignment::Type  m_Alignment;


        void renderShipHullStatus();

    public:
        ShipStatusDisplay(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const Alignment::Type& alignment);
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