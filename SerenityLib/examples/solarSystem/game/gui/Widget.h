#pragma once
#ifndef GAME_WIDGET_H
#define GAME_WIDGET_H

#include "WidgetInterface.h"
#include <glm/vec2.hpp>

class Widget : public IWidget {
    protected:
        void*                             m_UserPointer;
        unsigned int                      m_Width;
        unsigned int                      m_Height;
        glm::uvec2                        m_Position;
        glm::vec4                         m_Color;
        bool                              m_Hidden;
        bool                              m_MouseIsOver;
        Alignment::Type                   m_Alignment;

        const glm::uvec2 positionFromAlignment();
        const glm::uvec2 positionFromAlignment(const unsigned int& width, const unsigned int& height, const Alignment::Type& alignment);
    public:
        Widget(const glm::vec2& position, const unsigned int& width, const unsigned int& height);
        Widget(const unsigned int& x, const unsigned int& y, const unsigned int& width, const unsigned int& height);
        virtual ~Widget();

        virtual void setWidth(const unsigned int&);
        virtual void setHeight(const unsigned int&);
        virtual void setSize(const unsigned int& width, const unsigned int& height);

        virtual const unsigned int width() const;
        virtual const unsigned int height() const;

        virtual const bool isMouseOver() const;

        const glm::uvec2& position() const;
        const glm::vec4& color() const;

        void setAlignment(const Alignment::Type& alignment);

        void* getUserPointer() const;
        void setUserPointer(void*);

        virtual void setPosition(const unsigned int& x, const unsigned int& y);
        virtual void setPosition(const glm::uvec2& position);

        virtual void setColor(const float& r, const float& g, const float& b, const float& a);
        virtual void setColor(const glm::vec4& color);
        void show();
        void hide();

        virtual void update(const double& dt);
        virtual void render();
};

#endif