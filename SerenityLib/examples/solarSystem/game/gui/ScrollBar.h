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
        unsigned int        m_BorderSize;

        float               m_ScrollBarCurrentContentPercent;
        int                 m_ScrollBarCurrentPosition;
        unsigned int        m_ScrollBarStartAnchor;
        float               m_DragSnapshot;
        glm::vec4           m_ScrollBarColor;

        void internalUpdateScrollbarPosition();
    public:
        ScrollBar(const unsigned int& x, const unsigned int& y, const unsigned int& w, const unsigned int& h, const ScrollBarType::Type& type = ScrollBarType::Type::Vertical);
        ~ScrollBar();

        void setBorderSize(const unsigned int borderSize);

        void scroll(const int amount);

        void setScrollBarColor(const glm::vec4& color);

        void setPosition(const unsigned int& x, const unsigned int& y);
        void setPosition(const glm::uvec2& position);

        void setSliderSize(const float percent);
        void setType(const ScrollBarType::Type&);
        const unsigned int getSliderHeight() const;
        const unsigned int getSliderPosition() const;

        void update(const double& dt);
        void render();
};

#endif