#pragma once
#ifndef GAME_SCROLLBAR_H
#define GAME_SCROLLBAR_H

#include "Widget.h"

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
        float               m_BorderSize;

        float               m_ScrollBarCurrentContentPercent;
        float               m_ScrollBarCurrentPosition;
        float               m_ScrollBarStartAnchor;
        float               m_DragSnapshot;
        glm::vec4           m_ScrollBarColor;

        Button*             m_TopOrLeftButton;
        Button*             m_BottomOrRightButton;

        void internalUpdateScrollbarPosition();
    public:
        ScrollBar(const Font& font, const float x, const float y, const float w, const float h, const ScrollBarType::Type& type = ScrollBarType::Type::Vertical);
        ~ScrollBar();

        void resetScrollOffset();
        const bool isScrollable() const;

        void setBorderSize(const float borderSize);

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