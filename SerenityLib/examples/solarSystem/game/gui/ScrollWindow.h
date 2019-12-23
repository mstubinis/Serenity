#pragma once
#ifndef GAME_SCROLLWINDOW_H
#define GAME_SCROLLWINDOW_H

#include "Widget.h"
#include <unordered_map>
#include <vector>

class ScrollBar;
class Font;
class Text;
class ScrollFrame: public Widget {
    private:
        ScrollBar*                                m_ScrollBar;
        float                                     m_BorderSize;
        float                                     m_ContentPadding;
        float                                     m_ContentHeight;
        std::vector<Widget*>                      m_Content;

        void internal_recalculate_content_sizes();
        void fit_widget_to_window(Widget* widget);
    public:
        ScrollFrame(const float x, const float y, const float w, const float h);
        virtual ~ScrollFrame();

        const float contentHeight() const;

        void addContent(Widget* widget);
        void removeContent(Widget* widget);
        void setAlignment(const Alignment::Type& alignment);

        void setBorderSize(const float borderSize);
        void setContentPadding(const float padding);

        void setWidth(const float);
        void setHeight(const float);
        void setSize(const float width, const float height);

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        std::vector<Widget*>& content();

        void setPosition(const float x, const float y);
        void setPosition(const glm::vec2& position);

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setColor(const glm::vec4& color);

        void update(const double& dt);
        void render(const glm::vec4& scissor);
        void render();
};


#endif