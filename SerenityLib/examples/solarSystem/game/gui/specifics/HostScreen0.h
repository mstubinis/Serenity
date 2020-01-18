#pragma once
#ifndef GAME_GUI_HOST_SCREEN_0_H
#define GAME_GUI_HOST_SCREEN_0_H

class  Button;
class  Font;
class  Menu;
class  TextBox;
class  Text;

#include "../../map/MapEntry.h"
#include "../../modes/GameplayMode.h"

struct Host0_ButtonBack_OnClick;
struct HostTempServClick;
struct HostPersServClick;

class HostScreen0 final {
    friend struct Host0_ButtonBack_OnClick;
    friend struct HostTempServClick;
    friend struct HostPersServClick;
    private:
        Font& m_Font;
        Menu& m_Menu;
        Button* m_BackgroundEdgeGraphicBottom;
        Button* m_BackButton;

        Button* m_HostTemporaryServer;
        Button* m_HostPersistentServer;
    public:
        HostScreen0(Menu& menu, Font& font);
        ~HostScreen0();

        Button& getHostTemporaryServerButton();
        Button& getHostPersistentServerButton();

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void update(const double& dt);
        void render();
};

#endif