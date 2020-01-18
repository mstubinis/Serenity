#pragma once
#ifndef GAME_TEXT_BOX_H
#define GAME_TEXT_BOX_H

#include "Button.h"
#include <core/engine/events/Engine_EventObject.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

struct OnClick;
class TextBox : public Button, public EventObserver {
    friend struct OnClick;
    protected:
        bool                      m_Active;
        bool                      m_Locked;
        float                     m_Timer;
        unsigned short            m_MaxCharacters;
        std::string               m_Label;
        boost::function<void()>   m_OnEnter;
        glm::vec4                 m_LabelTextColor;

        void internalUpdateSize();
    public:
        TextBox(const std::string& label, const Font& font, const unsigned short maxCharacters, const float x, const float y);
        TextBox(const std::string& label, const Font& font, const unsigned short maxCharacters, const glm::vec2& position);
        virtual ~TextBox();

        template<class T> void setOnClickFunctor(const T& functor) { 
            m_FunctorOnClick = boost::bind<void>(functor, this); 
        }
        template<class T> void setOnEnterFunctor(const T& functor) { 
            m_OnEnter = boost::bind<void>(functor, this);
        }

        void lock();
        void unlock();
        const bool isLocked() const;

        void setWidth(const float);
        void setHeight(const float);
        void setSize(const float width, const float height);
        const float width() const;
        const float height() const;

        void setLabelTextColor(const float r, const float g, const float b, const float a);
        void setInputTextColor(const float r, const float g, const float b, const float a);
        void setLabelTextColor(const glm::vec4& color);
        void setInputTextColor(const glm::vec4& color);

        const glm::vec2 positionLocal() const;
        const glm::vec2 positionWorld() const;
        const glm::vec2 position(const bool local = true) const;

        void setPosition(const float x, const float y);
        void setPosition(const glm::vec2& position);

        const std::string& getLabel() const;

        void setText(const std::string& text);
        void setTextScale(const float x, const float y);
        void setTextScale(const glm::vec2& scale);

        void setLabel(const char* text);
        void setLabel(const std::string& text);

        void onEvent(const Event& e);

        void update(const double& dt);
        void render(const glm::vec4& scissor);
        void render();
};


#endif