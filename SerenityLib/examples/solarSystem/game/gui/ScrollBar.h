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
        float               m_ScrollBarCurrentPosition;
        float               m_ScrollBarStartAnchor;
        float               m_DragSnapshot;

    public:
        ScrollBar(const float& x, const float& y, const float& w, const float& h, const ScrollBarType::Type& type = ScrollBarType::Type::Vertical);
        ~ScrollBar();

        void setBorderSize(const float);

        void setSliderSize(const float percent);
        void setType(const ScrollBarType::Type&);
        const float getSliderHeight() const;
        const float getSliderPosition() const;

        void update(const double& dt);
        void render();
};

#endif