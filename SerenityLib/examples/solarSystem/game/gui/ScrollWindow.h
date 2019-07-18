#pragma once
#ifndef GAME_SCROLLWINDOW_H
#define GAME_SCROLLWINDOW_H

#include "Widget.h"
#include <unordered_map>

class ScrollBar;
class Font;
class Text;
class ScrollFrame: public Widget {
    private:
        ScrollBar*                                m_ScrollBar;
        float                                     m_BorderSize;
        float                                     m_ContentPadding;
        float                                     m_ContentHeight;
        std::unordered_map<std::string, Widget*>  m_Content;

        void internalAddContent();

    public:
        ScrollFrame(const float& x, const float& y, const float& w, const float& h);
        ~ScrollFrame();

        const float contentHeight() const;

        void addContent(const std::string& key, Widget* widget);
        void removeContent(const std::string& key);

        void setBorderSize(const float);

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setColor(const glm::vec4& color);

        void update(const double& dt);
        void render();
};


#endif