#pragma once
#ifndef GAME_GUI_SHIP_DESCRIPTION_H
#define GAME_GUI_SHIP_DESCRIPTION_H

class Font;
class ScrollFrame;
class Text;

#include <string>
#include <glm/vec2.hpp>

class ShipDescription final {
    private:
        Font&          m_Font;
        Text*          m_Text;
        ScrollFrame*   m_ScrollFrame;
        std::string    m_ShipClass;
    public:
        ShipDescription(Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth);
        ~ShipDescription();

        const bool setShipClass(const std::string& shipClass);

        void setPosition(const float& x, const float& y);
        void setPosition(const glm::vec2& position);

        void setSize(const float& width, const float& height);
        void setSize(const glm::vec2& size);

        void update(const double& dt);
        void render();
};

#endif