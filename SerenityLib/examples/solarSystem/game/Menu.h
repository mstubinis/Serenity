#pragma once
#ifndef GAME_MENU_H
#define GAME_MENU_H

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <core/engine/resources/Engine_Resources.h>

#include "GameState.h"

class  Font;
class  Ship;
class  Map;
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

struct ButtonBack_OnClick;
struct ButtonNext_OnClick;
struct Host_ButtonBack_OnClick;
struct Host_ButtonNext_OnClick;
struct Host1Persistent_ButtonNext_OnClick;

class  MainMenu;

class  HostScreen0;
class  HostScreen1;
class  HostScreen1Persistent;

class  HostScreenFFA2;
class  HostScreenTeamDeathmatch2;
class  HostScreenHomelandSecurity2;

class  LobbyScreenFFA;
class  LobbyScreenTeamDeathmatch;
class  LobbyScreenHomelandSecurity;

class Menu final{
    friend struct Host_ButtonBack_OnClick;
    friend struct Host_ButtonNext_OnClick;
    friend struct Host1Persistent_ButtonNext_OnClick;
    friend struct ButtonBack_OnClick;
    friend struct ButtonNext_OnClick;
    friend class  Core;
    friend class  Map;
    friend class  Client;
    friend class  Server;

    public:
        ServerLobbyChatWindow*             m_ServerLobbyChatWindow;
        ServerLobbyConnectedPlayersWindow* m_ServerLobbyConnectedPlayersWindow;
        //ServerLobbyShipSelectorWindow*     m_ServerLobbyShipSelectorWindow;

    private:
        glm::vec3                      m_Color;
        Handle                         m_FontHandle;
        Font*                          m_Font;
        GameState::State&              m_GameState;
        Core&                          m_Core;

        std::string                    m_MessageText;
        float                          m_ErrorTimer;

        MainMenu*                      m_MainMenuScreen;

        HostScreen0*                   m_HostScreen0;
        HostScreen1*                   m_HostScreen1;
        HostScreen1Persistent*         m_HostScreen1Persistent;

        //host part 2
        HostScreenFFA2*                m_HostScreenFFA2;
        HostScreenTeamDeathmatch2*     m_HostScreenTeamDeathmatch2;
        HostScreenHomelandSecurity2*   m_HostScreenHomelandSecurity2;

        //host part 3
        LobbyScreenFFA* m_LobbyScreenFFA;
        //LobbyScreenTeamDeathmatch*      m_LobbyScreenTeamDeathmatch;
        //LobbyScreenHomelandSecurity*    m_LobbyScreenHomelandSecurity;


        Button*                        m_Back;
        Button*                        m_Next;

        TextBox*                       m_ServerIp;
        TextBox*                       m_UserName;
        TextBox*                       m_ServerPort;

        Text*                          m_InfoText;


        void update_game(const double& dt);
        void update_main_menu(const double& dt);

        void update_host_setup_0(const double& dt);
        void update_host_setup_1(const double& dt);
        void update_host_setup_1_persistent(const double& dt);

        void update_host_setup_ffa_2(const double& dt);
        void update_host_setup_td_2(const double& dt);
        void update_host_setup_hs_2(const double& dt);

        void update_host_lobby_td_3(const double& dt);
        void update_host_lobby_ffa_3(const double& dt);
        void update_host_lobby_hs_3(const double& dt);

        void update_join_setup_1(const double& dt);
        void update_join_lobby_2(const double& dt);
        void update_options_main(const double& dt);
        void update_options_sounds(const double& dt);
        void update_options_graphics(const double& dt);
        void update_options_keybinds(const double& dt);
        void update_encyclopedia_main(const double& dt);
        void update_encyclopedia_technology(const double& dt);
        void update_encyclopedia_ships(const double& dt);
        void update_encyclopedia_factions(const double& dt);

        void update_loading_screen(const double& dt);




        void render_game();
        void render_main_menu();

        void render_host_setup_0();
        void render_host_setup_1();
        void render_host_setup_1_persistent();

        void render_host_setup_ffa_2();
        void render_host_setup_td_2();
        void render_host_setup_hs_2();

        void render_host_lobby_ffa_3();
        void render_host_lobby_td_3();
        void render_host_lobby_hs_3();

        void render_join_setup_1();
        void render_join_lobby_2();
        void render_options_main();
        void render_options_sounds();
        void render_options_graphics();
        void render_options_keybinds();
        void render_encyclopedia_main();
        void render_encyclopedia_technology();
        void render_encyclopedia_ships();
        void render_encyclopedia_factions();

        void render_loading_screen();
    public:
        Menu(Scene& scene, Camera& camera, GameState::State& current, Core& core);
        ~Menu();

        void go_to_main_menu();
        void enter_the_game();

        void onResize(const uint& width, const uint& height);

        const GameState::State& getGameState() const;
        void setGameState(const GameState::State&);
        Font& getFont();
        Core& getCore();

        void setGoodText(const std::string& error, const float errorTime = 3.0f);
        void setErrorText(const std::string& error, const float errorTime = 3.0f);
        void setNormalText(const std::string& error, const float errorTime = 3.0f);

        void update(const double& dt);
        void render();
};
#endif