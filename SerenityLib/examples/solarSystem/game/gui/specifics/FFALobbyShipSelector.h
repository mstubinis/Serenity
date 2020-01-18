#pragma once
#ifndef GAME_GUI_FFA_LOBBY_SHIP_SELECTOR_H
#define GAME_GUI_FFA_LOBBY_SHIP_SELECTOR_H

class  Font;
class  Client;
class  LobbyScreenFFA;
class  ScrollFrame;
class  Text;
class  Button;

struct ShipTokenOnClick;
struct PacketConnectionAccepted;

#include <string>

class FFALobbyShipSelector final {
    friend struct ShipTokenOnClick;
    friend class  Client;
    friend class  LobbyScreenFFA;
    private:
        LobbyScreenFFA&    m_FFALobby;
        ScrollFrame*       m_ShipsFrame;
        Font&              m_Font;
        std::string        m_ChosenShipClass;

        void init_window_contents(const PacketConnectionAccepted& packet);
    public:
        FFALobbyShipSelector(LobbyScreenFFA&, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const std::string& labelText);
        ~FFALobbyShipSelector();

        const std::string& getChosenShipClass() const;

        void clearChosenShipClass();
        void clear();

        void setPosition(const float x, const float y);
        void setSize(const float width, const float height);

        void update(const double& dt);
        void render();
};

#endif