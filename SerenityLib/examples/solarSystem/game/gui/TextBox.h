#pragma once
#ifndef GAME_TEXT_BOX_H
#define GAME_TEXT_BOX_H

#include "Button.h"

struct OnClick;
class TextBox : public Button {
    friend struct OnClick;
    protected:
        bool             m_Active;
        unsigned short   m_MaxCharacters;
    public:
        TextBox(Font& font, const unsigned short& maxCharacters, const float& x, const float& y, const float& width, const float& height);
        TextBox(Font& font, const unsigned short& maxCharacters, const glm::vec2& position, const float& width, const float& height);
        virtual ~TextBox();

        template<class T> void setOnClickFunctor(const T& functor) { m_FunctorOnClick = boost::bind<void>(functor, *this); }

        void setText(const char* text);
        void setText(const std::string& text);

        void update(const double& dt);
        void render();
};


#endif