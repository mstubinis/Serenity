#pragma once
#ifndef GAME_SCROLLBAR_H
#define GAME_SCROLLBAR_H

#include "Widget.h"

struct ScrollBarType final {enum Type {
    Vertical,
    Horizontal,
};};


class ScrollBar final : public Widget {
    private:
        ScrollBarType::Type m_Type;
        bool                m_CurrentlyDragging;
        float               m_BorderSize;

        float               m_ScrollBarCurrentContentPercent;
        float               m_ScrollBarCurrentPosition;
        float               m_ScrollBarStartAnchor;
        float               m_DragSnapshot;
        glm::vec4           m_ScrollBarColor;

        void internalUpdateScrollbarPosition();
    public:
        ScrollBar(const float x, const float y, const float w, const float h, const ScrollBarType::Type& type = ScrollBarType::Type::Vertical);
        ~ScrollBar();

        void setBorderSize(const float borderSize);

        void scroll(const float amount);

        void setScrollBarColor(const glm::vec4& color);

        void setPosition(const float x, const float y);
        void setPosition(const glm::vec2& position);

        void setSliderSize(const float percent);
        void setType(const ScrollBarType::Type&);
        const float getSliderHeight() const;
        const float getSliderPosition() const;


        void update(const double& dt);
        void render(const glm::vec4& scissor);
        void render();
};

#endif