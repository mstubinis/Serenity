#pragma once
#ifndef GAME_GUI_HOST_SCREEN_1_PERSISTENT_H
#define GAME_GUI_HOST_SCREEN_1_PERSISTENT_H
 
class  Font;
class  Menu;
class  Button;

class  OwnedServersSelectionWindow;
class  CreateServerWindow;

struct Host1Persistent_ButtonBack_OnClick;
struct Host1Persistent_ButtonNext_OnClick;

struct ServerCreateOnClick;

class HostScreen1Persistent final {
    friend struct Host1Persistent_ButtonBack_OnClick;
    friend struct Host1Persistent_ButtonNext_OnClick;
    friend struct ServerCreateOnClick;
    private:
        Font&   m_Font;
        Menu&   m_Menu;

        OwnedServersSelectionWindow* m_OwnedServersWindow;
        CreateServerWindow*          m_CreateServersWindow;

        Button* m_BackgroundEdgeGraphicBottom;
        Button* m_BackButton;
        Button* m_ForwardButton;

    public:
        HostScreen1Persistent(Menu&, Font&);
        ~HostScreen1Persistent();

        void onResize(const unsigned int& newWidth, const unsigned int& newHeight);

        void update(const double& dt);
        void render();
};

#endif