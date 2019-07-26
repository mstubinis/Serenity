#pragma once
#ifndef GAME_BUTTON_H
#define GAME_BUTTON_H

#include "Widget.h"
#include <core/engine/fonts/FontIncludes.h>
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>

class Font;
class Button : public Widget {
    protected:
        Font*                     m_Font;
        std::string               m_Text;
        glm::vec2                 m_TextScale;
        glm::vec4                 m_TextColor;
        boost::function<void()>   m_FunctorOnClick;
        unsigned int              m_Padding;
        TextAlignment::Type       m_TextAlignment;

        void internalSetSize();
    public:
        Button(const Font& font, const unsigned int& x, const unsigned int& y, const unsigned int& width, const unsigned int& height);
        Button(const Font& font, const glm::uvec2& position, const unsigned int& width, const unsigned int& height);
        virtual ~Button();

        const std::string& text() const;

        template<class T> void setOnClickFunctor(const T& functor) { m_FunctorOnClick = boost::bind<void>(functor, this); }

        const unsigned int getTextHeight() const;
        const unsigned int getTextWidth() const;
        void setTextAlignment(const TextAlignment::Type&);
        const TextAlignment::Type& getTextAlignment() const;

        virtual void setTextScale(const glm::vec2& scale);
        virtual void setTextScale(const float& x, const float& y);
        const glm::vec2& getTextScale() const;

        void setFont(const Font& font);
        void setTextColor(const float& r, const float& g, const float& b, const float& a);
        void setTextColor(const glm::vec4& color);
        virtual void setText(const char* text);
        virtual void setText(const std::string& text);

        virtual void update(const double& dt);
        virtual void render();
};

#endif