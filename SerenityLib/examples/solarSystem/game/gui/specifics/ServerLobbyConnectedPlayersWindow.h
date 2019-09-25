#pragma once
#ifndef GAME_SERVER_LOBBY_CONNECTED_PLAYERS_WINDOW_H
#define GAME_SERVER_LOBBY_CONNECTED_PLAYERS_WINDOW_H

#include "../ScrollWindow.h"
#include "../TextBox.h"

class ServerLobbyConnectedPlayersWindow final {
    private:
        ScrollFrame* m_ChatWindow;
        Font&        m_Font;
        float        m_Width;
        float        m_Height;
        void*        m_UserPointer;
    public:
        ServerLobbyConnectedPlayersWindow(const Font& font, const float x, const float y);
        ~ServerLobbyConnectedPlayersWindow();

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setPosition(const float x, const float y);

        void setUserPointer(void*);
        void* getUserPointer();

        void addContent(Widget* widget);
        void removeContent(const std::string& content);
        void clear();

        Font& getFont();
        ScrollFrame& getWindowFrame();

        void update(const double& dt);
        void render();
};
#endif