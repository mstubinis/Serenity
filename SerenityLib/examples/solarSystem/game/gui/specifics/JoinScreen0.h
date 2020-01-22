#pragma once
#ifndef GAME_GUI_JOIN_SCREEN_0_H
#define GAME_GUI_JOIN_SCREEN_0_H

class  Button;
class  Font;
class  Menu;
class  TextBox;
class  Text;

struct Join0_ButtonBack_OnClick;

class JoinScreen0 final {
    friend struct Join0_ButtonBack_OnClick;
    private:
        Font&   m_Font;
        Menu&   m_Menu;
        Button* m_BackgroundEdgeGraphicBottom;
        Button* m_BackButton;

    public:
        JoinScreen0(Menu& menu, Font& font);
        ~JoinScreen0();

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void update(const double& dt);
        void render();
};

#endif