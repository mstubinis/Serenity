#pragma once
#ifndef GAME_GUI_HOST_SCREEN_MAP_SELECTION_WINDOW_H
#define GAME_GUI_HOST_SCREEN_MAP_SELECTION_WINDOW_H
 

class  Font;
class  Client;
class  HostScreen1;
class  ScrollFrame;

struct MapSelectorButtonOnClick;
struct CycleGameModeLeftButtonOnClick;
struct CycleGameModeRightButtonOnClick;

#include "../RoundedWindow.h"
#include <string>

#include "../../map/MapEntry.h"
#include "../../modes/GameplayMode.h"

class MapSelectionWindow final: public RoundedWindow {

    friend struct MapSelectorButtonOnClick;
    friend struct CycleGameModeLeftButtonOnClick;
    friend struct CycleGameModeRightButtonOnClick;
    friend class  Client;
    friend class  HostScreen1;

    struct ButtonPtr final {
        HostScreen1*   hostScreen;
        MapEntryData  mapChoice;
        ButtonPtr() {
            hostScreen = nullptr;
        }
        ~ButtonPtr() {}
    };

    private:
        HostScreen1&             m_HostScreen;
        Button*                  m_ChangeGameModeLeftButton;
        Button*                  m_ChangeGameModeRightButton;
        ScrollFrame*             m_MapFileScrollFrame;
        Font&                    m_Font;

        void clear_chosen_map();
        void recalculate_maps();
    public:
        MapSelectionWindow(HostScreen1&, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const std::string& labelText);
        ~MapSelectionWindow();

        void clearWindow();

        void onResize(const unsigned int& newWidth, const unsigned int& newHeight);

        void update(const double& dt);
        void render();
};



#endif