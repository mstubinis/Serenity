#pragma once
#ifndef GAME_GUI_FFA_SERVER_INFO_H
#define GAME_GUI_FFA_SERVER_INFO_H

class  Font;
class  Client;
class  HostScreenFFA2;
class  ScrollFrame;
class  Text;
class  Button;
class  TextBox;

struct ShipTokenOnClickTotal;
struct ShipTokenOnClickAllowed;

#include "../RoundedWindow.h"
#include <string>

#include "../../map/MapEntry.h"
#include "../../modes/GameplayMode.h"

class FFAServerInfo final : public RoundedWindow {
    friend struct ShipTokenOnClickTotal;
    friend struct ShipTokenOnClickAllowed;
    friend class  Client;
    friend class  HostScreenFFA2;
    private:
        HostScreenFFA2&  m_HostScreen;
        ScrollFrame*     m_ScrollFrame;
        Font&            m_Font;
        bool             m_IsPersistent;
    public:
        FFAServerInfo(HostScreenFFA2&, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const std::string& labelText);
        ~FFAServerInfo();

        void onResize(const unsigned int& newWidth, const unsigned int& newHeight);

        void setPersistent();
        const bool isPersistent() const;

        TextBox& getYourNameTextBox();
        TextBox& getServerPortTextBox();
        TextBox& getMatchDurationTextBox();
        TextBox& getMaxPlayersTextBox();
        TextBox& getLobbyDurationTextBox();

        void update(const double& dt);
        void render();
};

#endif