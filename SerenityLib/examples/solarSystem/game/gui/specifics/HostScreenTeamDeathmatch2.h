#pragma once
#ifndef GAME_GUI_HOST_SCREEN_2_TEAM_DEATHMATCH_H
#define GAME_GUI_HOST_SCREEN_2_TEAM_DEATHMATCH_H

class  Button;
class  Font;
class  Menu;
class  TextBox;
class  Text;
class  ServerHostingMapSelectorWindow;
class  MapDescriptionWindow;
struct Host2TD_ButtonBack_OnClick;
struct Host2TD_ButtonNext_OnClick;
class  HostScreen;

class HostScreenTeamDeathmatch2 final {
    friend class  HostScreen;
    friend struct Host2TD_ButtonBack_OnClick;
    friend struct Host2TD_ButtonNext_OnClick;
    private:
        HostScreen&   m_HostScreen1;
        Menu&         m_Menu;

        Button* m_BackgroundEdgeGraphicTop;
        Text* m_TopLabel;

        TextBox*      m_UserName_TextBox;
        TextBox*      m_ServerPort_TextBox;

        Button*       m_BackgroundEdgeGraphicBottom;
        Button*       m_BackButton;
        Button*       m_ForwardButton;

    public:
        HostScreenTeamDeathmatch2(HostScreen& hostScreen1, Menu&, Font& font);
        ~HostScreenTeamDeathmatch2();

        Menu& getMenu();

        void setTopText(const std::string& text);

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void update(const double& dt);
        void render();
};

#endif