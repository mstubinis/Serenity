#pragma once
#ifndef GAME_GUI_HOST_SCREEN_H
#define GAME_GUI_HOST_SCREEN_H

class  Button;
class  Font;
class  Menu;
class  TextBox;
class  Text;
class  ServerHostingMapSelectorWindow;
class  RoundedWindow;
class  MapSelectionWindow;
class  MapDescriptionWindow;
    class HostScreen final {
        private:
            Menu& m_Menu;

            Button* m_BackgroundEdgeGraphicBottom;

            //left window
            MapSelectionWindow* m_LeftWindow;

            //right window
            RoundedWindow*   m_RightWindow;

            TextBox* m_UserName_TextBox;
            TextBox* m_ServerPort_TextBox;
 
            MapDescriptionWindow*           m_MapDescriptionWindow;

            Button* m_BackButton;
            Button* m_ForwardButton;

        public:
            HostScreen(Menu&, Font& font);
            ~HostScreen();

            Menu& getMenu();

            TextBox& getUserNameTextBox();
            TextBox& getServerPortTextBox();
            MapSelectionWindow& getMapSelectionWindow();
            MapDescriptionWindow& getMapDescriptionWindow();

            void onResize(const unsigned int newWidth, const unsigned int newHeight);

            void update(const double& dt);
            void render();
};

#endif