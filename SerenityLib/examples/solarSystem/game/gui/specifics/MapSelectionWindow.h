#pragma once
#ifndef GAME_GUI_HOST_SCREEN_MAP_SELECTION_WINDOW_H
#define GAME_GUI_HOST_SCREEN_MAP_SELECTION_WINDOW_H
 

class  Font;
class  Client;
class  HostScreen;
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

    struct ButtonPtr final {
        HostScreen*   hostScreen;
        MapEntryData  mapChoice;
        ButtonPtr() {
            hostScreen = nullptr;
        }
        ~ButtonPtr() {}
    };

    private:
        HostScreen&              m_HostScreen;
        GameplayModeType::Mode   m_CurrentGameMode;
        Button*                  m_ChangeGameModeLeftButton;
        Button*                  m_ChangeGameModeRightButton;
        MapEntryData             m_CurrentChoice;
        ScrollFrame*             m_MapFileScrollFrame;
        Font&                    m_Font;

        void clear_chosen_map();
        void recalculate_maps();
    public:
        MapSelectionWindow(HostScreen&, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const float borderSize, const std::string& labelText);
        ~MapSelectionWindow();

        const MapEntryData& getCurrentChoice() const;

        void clearWindow();

        void setCurrentGameMode(const GameplayModeType::Mode& currentGameMode);

        void onResize(const unsigned int& newWidth, const unsigned int& newHeight);

        void update(const double& dt);
        void render();
};



#endif