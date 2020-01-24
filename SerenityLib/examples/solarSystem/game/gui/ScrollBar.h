#pragma once
#ifndef GAME_SCROLLBAR_H
#define GAME_SCROLLBAR_H

#include "Widget.h"
#include "GUIRenderElement.h"

class  Button;
class  Font;
struct ScrollBar_TopButtonFunctor;
struct ScrollBar_BottomButtonFunctor;

struct ScrollBarType final {enum Type {
    Vertical,
    Horizontal,
};};

class ScrollBar final : public Widget {
    friend struct ScrollBar_TopButtonFunctor;
    friend struct ScrollBar_BottomButtonFunctor;
    private:
        ScrollBarType::Type m_Type;
        bool                m_CurrentlyDragging;


        float               m_ScrollBarCurrentContentPercent;
        float               m_ScrollBarCurrentPosition;
        float               m_ScrollBarStartAnchor;
        float               m_DragSnapshot;
        glm::vec4           m_ScrollBarColor;

        Button*             m_ScrollAreaBackground;
        Button*             m_ScrollArea;
        Button*             m_TopOrLeftButton;
        Button*             m_BottomOrRightButton;

        void update_scroll_bar_position();
        const float get_scroll_bar_starting_y() const;
        const float get_scroll_area_max_height() const;
    public:
        ScrollBar(const Font& font, const float x, const float y, const float w, const float h, const float depth, const ScrollBarType::Type& type = ScrollBarType::Type::Vertical);
        ~ScrollBar();

        void resetScrollOffset();
        const bool isScrollable() const;

        void scroll(const float amount);

        void setScrollBarColor(const glm::vec4& color);

        void setPosition(const float x, const float y);
        void setPosition(const glm::vec2& position);

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setColor(const glm::vec4& color);

        void setWidth(const float);
        void setHeight(const float);
        void setSize(const float width, const float height);

        void setSliderSize(const float percent);
        void setType(const ScrollBarType::Type&);
        const float getSliderHeight() const;
        const float getSliderPosition() const;


        void update(const double& dt);
        void render(const glm::vec4& scissor);
        void render();
};

#endif