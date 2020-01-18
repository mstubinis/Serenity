#pragma once
#ifndef GAME_GUI_LOBBY_FFA_H
#define GAME_GUI_LOBBY_FFA_H

class  Button;
class  Font;
class  Menu;
class  Core;
class  RoundedWindow;
class  FFALobbyShipSelector;
class  ShipDescription;
class  Ship3DViewer;
class  MessageWithArrow;
class  Scene;
class  Camera;

struct FFALobbyBackOnClick;
struct FFALobbyForwardOnClick;
struct PacketConnectionAccepted;

class LobbyScreenFFA final {
    friend struct FFALobbyBackOnClick;
    friend struct FFALobbyForwardOnClick;
    private:
        bool m_IsHost;
        Menu& m_Menu;
        Core& m_Core;
        Font& m_Font;

        double m_TimeLeftUntilMatchStarts;
        /*
        ChatRoomMessages
        ChatRoomPlayerList
        */

        RoundedWindow*                m_RoundedWindow;

        Button*                       m_BackgroundEdgeGraphicBottom;
        Button*                       m_BackButton;
        Button*                       m_ForwardButton; //host only

        FFALobbyShipSelector*         m_ShipSelector;
        Ship3DViewer*                 m_Ship3DViewer;
        ShipDescription*              m_ShipDescription;
        MessageWithArrow*             m_ChooseShipMessage;

    public:
        LobbyScreenFFA(Font& font, Menu& menu, Core& core, Scene& scene, Camera& camera);
        ~LobbyScreenFFA();

        FFALobbyShipSelector& getShipSelector();
        Ship3DViewer& getShip3DViewer();
        ShipDescription& getShipDescriptionWindow();
        MessageWithArrow& getChooseShipMessageDisplay();

        void initShipSelector(const PacketConnectionAccepted& packet);
        void clearShipSelector();

        const bool setChosenShipClass(const std::string& shipClass);
        void setTimeLeftUntilMatchStartsInSeconds(const double& seconds);

        void showShipViewer();
        void hideShipViewer();

        void setTopLabelText(const std::string&);
        void setHost(const bool host);

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void update(const double& dt);
        void render();

        const bool isHost() const;
};

#endif