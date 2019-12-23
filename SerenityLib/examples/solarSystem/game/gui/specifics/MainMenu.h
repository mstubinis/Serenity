#pragma once
#ifndef GAME_GUI_MAIN_MENU_H
#define GAME_GUI_MAIN_MENU_H

class  Button;
class  Font;
class  Menu;

#include <string>

class MainMenu final {
    private:


        Button*          m_ButtonHost;
        Button*          m_ButtonJoin;
        Button*          m_ButtonOptions;
        Button*          m_ButtonEncyclopedia;

    public:
        MainMenu(Menu&, Font& font);
        ~MainMenu();

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void update(const double& dt);
        void render();
};

#endif