#pragma once
#ifndef GAME_BUTTON_H
#define GAME_BUTTON_H

#include "Widget.h"
#include <core/engine/fonts/Font.h>
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>

class Button : public Widget {
    protected:
        Font*                     m_Font;
        std::string               m_Text;
        boost::function<void()>   m_FunctorOnClick;
    public:
        Button(Font& font, const float& x, const float& y, const float& width, const float& height);
        Button(Font& font, const glm::vec2& position, const float& width, const float& height);
        virtual ~Button();

        const std::string& text() const;

        template<class T> void setOnClickFunctor(const T& functor) { m_FunctorOnClick = boost::bind<void>(functor, *this); }

        void setFont(Font& font);
        void setText(const char* text);
        void setText(const std::string& text);

        void update(const double& dt);
        void render();
};

#endif