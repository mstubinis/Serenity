#include "HUD.h"
#include "map/Map.h"
#include "Planet.h"
#include "Ship.h"
#include "Core.h"
#include "Server.h"
#include "Client.h"
#include "Packet.h"
#include "ResourceManifest.h"

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Camera.h>
#include <core/Material.h>


#include <glm/vec4.hpp>

#include "gui/Button.h"
#include "gui/TextBox.h"
#include "gui/ScrollWindow.h"
#include "gui/Text.h";
#include "gui/specifics/ServerLobbyChatWindow.h"
#include "gui/specifics/ServerLobbyConnectedPlayersWindow.h"
#include "gui/specifics/ServerLobbyShipSelectorWindow.h"
#include "gui/specifics/ServerHostingMapSelectorWindow.h"

#include <regex>


#include <core/engine/sounds/Sounds.h>

using namespace Engine;
using namespace std;


struct ButtonHost_OnClick { void operator()(Button* button) const {
    HUD& hud = *static_cast<HUD*>(button->getUserPointer());
    hud.m_GameState = GameState::Host_Server_Port_And_Name_And_Map;
    hud.setErrorText("", 0.2f);
}};
struct ButtonJoin_OnClick {void operator()(Button* button) const {
    HUD& hud = *static_cast<HUD*>(button->getUserPointer());
    hud.m_GameState = GameState::Join_Server_Port_And_Name_And_IP;
    hud.setErrorText("", 0.2f);
}};


struct ButtonBack_OnClick {void operator()(Button* button) const {
    HUD& hud = *static_cast<HUD*>(button->getUserPointer());
    switch (hud.m_GameState) {
        case GameState::Host_Server_Port_And_Name_And_Map:{
            hud.go_to_main_menu();

            //force server to disconnect client
            hud.m_Core.shutdownClient();
            hud.m_Core.shutdownServer();

            break;
        }case GameState::Host_Server_Lobby_And_Ship: {

            //force server to disconnect client
            hud.m_Core.shutdownClient();
            hud.m_Core.shutdownServer();

            hud.m_ServerLobbyShipSelectorWindow->setShipViewportActive(false);
            hud.m_GameState = GameState::Host_Server_Port_And_Name_And_Map;
            hud.m_Next->setText("Next");
            break;
        }case GameState::Join_Server_Port_And_Name_And_IP: {
            //force server to disconnect client
            hud.m_Core.shutdownClient();

            hud.go_to_main_menu();
            break;
        }case GameState::Join_Server_Server_Lobby: {
            //force server to disconnect client
            hud.m_Core.shutdownClient();
            hud.m_ServerLobbyShipSelectorWindow->setShipViewportActive(false);

            hud.m_GameState = GameState::Join_Server_Port_And_Name_And_IP;

            hud.m_Next->setText("Next");
            break;
        }default: {
            hud.go_to_main_menu();
            break;
        }
    }
}};
struct ButtonNext_OnClick {void operator()(Button* button) const {
    HUD& hud = *static_cast<HUD*>(button->getUserPointer());
    switch (hud.m_GameState) {
        case GameState::Host_Server_Port_And_Name_And_Map: {
            const string& username   = hud.m_UserName->text();
            const string& portstring = hud.m_ServerPort->text();
            const string& map        = hud.m_ServerHostMapSelector->getCurrentChoice().text();
            if (!portstring.empty() && !username.empty() && !map.empty()) {
                //TODO: prevent special characters in usename
                if (std::regex_match(portstring, std::regex("^(0|[1-9][0-9]*)$"))) {
                    if (username.find_first_not_of(' ') != std::string::npos) {
                        if (std::regex_match(username, std::regex("[a-zA-ZäöüßÄÖÜ]+"))) {
                            const int port = stoi(portstring);
                            hud.m_Core.startServer(port, map);
                            hud.m_Core.startClient(port, username, "127.0.01"); //the client will request validation at this stage
                            hud.m_ServerLobbyChatWindow->setUserPointer(hud.m_Core.getClient());
                        }else{
                            hud.setErrorText("The username must only contain letters");
                        }
                    }else{
                        hud.setErrorText("The username must have some letters in it");
                    }
                }else{
                    hud.setErrorText("Server port must contain numbers only");
                }
            }else{
                hud.setErrorText("Do not leave any fields blank");
            }
            break;
        }case GameState::Host_Server_Lobby_And_Ship: {
            hud.enter_the_game();
            break;
        }case GameState::Join_Server_Port_And_Name_And_IP: {
            const string& username   = hud.m_UserName->text();
            const string& portstring = hud.m_ServerPort->text();
            const string& ip         = hud.m_ServerIp->text();
            if (!portstring.empty() && !ip.empty() && !username.empty()) {
                //TODO: prevent special characters in usename
                if (std::regex_match(portstring, std::regex("^(0|[1-9][0-9]*)$"))) {
                    if (username.find_first_not_of(' ') != std::string::npos) {
                        if (std::regex_match(username, std::regex("[a-zA-ZäöüßÄÖÜ]+"))) {
                            hud.m_Core.startClient(stoi(portstring), username, ip); //the client will request validation at this stage
                            hud.m_ServerLobbyChatWindow->setUserPointer(hud.m_Core.getClient());
                        }else {
                            hud.setErrorText("The username must only contain letters");
                        }
                    }else {
                        hud.setErrorText("The username must have some letters in it");
                    }

                }else{
                    hud.setErrorText("Server port must contain numbers only");
                }
            }else{
                hud.setErrorText("Do not leave any fields blank");
            }
            break;
        }case GameState::Join_Server_Server_Lobby: { 
            hud.enter_the_game();
            break;
        }default: {
            hud.go_to_main_menu();
            break;
        }
    }
}};


