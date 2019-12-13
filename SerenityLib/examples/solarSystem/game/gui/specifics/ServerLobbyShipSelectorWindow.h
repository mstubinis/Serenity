#pragma once
#ifndef GAME_SERVER_LOBBY_SHIP_SELECTOR_WINDOW_H
#define GAME_SERVER_LOBBY_SHIP_SELECTOR_WINDOW_H

#include "../ScrollWindow.h"
#include "Ship3DViewer.h"
#include "../Text.h"
#include "../Button.h"

class  Scene;
class  Core;
class  Client;
struct ShipSelectorButtonOnClick;
struct ButtonNext_OnClick;
class  Menu;
class ServerLobbyShipSelectorWindow final {
    friend struct ShipSelectorButtonOnClick;
    friend struct ButtonNext_OnClick;
    friend class  Menu;
    friend class  Client;
    private:
        Ship3DViewer*    m_3DViewer;
        ScrollFrame*     m_ShipWindow;

        Font&            m_Font;
        Core&            m_Core;
        float            m_Width;
        float            m_Height;
    public:
        ServerLobbyShipSelectorWindow(Core& core, Scene& scene,Camera& camera, const Font& font, const float x, const float y);
        ~ServerLobbyShipSelectorWindow();

        void addShipButton(const std::string& shipClass);

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setPosition(const float x, const float y);

        void setShipClass(const std::string& ship_class);
        void clear();

        void setShipViewportActive(const bool& active);
        void addContent(Widget* widget);
        const std::string& getShipClass() const;

        Font& getFont();
        ScrollFrame& getWindowFrame();

        void update(const double& dt);
        void render();
};

#endif