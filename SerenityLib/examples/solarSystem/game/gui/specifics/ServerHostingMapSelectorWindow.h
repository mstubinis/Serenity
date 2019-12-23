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
        std::string  m_CurrentChoice;
        ScrollFrame* m_MapFileScrollFrame;
        Font&        m_Font;
        void*        m_UserPointer;
    public:
        ServerHostingMapSelectorWindow(const Font& font, const float x, const float y, const float w, const float h);
        ~ServerHostingMapSelectorWindow();

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setPosition(const float x, const float y);

        void setSize(const float& w, const float& h);

        void setUserPointer(void*);
        void* getUserPointer();
        void clear();

        void addContent(Widget* widget);

        const std::string& getCurrentChoice() const;
        Font& getFont();
        ScrollFrame& getWindowFrame();

        void update(const double& dt);
        void render();
};

#endif