HUD::HUD(Scene& scene, Camera& camera, GameState::State& _state, Core& core):m_GameState(_state),m_Core(core){
    m_FontHandle = Resources::addFont(ResourceManifest::BasePath + "data/Fonts/consolas.fnt");
    m_Font = Resources::getFont(m_FontHandle);
    Engine::Math::setColor(m_Color, 255, 255, 0);
    m_Active = true;
    m_MessageText = "";
    m_ErrorTimer = 0.0f;

    const auto& windowDimensions = Resources::getWindowSize();

    m_ButtonHost = new Button(*m_Font, glm::vec2(windowDimensions.x / 2, 275), 150, 50);
    m_ButtonHost->setText("Host");
    m_ButtonHost->setColor(0.5f, 0.5f, 0.5f, 1);
    m_ButtonHost->setTextColor(1, 1, 0, 1);
    m_ButtonJoin = new Button(*m_Font, glm::vec2(windowDimensions.x / 2, 155), 150, 50);
    m_ButtonJoin->setText("Join");
    m_ButtonJoin->setColor(0.5f, 0.5f, 0.5f, 1);
    m_ButtonJoin->setTextColor(1, 1, 0, 1);
    
    m_ButtonHost->setUserPointer(this);
    m_ButtonJoin->setUserPointer(this);

    m_ButtonHost->setOnClickFunctor(ButtonHost_OnClick());
    m_ButtonJoin->setOnClickFunctor(ButtonJoin_OnClick());


    m_Back = new Button(*m_Font, glm::vec2(100, 50), 150, 50);
    m_Back->setText("Back");
    m_Back->setColor(0.5f, 0.5f, 0.5f, 1);
    m_Back->setTextColor(1, 1, 0, 1);
    m_Next = new Button(*m_Font, glm::vec2(windowDimensions.x - 100, 50), 150, 50);
    m_Next->setText("Next");
    m_Next->setColor(0.5f, 0.5f, 0.5f, 1);
    m_Next->setTextColor(1, 1, 0, 1);
    m_Back->setUserPointer(this);
    m_Next->setUserPointer(this);

    m_Back->setOnClickFunctor(ButtonBack_OnClick());
    m_Next->setOnClickFunctor(ButtonNext_OnClick());


    m_ServerIp = new TextBox("Server IP",*m_Font, 40, windowDimensions.x / 2, 115);
    m_ServerIp->setColor(0.5f, 0.5f, 0.5f, 1);
    m_ServerIp->setTextColor(1, 1, 0, 1);
    m_UserName = new TextBox("Your Name",*m_Font, 20, windowDimensions.x / 2, 275);
    m_UserName->setColor(0.5f, 0.5f, 0.5f, 1);
    m_UserName->setTextColor(1, 1, 0, 1);
    m_ServerPort = new TextBox("Server Port",*m_Font, 7, windowDimensions.x / 2, 195);
    m_ServerPort->setColor(0.5f, 0.5f, 0.5f, 1);
    m_ServerPort->setTextColor(1, 1, 0, 1);
    m_ServerPort->setText("55000");

    m_InfoText = new Text(Resources::getWindowSize().x / 2, 65, *m_Font);
    m_InfoText->setTextAlignment(TextAlignment::Center);


    m_ServerHostMapSelector = new ServerHostingMapSelectorWindow(*m_Font, Resources::getWindowSize().x / 2 - 300, 630);



    m_ServerLobbyChatWindow = new ServerLobbyChatWindow(*m_Font, 50, 140 + 300);
    m_ServerLobbyChatWindow->setColor(1, 1, 0, 1);

    m_ServerLobbyConnectedPlayersWindow = new ServerLobbyConnectedPlayersWindow(*m_Font, 50 + m_ServerLobbyChatWindow->getWindowFrame().width(), 140 + 300);
    m_ServerLobbyConnectedPlayersWindow->setColor(1, 1, 0, 1);

    m_ServerLobbyShipSelectorWindow = new ServerLobbyShipSelectorWindow(core,scene, camera, *m_Font, 50, windowDimensions.y - 50);
    m_ServerLobbyShipSelectorWindow->setColor(1, 1, 0, 1);
    m_ServerLobbyShipSelectorWindow->setUserPointer(core.m_ChosenShip);

    m_MainMenuMusic = Engine::Sound::playMusic(ResourceManifest::MenuMusic);
    m_MainMenuMusic->stop();
}
HUD::~HUD() {
    SAFE_DELETE(m_ButtonHost);
    SAFE_DELETE(m_ButtonJoin);
    SAFE_DELETE(m_Back);
    SAFE_DELETE(m_Next);
    SAFE_DELETE(m_ServerIp);
    SAFE_DELETE(m_UserName);
    SAFE_DELETE(m_ServerPort);

    SAFE_DELETE(m_InfoText);

    SAFE_DELETE(m_ServerHostMapSelector);

    SAFE_DELETE(m_ServerLobbyConnectedPlayersWindow);
    SAFE_DELETE(m_ServerLobbyChatWindow);
    SAFE_DELETE(m_ServerLobbyShipSelectorWindow);
}

