#pragma once
#ifndef GAME_HUD_H
#define GAME_HUD_H

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <core/engine/resources/Engine_Resources.h>

#include "GameState.h"

class  Font;
class  Ship;
class  Core;
class  SoundMusic;
struct Entity;

class  Client;
class  Server;

class  Button;
class  TextBox;
class  Text;
class  ScrollFrame;
class  ServerLobbyChatWindow;
class  ServerLobbyConnectedPlayersWindow;
class  ServerLobbyShipSelectorWindow;
class  ServerHostingMapSelectorWindow;


struct ButtonHost_OnClick;
struct ButtonJoin_OnClick;
struct ButtonBack_OnClick;
struct ButtonNext_OnClick;

class HUD final{
    friend struct ButtonHost_OnClick;
    friend struct ButtonJoin_OnClick;
    friend struct ButtonBack_OnClick;
    friend struct ButtonNext_OnClick;
    friend class Core;
    friend class Client;
    friend class Server;
    private:
        glm::vec3                      m_Color;
        Handle                         m_FontHandle;
        Font*                          m_Font;
        bool                           m_Active;
        GameState::State&              m_GameState;
        Core&                          m_Core;

        std::string                    m_MessageText;
        float                          m_ErrorTimer;

        Button*                        m_ButtonHost;
        Button*                        m_ButtonJoin;

        Button*                        m_Back;
        Button*                        m_Next;

        TextBox*                       m_ServerIp;
        TextBox*                       m_UserName;
        TextBox*                       m_ServerPort;
        Text*                          m_InfoText;

        SoundMusic*                    m_MainMenuMusic;


        ServerHostingMapSelectorWindow*     m_ServerHostMapSelector;

        ServerLobbyChatWindow*              m_ServerLobbyChatWindow;
        ServerLobbyConnectedPlayersWindow*  m_ServerLobbyConnectedPlayersWindow;
        ServerLobbyShipSelectorWindow*      m_ServerLobbyShipSelectorWindow;

        void go_to_main_menu();
        void enter_the_game();

        void update_game(const double& dt);
        void update_main_menu(const double& dt);
        void update_host_server_lobby_and_ship(const double& dt);
        void update_host_server_port_and_name_and_map(const double& dt);
        void update_join_server_port_and_name_and_ip(const double& dt);
        void update_join_server_server_lobby(const double& dt);

        void render_game();
        void render_main_menu();
        void render_host_server_lobby_and_ship();
        void render_host_server_port_and_name_and_map();
        void render_join_server_port_and_name_and_ip();
        void render_join_server_server_lobby();

    public:
        HUD(Scene& scene, Camera& camera, GameState::State& current, Core& core);
        ~HUD();

        void onResize(const uint& width, const uint& height);

        void setGoodText(const std::string& error, const float errorTime = 3.0f);
        void setErrorText(const std::string& error, const float errorTime = 3.0f);
        void setNormalText(const std::string& error, const float errorTime = 3.0f);

        void update(const double& dt);
        void render();
};
#endif