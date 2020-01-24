#pragma once
#ifndef GAME_GUI_TEMPLATE_MESSAGE_WITH_ARROW_H
#define GAME_GUI_TEMPLATE_MESSAGE_WITH_ARROW_H

class Text;
class Font;

#include <glm/vec2.hpp>
#include <string>

class MessageWithArrow final {
    private:
        Text*   m_Text;
        bool    m_Shown;
        double  m_Timer;
    public:
        MessageWithArrow(Font& font, const std::string& text, const float& x, const float& y, const float& textScale);
        ~MessageWithArrow();

        void setPosition(const float& x, const float& y);
        void setPosition(const glm::vec2& position);

        void hide();
        void show();

        void update(const double& dt);
        void render();
};

#endif