#pragma once
#ifndef GAME_TEXT_H
#define GAME_TEXT_H

#include "Widget.h"
#include <string>

#include <core/engine/fonts/FontIncludes.h>

class Font;
class Text final: public Widget {
    private:
        std::string          m_Text;
        Font*                m_Font;
        glm::vec2            m_TextScale;
        TextAlignment::Type  m_TextAlignment;

        void internalUpdateSize();
    public:
        Text(const float& x, const float& y, Font& font, const std::string& text = "");
        ~Text();

        void setFont(const Font&);
        void setText(const std::string&);

        void setTextAlignment(const TextAlignment::Type&);
        void setTextScale(const glm::vec2& scale);
        void setTextScale(const float& x, const float& y);

        const float width() const;
        const float height() const;
        const std::string& text() const;
        const Font& font() const;
        const glm::vec2& textScale() const;

        void update(const float& dt);
        void render();
};
#endif