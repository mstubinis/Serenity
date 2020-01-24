#pragma once
#ifndef GAME_GUI_HOST_CREATE_SERVER_WINDOW_H
#define GAME_GUI_HOST_CREATE_SERVER_WINDOW_H

class  Font;
class  Client;
class  HostScreen1Persistent;
class  ScrollFrame;
class  TextBox;

#include "../RoundedWindow.h"
#include <string>

#include "../../map/MapEntry.h"
#include "../../modes/GameplayMode.h"

struct ServerCreateOnClick;

class CreateServerWindow final : public RoundedWindow {
    friend class  Client;
    friend class  HostScreen1Persistent;
    friend struct ServerCreateOnClick;
    private:
        HostScreen1Persistent& m_HostScreen1Persistent;

        ScrollFrame* m_ScrollFrame;
        Font& m_Font;

        Button* m_CreateButton;

    public:
        CreateServerWindow(HostScreen1Persistent& hostScreen1Persistent, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const std::string& labelText);
        ~CreateServerWindow();

        void resetWindow();

        TextBox& getServerNameTextBox();
        TextBox& getServerPortTextBox();
        TextBox& getUsernameTextBox();
        TextBox& getPasswordTextBox();

        void onResize(const unsigned int& newWidth, const unsigned int& newHeight);

        void update(const double& dt);
        void render();
};
#endif