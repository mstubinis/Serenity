#include "Menu.h"
#include "map/Map.h"
#include "map/Anchor.h"
#include "Core.h"
#include "networking/server/Server.h"
#include "networking/client/Client.h"
#include "networking/packets/PacketMessage.h"
#include "ResourceManifest.h"
#include "GameCamera.h"

#include <core/engine/system/Engine.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/system/window/Engine_Window.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/materials/Material.h>

#include <glm/vec4.hpp>
#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string.hpp>

#include "gui/Button.h"
#include "gui/TextBox.h"
#include "gui/ScrollFrame.h"
#include "gui/Text.h"
#include "gui/specifics/ServerLobbyChatWindow.h"
#include "gui/specifics/ServerLobbyConnectedPlayersWindow.h"
#include "gui/specifics/ServerLobbyShipSelectorWindow.h"

#include "gui/specifics/MainMenu.h"
#include "gui/specifics/HostScreen.h"

#include "gui/specifics/HostScreenFFA2.h"
#include "gui/specifics/HostScreenTeamDeathmatch2.h"
#include "gui/specifics/HostScreenHomelandSecurity2.h"

#include "hud/HUD.h"
#include "modes/GameplayMode.h"
#include "teams/Team.h"

#include <regex>
#include <core/engine/sounds/Sounds.h>

#include <core/engine/discord/Discord.h>

using namespace Engine;
using namespace std;

struct ButtonBack_OnClick {void operator()(Button* button) const {
    Menu& menu = *static_cast<Menu*>(button->getUserPointer());
    switch (menu.m_GameState) {
        case GameState::Join_Screen_Setup_1: {
            //force server to disconnect client
            menu.m_Core.shutdownClient();

            menu.go_to_main_menu();
            break;
        }case GameState::Join_Screen_Lobby_2: {
            //force server to disconnect client
            menu.m_Core.shutdownClient();
            menu.m_ServerLobbyShipSelectorWindow->setShipViewportActive(false);

            menu.m_GameState = GameState::Join_Screen_Setup_1;

            menu.m_Next->setText("Next");
            break;
        }default: {
            menu.go_to_main_menu();
            break;
        }
    }
}};
struct ButtonNext_OnClick {void operator()(Button* button) const {
    Menu& menu = *static_cast<Menu*>(button->getUserPointer());
    switch (menu.m_GameState) {
        case GameState::Host_Screen_Lobby_3: {
            menu.enter_the_game();
            break;
        }case GameState::Join_Screen_Setup_1: {
            const string& username   = menu.m_UserName->text();
            const string& portstring = menu.m_ServerPort->text();
            const string& ip         = menu.m_ServerIp->text();
            if (!portstring.empty() && !ip.empty() && !username.empty()) {
                //TODO: prevent special characters in usename
                if (std::regex_match(portstring, std::regex("^(0|[1-9][0-9]*)$"))) {
                    if (username.find_first_not_of(' ') != std::string::npos) {
                        if (std::regex_match(username, std::regex("[a-zA-ZäöüßÄÖÜ]+"))) {
                            const int port = stoi(portstring);
                            menu.m_Core.startClient(nullptr, port, username, ip); //the client will request validation at this stage

                            menu.m_ServerLobbyChatWindow->setUserPointer(menu.m_Core.getClient());
                        }else {
                            menu.setErrorText("The username must only contain letters");
                        }
                    }else {
                        menu.setErrorText("The username must have some letters in it");
                    }

                }else{
                    menu.setErrorText("Server port must contain numbers only");
                }
            }else{
                menu.setErrorText("Do not leave any fields blank");
            }
            break;
        }case GameState::Join_Screen_Lobby_2: { 
            menu.enter_the_game();
            break;
        }default: {
            menu.go_to_main_menu();
            break;
        }
    }
}};

