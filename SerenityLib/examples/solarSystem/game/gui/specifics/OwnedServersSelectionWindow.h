#pragma once
#ifndef GAME_GUI_HOST_OWNED_SERVERS_SELECTION_WINDOW_H
#define GAME_GUI_HOST_OWNED_SERVERS_SELECTION_WINDOW_H

class  Font;
class  Client;
class  HostScreen1Persistent;
class  ScrollFrame;

#include "../RoundedWindow.h"
#include <string>

#include "../../map/MapEntry.h"
#include "../../modes/GameplayMode.h"

struct ServerButtonOnClick;

class OwnedServersSelectionWindow final : public RoundedWindow {
    friend class  Client;
    friend class  HostScreen1Persistent;
    friend struct ServerButtonOnClick;

    struct UserPointer final {
        OwnedServersSelectionWindow* window;
        std::string                  ownerName;
        std::string                  serverName;
        unsigned int                 port;
    };

    private:
        HostScreen1Persistent& m_HostScreen1Persistent;
        UserPointer*           m_SelectedServer;
        ScrollFrame*           m_ScrollFrame;
        Font&                  m_Font;

        void add_button(const std::string& serverName, const unsigned int& serverPort, const std::string& username);
        void clear_selected_server();
    public:
        OwnedServersSelectionWindow(HostScreen1Persistent& hostScreen1Persistent, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const std::string& labelText);
        ~OwnedServersSelectionWindow();

        void clearWindow();
        void populateWindow();

        const unsigned int getNumServers();

        UserPointer* getSelectedServer();

        const bool hasServer(const std::string& serverName);

        const bool addServer(const std::string& serverName, const unsigned int& serverPort, const std::string& username, const std::string& password);

        void onResize(const unsigned int& newWidth, const unsigned int& newHeight);

        void update(const double& dt);
        void render();
};
#endif