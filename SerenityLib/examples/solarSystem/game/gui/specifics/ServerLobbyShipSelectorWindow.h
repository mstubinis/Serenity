#pragma once
#ifndef GAME_SERVER_LOBBY_SHIP_SELECTOR_WINDOW_H
#define GAME_SERVER_LOBBY_SHIP_SELECTOR_WINDOW_H

#include "../ScrollWindow.h"
#include "../Text.h"
#include "../Button.h"

class Viewport;
class Scene;
class Camera;
class ServerLobbyShipSelectorWindow final {
    private:
        ScrollFrame* m_ShipWindow;
        Viewport*    m_ShipDisplay;
        Font&        m_Font;
        float        m_Width;
        float        m_Height;
        void*        m_UserPointer;
    public:
        ServerLobbyShipSelectorWindow(const Scene& scene,const Camera& camera, const Font& font, const float& x, const float& y);
        ~ServerLobbyShipSelectorWindow();

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setPosition(const float& x, const float& y);

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