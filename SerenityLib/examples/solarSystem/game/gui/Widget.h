#pragma once
#ifndef GAME_WIDGET_H
#define GAME_WIDGET_H

#include "WidgetInterface.h"
#include <vector>
#include <glm/vec2.hpp>

class GUIRenderElement;
class Widget : public IWidget {
    friend class GUIRenderElement;
    protected:
        void*                             m_UserPointer;
        float                             m_Width;
        float                             m_Height;
        glm::vec2                         m_Position;
        glm::vec4                         m_Color;
        bool                              m_Hidden;
        bool                              m_MouseIsOver;
        Alignment::Type                   m_Alignment;

        std::vector<Widget*>              m_Children;
        Widget*                           m_Parent;

        const glm::vec2 positionFromAlignmentWorld();
        const glm::vec2 positionFromAlignmentWorld(const float width, const float height, const Alignment::Type& alignment);
        const glm::vec2 positionFromAlignment();
        const glm::vec2 positionFromAlignment(const float width, const float height, const Alignment::Type& alignment);
    public:
        Widget(const glm::vec2& position, const glm::vec2& size);
        Widget(const glm::vec2& position, const float width, const float height);
        Widget(const float x, const float y, const float width, const float height);
        Widget(const float x, const float y, const glm::vec2& size);
        virtual ~Widget();

        const bool setParent(Widget* parent);
        const bool clearParent();

        const bool addChild(Widget* child);
        const bool removeChild(Widget* child);

        virtual void setWidth(const float);
        virtual void setHeight(const float);
        virtual void setSize(const float width, const float height);

        virtual const float width() const;
        virtual const float height() const;

        virtual const bool isMouseOver() const;

        inline const glm::vec2& positionLocal() const;
        const glm::vec2  positionWorld() const;
        const glm::vec2& position(const bool local = true) const;
        const glm::vec4& color() const;

        void setAlignment(const Alignment::Type& alignment);
        const Alignment::Type& getAlignment() const;

        void* getUserPointer() const;
        void setUserPointer(void*);

        virtual void setPosition(const float x, const float y);
        virtual void setPosition(const glm::vec2& position);

        virtual void setColor(const float& r, const float& g, const float& b, const float& a);
        virtual void setColor(const glm::vec4& color);
        void show();
        void hide();

        virtual void update(const double& dt);
        virtual void render(const glm::vec4& scissor);
        virtual void render();
};

#endif