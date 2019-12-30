#pragma once
#ifndef GAME_GUI_SCROLL_FRAME_H
#define GAME_GUI_SCROLL_FRAME_H

#include "Widget.h"
#include <unordered_map>
#include <vector>

class ScrollBar;
class Font;
class Text;
class ScrollFrame: public Widget {

    struct WidgetEntry {
        Widget* widget;
        float original_width;
        WidgetEntry() {
            widget = nullptr;
            original_width = 0.0f;
        }
        WidgetEntry(Widget* widget_) {
            widget = widget_;
            original_width = widget_->width();
        }
        ~WidgetEntry() {}
    };

    private:
        ScrollBar*                                m_ScrollBar;
        float                                     m_BorderSize;
        float                                     m_ContentHeight;
        std::vector<ScrollFrame::WidgetEntry>     m_Content;

        void reposition_scroll_bar();
        void internal_recalculate_content_sizes();
        void fit_widget_to_window(WidgetEntry& widget);
    public:
        ScrollFrame(const Font& font, const float x, const float y, const float w, const float h);
        virtual ~ScrollFrame();

        const float contentHeight() const;

        void clear();

        void addContent(Widget* widget);
        void removeContent(Widget* widget);
        void setAlignment(const Alignment::Type& alignment);

        void setBorderSize(const float borderSize);

        void setWidth(const float);
        void setHeight(const float);
        void setSize(const float width, const float height);

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        std::vector<ScrollFrame::WidgetEntry>& content();

        void setPosition(const float x, const float y);
        void setPosition(const glm::vec2& position);

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setColor(const glm::vec4& color);

        void update(const double& dt);
        void render(const glm::vec4& scissor);
        void render();
};


#endif