void HUD::enter_the_game() {
    /*
    TODO: once a client wants to enter the game, he will ping the server that he entered the map. the server will then respond with the other connected players currently
    ingame and give the client info about their positions. the server will then very rapidly and frequently update the client with all the other connected player positions
    */
    if (!m_ServerLobbyShipSelectorWindow->m_ChosenShipName.empty()) {
        PacketMessage p;
        p.PacketType = PacketType::Client_To_Server_Request_Map_Entry;
        p.name = m_Core.m_Client->m_username;
        p.data = m_ServerLobbyShipSelectorWindow->m_ChosenShipName + "," + m_Core.m_Client->m_mapname;
        m_Core.m_Client->send(p);
    }else{
        setErrorText("You must choose your ship", 5);
    }
}

void HUD::go_to_main_menu() {
    m_GameState = GameState::Main_Menu;
    m_ServerLobbyShipSelectorWindow->setShipViewportActive(false);
    m_Next->setText("Next");
    setErrorText("", 0);
    auto& window = Resources::getWindow();
    auto _status = m_MainMenuMusic->status();
    if (_status == SoundStatus::Fresh || _status == SoundStatus::Stopped || _status == SoundStatus::Paused) {
        //m_MainMenuMusic->play();
    }
    window.keepMouseInWindow(false);
    window.setMouseCursorVisible(true);
}