Menu::Menu(Scene& menu_scene, Camera& game_camera, GameState::State& _state, Core& core):m_GameState(_state),m_Core(core){
    m_FontHandle = Resources::addFont(ResourceManifest::BasePath + "data/Fonts/consolas.fnt");
    m_Font = Resources::getFont(m_FontHandle);
    Engine::Math::setColor(m_Color, 255.0f, 255.0f, 0.0f);
    m_MessageText = "";
    m_ErrorTimer = 0.0f;

    const auto& windowDimensions = Resources::getWindowSize();
    
    m_MainMenuScreen = new MainMenu(*this, *m_Font, 0.1f);
    m_HostScreen = new HostScreen(*this, *m_Font);

    m_HostScreenFFA2 = new HostScreenFFA2(*m_HostScreen , *this, *m_Font);
    m_HostScreenTeamDeathmatch2 = new HostScreenTeamDeathmatch2(*m_HostScreen, *this, *m_Font);
    m_HostScreenHomelandSecurity2 = new HostScreenHomelandSecurity2(*m_HostScreen, *this, *m_Font);

    //todo: remove
    m_Back = NEW Button(*m_Font, 100.0f, 50.0f, 150.0f, 50.0f);
    m_Back->setText("Back");
    m_Back->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_Back->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    m_Next = NEW Button(*m_Font, windowDimensions.x - 100.0f, 50.0f, 150.0f, 50.0f);
    m_Next->setText("Next");
    m_Next->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_Next->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    m_Back->setUserPointer(this);
    m_Next->setUserPointer(this);

    m_Back->setOnClickFunctor(ButtonBack_OnClick());
    m_Next->setOnClickFunctor(ButtonNext_OnClick());
    m_ServerIp = NEW TextBox("Server IP",*m_Font, 40, windowDimensions.x / 2.0f, 115.0f);
    m_ServerIp->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_ServerIp->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    m_UserName = NEW TextBox("Your Name",*m_Font, 20, windowDimensions.x / 2.0f, 275.0f);
    m_UserName->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_UserName->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    m_ServerPort = NEW TextBox("Server Port",*m_Font, 7, windowDimensions.x / 2.0f, 195.0f);
    m_ServerPort->setColor(0.5f, 0.5f, 0.5f, 1.0f);
    m_ServerPort->setTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    m_ServerPort->setText("55000");

    m_InfoText = NEW Text(Resources::getWindowSize().x / 2.0f, 65.0f, *m_Font);
    m_InfoText->setTextAlignment(TextAlignment::Center);
    
    m_ServerLobbyChatWindow = NEW ServerLobbyChatWindow(*m_Font, 50.0f, 140.0f + 300.0f);
    m_ServerLobbyChatWindow->setColor(1.0f, 1.0f, 0.0f, 1.0f);
    
    m_ServerLobbyConnectedPlayersWindow = NEW ServerLobbyConnectedPlayersWindow(*m_Font, 50.0f + m_ServerLobbyChatWindow->getWindowFrame().width(), 140.0f + 300.0f);
    m_ServerLobbyConnectedPlayersWindow->setColor(1.0f, 1.0f, 0.0f, 1.0f);
    
    m_ServerLobbyShipSelectorWindow = NEW ServerLobbyShipSelectorWindow(core, menu_scene, game_camera, *m_Font, 50.0f, windowDimensions.y - 50.0f, 578, 270);
    m_ServerLobbyShipSelectorWindow->setColor(1.0f, 1.0f, 0.0f, 1.0f);

    go_to_main_menu();  
}
Menu::~Menu() {
    SAFE_DELETE(m_MainMenuScreen);
    SAFE_DELETE(m_HostScreen);

    SAFE_DELETE(m_HostScreenFFA2);
    SAFE_DELETE(m_HostScreenTeamDeathmatch2);
    SAFE_DELETE(m_HostScreenHomelandSecurity2);

    SAFE_DELETE(m_Back);
    SAFE_DELETE(m_Next);
    SAFE_DELETE(m_ServerIp);
    SAFE_DELETE(m_UserName);
    SAFE_DELETE(m_ServerPort);
    SAFE_DELETE(m_InfoText);
    SAFE_DELETE(m_ServerLobbyChatWindow);
    SAFE_DELETE(m_ServerLobbyConnectedPlayersWindow);
    SAFE_DELETE(m_ServerLobbyShipSelectorWindow);
}
void Menu::setGameState(const GameState::State& state) {
    m_GameState = state;
}
const GameState::State& Menu::getGameState() const {
    return m_GameState;
}
Font& Menu::getFont() {
    return *m_Font;
}
Core& Menu::getCore() {
    return m_Core;
}
void Menu::enter_the_game() {
    if (!m_ServerLobbyShipSelectorWindow->getShipClass().empty()) {
        PacketMessage p;
        p.PacketType = PacketType::Client_To_Server_Request_Map_Entry;
        p.name = m_Core.m_Client->m_Username;

        p.data = m_ServerLobbyShipSelectorWindow->getShipClass(); //ship class [0]

        p.data += "," + m_Core.m_Client->m_MapSpecificData.m_Map->name(); //map name [1]
        if (m_Core.m_Client->m_MapSpecificData.m_Team) {
            p.data += "," + m_Core.m_Client->m_MapSpecificData.m_Team->getTeamNumberAsString();
        }else{
            p.data += ",-1"; //error, the client did not choose or was not assigned a team yet
        }

        m_Core.m_Client->send(p);
    }else{
        setErrorText("You must choose your ship", 5.0f);
    }
}

