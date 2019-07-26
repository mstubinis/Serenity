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
        unsigned int                              m_BorderSize;
        unsigned int                              m_ContentPadding;
        unsigned int                              m_ContentHeight;
        std::vector<Widget*>                      m_Content;
    public:
        ScrollFrame(const unsigned int& x, const unsigned int& y, const unsigned int& w, const unsigned int& h);
        ~ScrollFrame();

        const float contentHeight() const;

        void addContent(Widget* widget);
        void setAlignment(const Alignment::Type& alignment);

        void setBorderSize(const unsigned int);
        void setContentPadding(const unsigned int);

        void internalAddContent();
        std::vector<Widget*>& content();

        const unsigned int width() const;
        const unsigned int height() const;

        void setPosition(const unsigned int& x, const unsigned int& y);
        void setPosition(const glm::uvec2& position);

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setColor(const glm::vec4& color);

        void update(const double& dt);
        void render();
};


#endif