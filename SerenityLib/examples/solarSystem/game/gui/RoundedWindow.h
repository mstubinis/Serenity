#pragma once
#ifndef GAME_GUI_TEMPLATE_ROUNDED_WINDOW_H
#define GAME_GUI_TEMPLATE_ROUNDED_WINDOW_H

#include "Button.h"
#include <glm/vec2.hpp>
#include <string>
#include <functional>

class  Text;
class  Font;
class RoundedWindow: public Button {
    protected:
        Button*   m_Background;
        Text*     m_Label;
        float     m_BorderSize;

        std::function<glm::vec2()> m_SizeFunctor;
        std::function<glm::vec2()> m_PositionFunctor;
    public:
        template<typename T> void setSizeFunctor(const T& functor) {
            m_SizeFunctor = std::bind<glm::vec2>(functor, this);
        }

        template<typename T> void setPositionFunctor(const T& functor) {
            m_PositionFunctor = std::bind<glm::vec2>(functor, this);
        }
    public:
        RoundedWindow(Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth,const float borderSize = 1, const std::string& labelText = "");
        virtual ~RoundedWindow();

        virtual void onResize(const unsigned int& newWidth, const unsigned int& newHeight);

        void setLabelText(const std::string& text);

        virtual void update(const double& dt);
        virtual void render();
};

#endif