#pragma once
#ifndef GAME_GUI_HOST_SCREEN_1_H
#define GAME_GUI_HOST_SCREEN_1_H

class  Button;
class  Font;
class  Menu;
class  TextBox;
class  Text;
class  ServerHostingMapSelectorWindow;
class  RoundedWindow;
class  MapSelectionWindow;
class  MapDescriptionWindow;

class  HostScreenFFA2;
class  HostScreenTeamDeathmatch2;
class  HostScreenHomelandSecurity2;

#include "../../map/MapEntry.h"
#include "../../modes/GameplayMode.h"

class HostScreen1 final {
    friend class  HostScreenFFA2;
    friend class  HostScreenTeamDeathmatch2;
    friend class  HostScreenHomelandSecurity2;
    friend class  MapSelectionWindow;
    friend class  MapDescriptionWindow;

    private:
        Font&                      m_Font;
        Menu&                      m_Menu;

        Button*                    m_BackgroundEdgeGraphicBottom;
        Button*                    m_BackButton;
        Button*                    m_ForwardButton;

        MapSelectionWindow*        m_LeftWindow;
        MapDescriptionWindow*      m_RightWindow;

        bool m_IsPersistent;
    public:
        HostScreen1(Menu&, Font& font);
        ~HostScreen1();

        Menu& getMenu();

        void setPersistent();
        const bool isPersistent() const;

        void clearCurrentMapChoice();
        void setCurrentMapChoice(const MapEntryData& choice);
        void setCurrentGameMode(const GameplayModeType::Mode& currentGameMode);

        MapSelectionWindow& getMapSelectionWindow();
        MapDescriptionWindow& getMapDescriptionWindow();

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void update(const double& dt);
        void render();
};

#endif