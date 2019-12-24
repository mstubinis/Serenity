#pragma once
#ifndef GAME_SERVER_HOSTING_MAP_SELECTOR_WINDOW_H
#define GAME_SERVER_HOSTING_MAP_SELECTOR_WINDOW_H

class  HostScreen;
class  Font;
class  ScrollFrame;
class  Widget;
class  Text;
class  Client;
class  Button;
struct MapSelectorButtonOnClick;
struct CycleGameModeLeftButtonOnClick;
struct CycleGameModeRightButtonOnClick;

#include "../../map/MapEntry.h"
#include "../../modes/GameplayMode.h"

class ServerHostingMapSelectorWindow final {
    friend struct MapSelectorButtonOnClick;
    friend struct CycleGameModeLeftButtonOnClick;
    friend struct CycleGameModeRightButtonOnClick;
    friend class  Client;

    struct ButtonPtr final {
        HostScreen*                                       hostScreen;
        MapEntryData                                      mapChoice;
        ButtonPtr() {
            hostScreen = nullptr;
        }
    };

    private:
        HostScreen&                                         m_HostScreen;
        GameplayModeType::Mode                              m_CurrentGameMode;
        Button*                                             m_ChangeGameModeLeftButton;
        Button*                                             m_ChangeGameModeRightButton;

        MapEntryData                                        m_CurrentChoice;
        ScrollFrame*                                        m_MapFileScrollFrame;
        Font&                                               m_Font;
        void*                                               m_UserPointer;

        void clear_chosen_map();
        void recalculate_maps();
    public:
        ServerHostingMapSelectorWindow(HostScreen&, const Font& font, const float x, const float y, const float w, const float h);
        ~ServerHostingMapSelectorWindow();

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setPosition(const float x, const float y);

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void setSize(const float& w, const float& h);

        void setUserPointer(void*);
        void* getUserPointer();
        void clear();

        void addContent(Widget* widget);

        const MapEntryData& getCurrentChoice() const;
        Font& getFont();
        ScrollFrame& getWindowFrame();

        void update(const double& dt);
        void render();
};

#endif