void HUD::setGoodText(const string& text, const float errorTime) {
    m_InfoText->setText(text);
    m_ErrorTimer = errorTime;
    m_InfoText->setColor(0, 1, 0, 1);
}
void HUD::setNormalText(const string& text, const float errorTime) {
    m_InfoText->setText(text);
    m_ErrorTimer = errorTime;
    m_InfoText->setColor(1, 1, 0, 1);
}
void HUD::setErrorText(const string& text, const float errorTime) {
    m_InfoText->setText(text);
    m_ErrorTimer = errorTime;
    m_InfoText->setColor(1,0,0,1);
}

void HUD::onResize(const uint& width, const uint& height) {
    m_ButtonHost->setPosition(width / 2, 275);
    m_ButtonJoin->setPosition(width / 2, 155);

    m_Back->setPosition(100, 50);
    m_Next->setPosition(width - 100, 50);

    m_ServerIp->setPosition(width / 2, 115);
    m_ServerPort->setPosition(width / 2, 275);
    m_UserName->setPosition(width / 2, 195);

    m_InfoText->setPosition(width / 2 , 65);

    m_ServerHostMapSelector->setPosition(Resources::getWindowSize().x / 2 - 300, 630);

    m_ServerLobbyChatWindow->setPosition(50, 140 + 300);
    m_ServerLobbyConnectedPlayersWindow->setPosition(50 + m_ServerLobbyChatWindow->getWindowFrame().width(), 140 + 300);

    const auto& winSize = Resources::getWindowSize();
    m_ServerLobbyShipSelectorWindow->setPosition(50, winSize.y - 50);
}

uint _countShips = 0;
uint _countPlanets = 0;
void HUD::update_game(const double& dt) {
    if (Engine::isKeyDownOnce(KeyboardKey::LeftAlt, KeyboardKey::X) || Engine::isKeyDownOnce(KeyboardKey::RightAlt, KeyboardKey::X)) {
        m_Active = !m_Active;
    }

    Map* scene = static_cast<Map*>(Resources::getCurrentScene());
    auto& player = *scene->getPlayer();
    auto& playerName = player.entity().getComponent<ComponentName>()->name();

    if (Engine::isKeyDownOnce(KeyboardKey::Comma)) {
        const auto& ships = scene->getShips();
        vector<Ship*> shipsVect;
        shipsVect.reserve(ships.size());

        for (auto& p : ships) {
            auto& name = p.second->entity().getComponent<ComponentName>()->name();
            if (name != playerName)
                shipsVect.push_back(p.second);
        }


        if (shipsVect.size() > 0) {
            player.setTarget(shipsVect[_countShips]->entity().getComponent<ComponentName>()->name());
            ++_countShips;
            if (_countShips > shipsVect.size() - 1) {
                _countShips = 0;
            }
        }
    }else if (Engine::isKeyDownOnce(KeyboardKey::Period)) {
        const auto& planets = scene->getPlanets();
        vector<Planet*> planetsVect;
        planetsVect.reserve(planets.size());

        for (auto& p : planets) {
            planetsVect.push_back(p.second);
        }


        player.setTarget(planetsVect[_countPlanets]->entity().getComponent<ComponentName>()->name());
        ++_countPlanets;
        if (_countPlanets > planetsVect.size() - 1) {
            _countPlanets = 0;
        }
    }
}
void HUD::update_main_menu(const double& dt) {
    m_Font->renderText(epriv::Core::m_Engine->m_DebugManager.reportDebug(), glm::vec2(50), glm::vec4(1.0), 0);

    m_ButtonHost->update(dt);
    m_ButtonJoin->update(dt);
}
void HUD::update_host_server_lobby_and_ship(const double& dt) {
    m_ServerLobbyChatWindow->update(dt);
    m_ServerLobbyConnectedPlayersWindow->update(dt);
    m_ServerLobbyShipSelectorWindow->update(dt);

    m_Back->update(dt);
    m_Next->update(dt);
}
void HUD::update_host_server_port_and_name_and_map(const double& dt) {
    m_Back->update(dt);
    m_Next->update(dt);

    m_ServerPort->update(dt);
    m_UserName->update(dt);
    m_ServerHostMapSelector->update(dt);
}
void HUD::update_join_server_port_and_name_and_ip(const double& dt) {
    m_Back->update(dt);
    m_Next->update(dt);

    m_ServerIp->update(dt);
    m_ServerPort->update(dt);
    m_UserName->update(dt);
}
void HUD::update_join_server_server_lobby(const double& dt) {
    m_ServerLobbyChatWindow->update(dt);
    m_ServerLobbyConnectedPlayersWindow->update(dt);
    m_ServerLobbyShipSelectorWindow->update(dt);

    m_Back->update(dt);
    m_Next->update(dt);
}