void Menu::go_to_main_menu() {
    m_GameState = GameState::Main_Menu;
    m_ServerLobbyShipSelectorWindow->setShipViewportActive(false);
    m_Next->setText("Next");
    setErrorText("", 0.0f);

    auto& window = Resources::getWindow();
    window.keepMouseInWindow(false);
    window.setMouseCursorVisible(true);

    Discord::DiscordActivity activity;
    activity.setDetail("At Main Menu");
    activity.setType(discord::ActivityType::Playing);
    activity.setInstance(false);
    activity.setState("");
    activity.setTimestampStart(0);
    activity.setTimestampEnd(0);
    activity.setImageLarge("large_icon");
    activity.setImageLargeText("");
    activity.setImageSmallText("");
    Discord::update_activity(activity);
}


void Menu::setGoodText(const string& text, const float errorTime) {
    m_InfoText->setText(text);
    m_ErrorTimer = errorTime;
    m_InfoText->setColor(0.0f, 1.0f, 0.0f, 1.0f);
}
void Menu::setNormalText(const string& text, const float errorTime) {
    m_InfoText->setText(text);
    m_ErrorTimer = errorTime;
    m_InfoText->setColor(1.0f, 1.0f, 0.0f, 1.0f);
}
void Menu::setErrorText(const string& text, const float errorTime) {
    m_InfoText->setText(text);
    m_ErrorTimer = errorTime;
    m_InfoText->setColor(1.0f, 0.0f, 0.0f, 1.0f);
}

void Menu::onResize(const uint& width, const uint& height) {
    m_MainMenuScreen->onResize(width, height);

    m_HostScreen->onResize(width, height);
    m_HostScreenFFA2->onResize(width, height);
    m_HostScreenTeamDeathmatch2->onResize(width, height);
    m_HostScreenHomelandSecurity2->onResize(width, height);

    m_Back->setPosition(100.0f, 50.0f);
    m_Next->setPosition(width - 100.0f, 50.0f);

    m_ServerIp->setPosition(width / 2.0f, 115.0f);
    m_ServerPort->setPosition(width / 2.0f, 275.0f);
    m_UserName->setPosition(width / 2.0f, 195.0f);

    m_InfoText->setPosition(width / 2.0f, 65.0f);


    m_ServerLobbyChatWindow->setPosition(50.0f, 140.0f + 300.0f);
    m_ServerLobbyConnectedPlayersWindow->setPosition(50.0f + m_ServerLobbyChatWindow->getWindowFrame().width(), 140.0f + 300.0f);

    const auto& winSize = Resources::getWindowSize();
    m_ServerLobbyShipSelectorWindow->setPosition(50, winSize.y - 50.0f);
}

