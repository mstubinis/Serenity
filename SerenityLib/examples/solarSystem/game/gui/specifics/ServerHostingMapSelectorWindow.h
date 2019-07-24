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
        float        m_Width;
        float        m_Height;
        void*        m_UserPointer;
    public:
        ServerHostingMapSelectorWindow(const Font& font, const float& x, const float& y);
        ~ServerHostingMapSelectorWindow();

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setPosition(const float& x, const float& y);

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