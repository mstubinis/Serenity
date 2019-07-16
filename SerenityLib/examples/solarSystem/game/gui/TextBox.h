#pragma once
#ifndef GAME_TEXT_BOX_H
#define GAME_TEXT_BOX_H

#include "Button.h"

struct OnClick;
class TextBox : public Button {
    friend struct OnClick;
    protected:
        bool             m_Active;
        float            m_Timer;
        unsigned short   m_MaxCharacters;
        std::string      m_Label;
    public:
        TextBox(const std::string& label, Font& font, const unsigned short& maxCharacters, const float& x, const float& y);
        TextBox(const std::string& label, Font& font, const unsigned short& maxCharacters, const glm::vec2& position);
        virtual ~TextBox();

        template<class T> void setOnClickFunctor(const T& functor) { 
            m_FunctorOnClick = boost::bind<void>(functor, this); 
        }

        const std::string& getLabel() const;

        void setText(const char* text);
        void setText(const std::string& text);
        void setTextScale(const float& x, const float& y);
        void setTextScale(const glm::vec2& scale);

        void setLabel(const char* text);
        void setLabel(const std::string& text);

        void update(const double& dt);
        void render();
};


#endif