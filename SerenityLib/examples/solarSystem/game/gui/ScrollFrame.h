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
        std::string original_text;
        WidgetEntry();
        WidgetEntry(Widget* widget_);
        ~WidgetEntry();
    };
    struct WidgetRow final {
        std::vector<WidgetEntry> widgets;
        float maxHeight;
        WidgetRow();
        ~WidgetRow();
        void clear();
    };

    private:
        ScrollBar*                                m_ScrollBar;
        float                                     m_ContentHeight;
        std::vector<ScrollFrame::WidgetRow>       m_Content;
        glm::uvec4                                m_PaddingSize;

        void recalc_all_max_row_heights();
        void recalc_max_row_height(WidgetRow&);

        void reposition_scroll_bar();
        void internal_recalculate_content_sizes();

        void fit_all_widgets_to_window();
        void fit_widget_to_window(WidgetEntry& widget);

        const float get_true_content_height(const bool updateToo = false, const double& dt = 0.0);
    public:
        ScrollFrame(const Font& font, const float x, const float y, const float w, const float h, const float depth);
        virtual ~ScrollFrame();

        const float contentHeight() const;

        void clear();

        void setPaddingSize(const unsigned int padding);
        void setPaddingSize(const unsigned int padding, const unsigned int index);
        void addContent(Widget* widget);
        void addContent(Widget* widget, const unsigned int row);
        void removeContent(Widget* widget);

        void setAlignment(const Alignment::Type& alignment);

        void setWidth(const float);
        void setHeight(const float);
        void setSize(const float width, const float height);

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        std::vector<ScrollFrame::WidgetRow>& content();

        void setPosition(const float x, const float y);
        void setPosition(const glm::vec2& position);

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setColor(const glm::vec4& color);

        void update(const double& dt);
        void render(const glm::vec4& scissor);
        void render();
};


#endif