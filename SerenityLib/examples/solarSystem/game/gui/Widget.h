#pragma once
#ifndef GAME_WIDGET_H
#define GAME_WIDGET_H

#include "WidgetInterface.h"
#include <glm/vec2.hpp>

class Widget : public IWidget {
    protected:
        void*                             m_UserPointer;
        float                             m_Width;
        float                             m_Height;
        glm::vec2                         m_Position;
        glm::vec4                         m_Color;
        bool                              m_Hidden;
        bool                              m_MouseIsOver;
        Alignment::Type                   m_Alignment;

        const glm::vec2 positionFromAlignment();
        const glm::vec2 positionFromAlignment(const float& width, const float& height, const Alignment::Type& alignment);
    public:
        Widget(const glm::vec2& position, const float& width, const float& height);
        Widget(const float& x, const float& y, const float& width, const float& height);
        virtual ~Widget();

        virtual void setWidth(const float&);
        virtual void setHeight(const float&);
        virtual void setSize(const float& width, const float& height);

        virtual const float width() const;
        virtual const float height() const;

        virtual const bool isMouseOver() const;

        const glm::vec2& position() const;
        const glm::vec4& color() const;

        void setAlignment(const Alignment::Type& alignment);

        void* getUserPointer() const;
        void setUserPointer(void*);

        virtual void setPosition(const float& x, const float& y);
        virtual void setPosition(const glm::vec2& position);

        virtual void setColor(const float& r, const float& g, const float& b, const float& a);
        virtual void setColor(const glm::vec4& color);
        void show();
        void hide();

        virtual void update(const double& dt);
        virtual void render();
};

#endif