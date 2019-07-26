#pragma once
#ifndef GAME_SERVER_HOSTING_MAP_SELECTOR_WINDOW_H
#define GAME_SERVER_HOSTING_MAP_SELECTOR_WINDOW_H

class  Font;
class  ScrollFrame;
class  Widget;
class  Text;
class  Client;
struct MapSelectorButtonOnClick;
class ServerHostingMapSelectorWindow final {
    friend struct MapSelectorButtonOnClick;
    friend class  Client;
    private:
        Text*        m_Label;
        Text*        m_CurrentChoice;
        ScrollFrame* m_MapFileWindow;
        Font&        m_Font;
        unsigned int m_Width;
        unsigned int m_Height;
        void*        m_UserPointer;
    public:
        ServerHostingMapSelectorWindow(const Font& font, const unsigned int& x, const unsigned int& y);
        ~ServerHostingMapSelectorWindow();

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setPosition(const unsigned int& x, const unsigned int& y);

        void setUserPointer(void*);
        void* getUserPointer();
        void clear();

        void addContent(Widget* widget);

        const Text& getCurrentChoice() const;
        Font& getFont();
        ScrollFrame& getWindowFrame();

        void update(const double& dt);
        void render();
};

#endif