#pragma region updates
void Menu::update_game(const double& dt) {

}
void Menu::update_main_menu(const double& dt) {
    m_Font->renderText(epriv::Core::m_Engine->m_DebugManager.reportDebug(), glm::vec2(50), glm::vec4(1.0), 0);

    m_MainMenuScreen->update(dt);
}
void Menu::update_host_setup_1(const double& dt) {
    m_HostScreen->update(dt);
}
void Menu::update_host_setup_ffa_2(const double& dt) {
    m_HostScreenFFA2->update(dt);
}
void Menu::update_host_setup_td_2(const double& dt) {
    m_HostScreenTeamDeathmatch2->update(dt);
}
void Menu::update_host_setup_hs_2(const double& dt) {
    m_HostScreenHomelandSecurity2->update(dt);
}

void Menu::update_host_lobby_3(const double& dt) {
    m_ServerLobbyChatWindow->update(dt);
    m_ServerLobbyConnectedPlayersWindow->update(dt);
    m_ServerLobbyShipSelectorWindow->update(dt);

    m_Back->update(dt);
    m_Next->update(dt);
}

void Menu::update_join_setup_1(const double& dt) {
    m_Back->update(dt);
    m_Next->update(dt);

    m_ServerIp->update(dt);
    m_ServerPort->update(dt);
    m_UserName->update(dt);
}
void Menu::update_join_lobby_2(const double& dt) {
    m_ServerLobbyChatWindow->update(dt);
    m_ServerLobbyConnectedPlayersWindow->update(dt);
    m_ServerLobbyShipSelectorWindow->update(dt);

    m_Back->update(dt);
    m_Next->update(dt);
}

void Menu::update_options_main(const double& dt) {

}
void Menu::update_options_sounds(const double& dt) {

}
void Menu::update_options_graphics(const double& dt) {

}
void Menu::update_options_keybinds(const double& dt) {

}

void Menu::update_encyclopedia_main(const double& dt) {

}
void Menu::update_encyclopedia_technology(const double& dt) {

}
void Menu::update_encyclopedia_ships(const double& dt) {

}
void Menu::update_encyclopedia_factions(const double& dt) {

}
#pragma endregion




void Menu::render_game() {

}
void Menu::render_main_menu() {
    m_MainMenuScreen->render();
}
void Menu::render_host_setup_1() {
    m_HostScreen->render();
}
void Menu::render_host_setup_ffa_2() {
    m_HostScreenFFA2->render();
}
void Menu::render_host_setup_td_2() {
    m_HostScreenTeamDeathmatch2->render();
}
void Menu::render_host_setup_hs_2() {
    m_HostScreenHomelandSecurity2->render();
}
void Menu::render_host_lobby_3() {
    m_ServerLobbyChatWindow->render();
    m_ServerLobbyConnectedPlayersWindow->render();
    m_ServerLobbyShipSelectorWindow->render();

    m_Back->render();
    m_Next->render();
}

void Menu::render_join_setup_1() {
    m_Back->render();
    m_Next->render();

    m_ServerIp->render();
    m_ServerPort->render();
    m_UserName->render();
}
void Menu::render_join_lobby_2() {
    m_ServerLobbyChatWindow->render();
    m_ServerLobbyConnectedPlayersWindow->render();
    m_ServerLobbyShipSelectorWindow->render();

    m_Back->render();
    m_Next->render();
}

void Menu::render_options_main() {

}
void Menu::render_options_sounds() {

}
void Menu::render_options_graphics() {

}
void Menu::render_options_keybinds() {

}