void HUD::render_game() {

    Map* scene = static_cast<Map*>(Resources::getCurrentScene());
    Ship* player = scene->getPlayer();
    glm::vec2 winSize = glm::vec2(Resources::getWindowSize().x, Resources::getWindowSize().y);

#pragma region renderCrossHairAndOtherInfo

    auto& target = player->getTarget();
    if (!target.null()) {
        auto& body = *target.getComponent<ComponentBody>();
        glm::vec3 pos = body.getScreenCoordinates(true);
        if (pos.z == 1) { //infront 
            auto boxPos = body.getScreenBoxCoordinates(8.0f);
            Material& crosshair = *(Material*)ResourceManifest::CrosshairMaterial.get();

            auto& crosshairTexture = *crosshair.getComponent(MaterialComponentType::Diffuse)->texture();
            const glm::vec4& color = glm::vec4(m_Color.x, m_Color.y, m_Color.z, 1.0f);

            crosshairTexture.render(boxPos.topLeft, color, 270.0f);
            crosshairTexture.render(boxPos.topRight, color, 180.0f);
            crosshairTexture.render(boxPos.bottomLeft, color, 0.0f);
            crosshairTexture.render(boxPos.bottomRight, color, 90.0f);

            auto& targetBody = *target.getComponent<ComponentBody>();
            auto& targetName = target.getComponent<ComponentName>()->name();

            unsigned long long distanceInKm = (targetBody.getDistanceLL(player->entity()) / 10);
            string stringRepresentation = "";
            if (distanceInKm > 0) {
                stringRepresentation = convertNumToNumWithCommas(unsigned long long(distanceInKm)) + " Km";
            }else {
                float distanceInm = (targetBody.getDistance(player->entity())) * 100.0f;
                stringRepresentation = to_string(uint(distanceInm)) + " m";
            }
            m_Font->renderText(targetName + "\n" + stringRepresentation, glm::vec2(pos.x + 40, pos.y - 15), glm::vec4(m_Color.x, m_Color.y, m_Color.z, 1), 0, glm::vec2(0.7f, 0.7f), 0.1f);
        }
        else { //behind
            float angle = 0;
            Material& crosshairArrow = *(Material*)ResourceManifest::CrosshairArrowMaterial.get();
            auto& crosshairArrowTexture = *crosshairArrow.getComponent(MaterialComponentType::Diffuse)->texture();
            uint textureSizeOffset = (crosshairArrowTexture.width() / 2) + 4;
            if (pos.y > 2 && pos.y < winSize.y - 2) { //if y is within window bounds
                if (pos.x < 2) {
                    angle = -45;
                    pos.x += textureSizeOffset;
                }else {
                    angle = 135;
                    pos.x -= textureSizeOffset;
                }
            }else if (pos.y <= 1) { //if y is below the window bounds
                pos.y += textureSizeOffset;
                if (pos.x <= 1) { //bottom left corner
                    angle = 0;
                    pos.x += textureSizeOffset - 4;
                    pos.y -= 4;
                }else if (pos.x > winSize.x - 2) { //bottom right corner
                    angle = 90;
                    pos.x -= textureSizeOffset - 4;
                    pos.y -= 4;
                }else { //bottom normal
                    angle = 45;
                }
            }else { //if y is above the window bounds
                pos.y -= textureSizeOffset;
                if (pos.x < 2) { //top left corner
                    angle = -90;
                    pos.x += textureSizeOffset - 4;
                    pos.y += 4;
                }else if (pos.x > winSize.x - 2) { //top right corner
                    angle = 180;
                    pos.x -= textureSizeOffset - 4;
                    pos.y += 4;
                }else { //top normal
                    angle = -135;
                }
            }
            crosshairArrowTexture.render(glm::vec2(pos.x, pos.y), glm::vec4(m_Color.x, m_Color.y, m_Color.z, 1.0f), angle);
        }
    }

#pragma endregion

    if (!m_Active) return;

#pragma region DrawDebugStuff
    m_Font->renderText(Engine::Data::reportTime() +
        epriv::Core::m_Engine->m_DebugManager.reportDebug(),
        glm::vec2(10, Resources::getWindowSize().y - 10), glm::vec4(m_Color.x, m_Color.y, m_Color.z, 1), 0, glm::vec2(0.8f, 0.8f), 0.1f);

#pragma endregion

}
void HUD::render_main_menu() {
    m_ButtonHost->render();
    m_ButtonJoin->render();
}
void HUD::render_host_server_lobby_and_ship() {
    m_ServerLobbyChatWindow->render();
    m_ServerLobbyConnectedPlayersWindow->render();
    m_ServerLobbyShipSelectorWindow->render();

    m_Back->render();
    m_Next->render();
}
void HUD::render_host_server_port_and_name_and_map() {
    m_Back->render();
    m_Next->render();

    m_ServerPort->render();
    m_UserName->render();
    m_ServerHostMapSelector->render();
}
void HUD::render_join_server_port_and_name_and_ip() {
    m_Back->render();
    m_Next->render();

    m_ServerIp->render();
    m_ServerPort->render();
    m_UserName->render();
}
void HUD::render_join_server_server_lobby() {
    m_ServerLobbyChatWindow->render();
    m_ServerLobbyConnectedPlayersWindow->render();
    m_ServerLobbyShipSelectorWindow->render();

    m_Back->render();
    m_Next->render();
}
void HUD::update(const double& dt) {
    switch (m_GameState) {
        case GameState::Main_Menu: {
            update_main_menu(dt); break;
        }case GameState::Host_Server_Lobby_And_Ship: {
            update_host_server_lobby_and_ship(dt); break;
        }case GameState::Host_Server_Port_And_Name_And_Map: {
            update_host_server_port_and_name_and_map(dt); break;
        }case GameState::Join_Server_Port_And_Name_And_IP: {
            update_join_server_port_and_name_and_ip(dt); break;
        }case GameState::Join_Server_Server_Lobby: {
            update_join_server_server_lobby(dt); break;
        }case GameState::Game: {
            update_game(dt); break;
        }default: {
            break;
        }
    }
    if (m_ErrorTimer > 0) {
        m_ErrorTimer -= static_cast<float>(dt);
        if (m_ErrorTimer < 0) {
            setErrorText("", 0);
            m_ErrorTimer = 0.0f;
        }
    }
}
void HUD::render() {
    switch (m_GameState) {
        case GameState::Main_Menu: {
            render_main_menu(); break;
        }case GameState::Host_Server_Lobby_And_Ship: {
            render_host_server_lobby_and_ship(); break;
        }case GameState::Host_Server_Port_And_Name_And_Map: {
            render_host_server_port_and_name_and_map(); break;
        }case GameState::Join_Server_Port_And_Name_And_IP: {
            render_join_server_port_and_name_and_ip(); break;
        }case GameState::Join_Server_Server_Lobby: {
            render_join_server_server_lobby(); break;
        }case GameState::Game: {
            render_game(); break;
        }default: {
            break;
        }
    }
    m_InfoText->render();
}
