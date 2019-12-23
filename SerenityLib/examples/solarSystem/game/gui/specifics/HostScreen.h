#pragma once
#ifndef GAME_GUI_HOST_SCREEN_H
#define GAME_GUI_HOST_SCREEN_H

class  Button;
class  Font;
class  Menu;
class  TextBox;
class  Text;
class  ServerHostingMapSelectorWindow;
class HostScreen final {
    private:
        Menu& m_Menu;


        Button* m_BackgroundEdgeGraphic;


        TextBox* m_UserName_TextBox;
        TextBox* m_ServerPort_TextBox;
        Text* m_Info_Text;

        ServerHostingMapSelectorWindow* m_ServerHostMapSelector;

        /*
        CurrentSelectedMapDescription
        CurrentSelectedMapScreenshot

        ShipsAvailableToUseOnMap
        ShipsCurrentlyAllowedOnMap

        Max#OfPlayers
        */


        Button* m_BackButton;
        Button* m_ForwardButton;

    public:
        HostScreen(Menu&, Font& font);
        ~HostScreen();

        Menu& getMenu();

        TextBox& getUserNameTextBox();
        TextBox& getServerPortTextBox();
        ServerHostingMapSelectorWindow& getMapSelectionWindow();

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void update(const double& dt);
        void render();
};

#endif