#pragma once
#ifndef GAME_GUI_HOST_SCREEN_2_FFA_H
#define GAME_GUI_HOST_SCREEN_2_FFA_H

class  Button;
class  Font;
class  Menu;
class  TextBox;
class  Text;
class  ServerHostingMapSelectorWindow;
class  MapDescriptionWindow;
struct Host2FFA_ButtonBack_OnClick;
struct Host2FFA_ButtonNext_OnClick;
class  HostScreen1;
class  FFAServerInfo;
class  FFAShipSelector;

class HostScreenFFA2 final {
    friend class  HostScreen1;
    friend struct Host2FFA_ButtonBack_OnClick;
    friend struct Host2FFA_ButtonNext_OnClick;
    friend class  FFAServerInfo;
    friend class  FFAShipSelector;
    private:
        HostScreen1& m_HostScreen1;
        Menu&       m_Menu;

        FFAShipSelector* m_ShipSelectorWindow;
        FFAServerInfo*   m_ServerInfoWindow;

        Button*     m_BackgroundEdgeGraphicTop;
        Text*       m_TopLabel;

        Button*     m_BackgroundEdgeGraphicBottom;
        Button*     m_BackButton;
        Button*     m_ForwardButton;

        const unsigned int get_duration_sec_helper(TextBox&);
        const float get_duration_min_helper(TextBox&);

    public:
        HostScreenFFA2(HostScreen1& hostScreen1, Menu&, Font& font);
        ~HostScreenFFA2();

        Menu& getMenu();
        
        const bool validateMaxNumPlayersTextBox();
        const bool validateShipSelector();
        const bool validateServerPortTextBox();
        const bool validateUsernameTextBox();
        const bool validateMatchDurationTextBox();
        const bool validateLobbyDurationTextBox();

        const unsigned int getMatchDurationFromTextBoxInSeconds();
        const float getMatchDurationFromTextBoxInMinutes();

        const unsigned int getLobbyDurationFromTextBoxInSeconds();
        const float getLobbyDurationFromTextBoxInMinutes();

        void setTopText(const std::string& text);

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void update(const double& dt);
        void render();
};

#endif