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

class OwnedServersSelectionWindow final : public RoundedWindow {
    friend class  Client;
    friend class  HostScreen1Persistent;
    private:
        HostScreen1Persistent& m_HostScreen1Persistent;
        Button* m_ChangeGameModeLeftButton;
        Button* m_ChangeGameModeRightButton;
        ScrollFrame* m_MapFileScrollFrame;
        Font& m_Font;

    public:
        OwnedServersSelectionWindow(HostScreen1Persistent& hostScreen1Persistent, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const std::string& labelText);
        ~OwnedServersSelectionWindow();

        void clearWindow();

        void onResize(const unsigned int& newWidth, const unsigned int& newHeight);

        void update(const double& dt);
        void render();
};
#endif