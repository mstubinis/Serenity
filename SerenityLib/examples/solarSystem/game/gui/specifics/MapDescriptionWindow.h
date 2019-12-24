#pragma once
#ifndef GAME_GUI_MAP_DESCRIPTION_WINDOW_H
#define GAME_GUI_MAP_DESCRIPTION_WINDOW_H

class  Font;
class  ScrollFrame;
class  Widget;

#include "../../map/MapEntry.h"

class MapDescriptionWindow final {
    private:
        ScrollFrame*  m_ScrollFrame;
        Font&         m_Font;
        void*         m_UserPointer;
    public:
        MapDescriptionWindow(Font& font, const float& x, const float& y, const float& width, const float& height);
        ~MapDescriptionWindow();

        void setCurrentMapChoice(const MapEntryData& choice);

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setPosition(const float x, const float y);

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void setSize(const float& w, const float& h);

        void setUserPointer(void*);
        void* getUserPointer();
        void clear();

        void addContent(Widget* widget);

        Font& getFont();
        ScrollFrame& getWindowFrame();

        void update(const double& dt);
        void render();
};

#endif