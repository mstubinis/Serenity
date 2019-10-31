#pragma once
#ifndef GAME_SERVER_LOBBY_SHIP_SELECTOR_WINDOW_H
#define GAME_SERVER_LOBBY_SHIP_SELECTOR_WINDOW_H

#include "../ScrollWindow.h"
#include "../Text.h"
#include "../Button.h"

class  Viewport;
class  Scene;
class  Camera;
class  Core;
class  Client;
struct GameCameraShipSelectorLogicFunctor;
struct ShipSelectorButtonOnClick;
struct ButtonNext_OnClick;
class  Menu;
class ServerLobbyShipSelectorWindow final {
    friend struct GameCameraShipSelectorLogicFunctor;
    friend struct ShipSelectorButtonOnClick;
    friend struct ButtonNext_OnClick;
    friend class  Menu;
    friend class  Client;
    private:
        bool         m_IsCurrentlyDragging;
        bool         m_IsCurrentlyOverShipWindow;
        ScrollFrame* m_ShipWindow;
        Viewport*    m_ShipDisplay;
        Font&        m_Font;
        Core&        m_Core;
        float        m_Width;
        float        m_Height;
        void*        m_UserPointer;
        std::string  m_ChosenShipName;
    public:
        ServerLobbyShipSelectorWindow(Core& core, Scene& scene,Camera& camera, const Font& font, const float x, const float y);
        ~ServerLobbyShipSelectorWindow();

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setPosition(const float x, const float y);

        void setShipViewportActive(const bool& active);

        void setUserPointer(void*);
        void* getUserPointer();
        void clear();

        void addContent(Widget* widget);

        Font& getFont();
        ScrollFrame& getWindowFrame();
        const Viewport& getShipDisplay() const;

        void update(const double& dt);
        void render();
};

#endif