void Menu::render_encyclopedia_main() {

}
void Menu::render_encyclopedia_technology() {

}
void Menu::render_encyclopedia_ships() {

}
void Menu::render_encyclopedia_factions() {

}


void Menu::update(const double& dt) {
    switch (m_GameState) {
        case GameState::Main_Menu: {
            update_main_menu(dt); break;
        }case GameState::Host_Screen_Setup_1: {
            update_host_setup_1(dt); break;
        }case GameState::Host_Screen_Setup_FFA_2: {
            update_host_setup_ffa_2(dt); break;
        }case GameState::Host_Screen_Setup_TeamDeathMatch_2: {
            update_host_setup_td_2(dt); break;
        }case GameState::Host_Screen_Setup_HomelandSecurity_2: {
            update_host_setup_hs_2(dt); break;
        }case GameState::Host_Screen_Lobby_3: {
            update_host_lobby_3(dt); break;
        }case GameState::Join_Screen_Setup_1: {
            update_join_setup_1(dt); break;
        }case GameState::Join_Screen_Lobby_2: {
            update_join_lobby_2(dt); break;
        }case GameState::Encyclopedia_Main: {
            update_encyclopedia_main(dt); break;
        }case GameState::Encyclopedia_Ships: {
            update_encyclopedia_ships(dt); break;
        }case GameState::Encyclopedia_Factions: {
            update_encyclopedia_factions(dt); break;
        }case GameState::Encyclopedia_Technology: {
            update_encyclopedia_technology(dt); break;
        }case GameState::Options_Main: {
            update_options_main(dt); break;
        }case GameState::Options_Graphics: {
            update_options_graphics(dt); break;
        }case GameState::Options_Sounds: {
            update_options_sounds(dt); break;
        }case GameState::Options_Keybinds: {
            update_options_keybinds(dt); break;
        }case GameState::Game: {
            update_game(dt); break;
        }default: {
            break;
        }
    }
    if (m_ErrorTimer > 0.0f) {
        m_ErrorTimer -= static_cast<float>(dt);
        if (m_ErrorTimer < 0.0f) {
            setErrorText("", 0.0f);
            m_ErrorTimer = 0.0f;
        }
    }
}
void Menu::render() {
    switch (m_GameState) {
        case GameState::Main_Menu: {
            render_main_menu(); break;
        }case GameState::Host_Screen_Setup_1: {
            render_host_setup_1(); break;
        }case GameState::Host_Screen_Setup_FFA_2: {
            render_host_setup_ffa_2(); break;
        }case GameState::Host_Screen_Setup_TeamDeathMatch_2: {
            render_host_setup_td_2(); break;
        }case GameState::Host_Screen_Setup_HomelandSecurity_2: {
            render_host_setup_hs_2(); break;
        }case GameState::Host_Screen_Lobby_3: {
            render_host_lobby_3(); break;
        }case GameState::Join_Screen_Setup_1: {
            render_join_setup_1(); break;
        }case GameState::Join_Screen_Lobby_2: {
            render_join_lobby_2(); break;
        }case GameState::Encyclopedia_Main: {
            render_encyclopedia_main(); break;
        }case GameState::Encyclopedia_Ships: {
            render_encyclopedia_ships(); break;
        }case GameState::Encyclopedia_Factions: {
            render_encyclopedia_factions(); break;
        }case GameState::Encyclopedia_Technology: {
            render_encyclopedia_technology(); break;
        }case GameState::Options_Main: {
            render_options_main(); break;
        }case GameState::Options_Graphics: {
            render_options_graphics(); break;
        }case GameState::Options_Sounds: {
            render_options_sounds(); break;
        }case GameState::Options_Keybinds: {
            render_options_keybinds(); break;
        }case GameState::Game: {
            render_game(); break;
        }default: {
            break;
        }
    }
    m_InfoText->render();
}
