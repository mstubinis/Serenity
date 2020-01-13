#pragma once
#ifndef GAME_GUI_LOBBY_FFA_H
#define GAME_GUI_LOBBY_FFA_H

class Button;
class Font;
class LobbyScreenFFA final {
    private:
        bool m_IsHost;
        /*
        TimeLeftUntilMatchStarts

        ShipSelectionIconSheet
        ShipDescription
        Ship3DView

        ChatRoomMessages
        ChatRoomPlayerList
        */
        Button*                       m_BackButton;
        Button*                       m_ForwardButton; //host only

    public:
        LobbyScreenFFA(Font& font, const bool isHost = false);
        ~LobbyScreenFFA();

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void update(const double& dt);
        void render();

        const bool isHost() const;
};

#endif