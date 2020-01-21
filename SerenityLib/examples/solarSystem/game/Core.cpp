#include "Core.h"

#include "Menu.h"
#include "networking/server/Server.h"
#include "networking/client/Client.h"
#include "Ship.h"
#include "map/Map.h"
#include "networking/packets/PacketMessage.h"
#include "Planet.h"
#include "GameCamera.h"
#include "GameSkybox.h"
#include "ResourceManifest.h"
#include "teams/Team.h"
#include "modes/GameplayMode.h"
#include "config/ConfigFile.h"

#include <core/engine/system/Engine.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/lights/SunLight.h>
#include <core/engine/system/window/Window.h>
#include <ecs/Components.h>


using namespace std;
using namespace Engine;

Core Core::_DUMMY_VALUE = Core();

Core::Core() {
    m_Menu              = nullptr;
    m_Server            = nullptr;
    m_Client            = nullptr;
    m_Initalized        = false;
    m_GameTime          = 0.0;
    m_GameState         = GameState::Main_Menu;
}
Core::~Core() {
    SAFE_DELETE(m_Client);
    SAFE_DELETE(m_Server);
    SAFE_DELETE(m_Menu);
}
Server* Core::getServer() {
    return m_Server;
}
Client* Core::getClient() {
    return m_Client;
}

void Core::startServer(const unsigned short& port) {
    if (!m_Server) {
        m_Server = NEW Server(*this, port);
        m_Server->startup();
    }
}
void Core::shutdownServer() {
    if (m_Server) {
        m_Server->shutdownMap();
        m_Server->shutdown();
        SAFE_DELETE(m_Server);
    }
}
void Core::startClient(Team* team, const unsigned short& port, const string& name, const string& ip) {
    if (!m_Client) {
        m_Client = NEW Client(team, *this, port, ip, 0);
    }
    auto& client = *m_Client;
    client.m_Username = name;
    client.changeConnectionDestination(port, ip);
    m_Client->connect(10);
}
void Core::shutdownClient(const bool& serverShutdownFirst) {
    if (m_Client) {
        auto& client = *m_Client;
        if (!serverShutdownFirst) {
            PacketMessage p;
            p.PacketType = PacketType::Client_To_Server_Request_Disconnection;
            const auto status = client.send(p);
        }
        client.m_TcpSocket->setBlocking(false);
        client.disconnect();
        client.cleanup();
    }
}
void Core::requestValidation(const string& name) {
    PacketMessage p;
    p.PacketType = PacketType::Client_To_Server_Request_Connection;
    p.data = name;
    const auto& status = m_Client->send(p);
    if (status == sf::Socket::Status::Done) {
        std::cout << "Client: requesting validation connection to the server..." << endl;
    }else{
        m_Menu->setErrorText("Connection timed out");
    }
}
void Core::enterMap(Team& playerTeam, const string& mapFile, const string& playerShipClass, const string& playername, const float& x, const float& y, const float& z) {
    if (Resources::getCurrentScene()->name() == mapFile) {
        return;
    }
    auto& window = Resources::getWindow();
    Resources::setCurrentScene(mapFile);

    auto& map = *m_Client->m_MapSpecificData.m_Map;

    Ship* playerShip = map.createShip(AIType::Player_You, playerTeam, *m_Client, playerShipClass, playername, glm::vec3(x, y, z));
    map.setPlayer(playerShip);

    window.keepMouseInWindow(true);
    window.setMouseCursorVisible(false);
}
void Core::onResize(const uint& width, const uint& height) {
    if(m_Menu)
        m_Menu->onResize(width, height);
}
void Core::init() {
    if (m_Initalized)
        return;

    auto& window = Resources::getWindow();
    window.setKeyRepeatEnabled(false);
    window.setFramerateLimit(60);
 
    auto& winSize = window.getSize();
    Scene* menuScene = NEW Scene("Menu");
    Resources::setCurrentScene(menuScene);
    Camera* main_camera = NEW Camera(60, winSize.x / static_cast<float>(winSize.y), 0.1f, 15000.0f, menuScene);
    menuScene->setActiveCamera(*main_camera);
    menuScene->getMainViewport().removeRenderFlag(ViewportRenderingFlag::Skybox);
    
    SunLight* ship_3d_viewer_light = NEW SunLight(glm::vec3(0.0f), LightType::Sun, menuScene);
    ship_3d_viewer_light->setColor(1.55f, 1.55f, 1.3f);
    ship_3d_viewer_light->setPosition(0.0f, 3000.0f, -10000.0f);
    
    GameCamera* ship_camera = NEW GameCamera(0.1f, 50.0f, menuScene);
    m_Menu = NEW Menu(*menuScene, *ship_camera, m_GameState, *this);
    m_Menu->go_to_main_menu();
    
    ModelInstance::setDefaultViewportFlag(ViewportFlag::_1); //for now on, all objects render in the first viewport only unless otherwise specified
    
    m_Initalized = true;
}
void Core::update(const double& dt) {
    m_GameTime += dt;
    if (Engine::isKeyDownOnce(KeyboardKey::F6)) {
        auto& window = Resources::getWindow();
        const auto res = window.setFullscreen(!window.isFullscreenNonWindowed());
        ConfigFile config;
        if (window.isFullscreenNonWindowed()) {
            config.updateWindowMode("fullscreen");
        }else {
            config.updateWindowMode("windowed");
        }
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F7)) {
        auto& window = Resources::getWindow();
        const auto res = window.setFullscreenWindowed(!window.isFullscreenWindowed());
        ConfigFile config;
        if (window.isFullscreenWindowed()) {
            config.updateWindowMode("windowed_fullscreen");
        }else {
            config.updateWindowMode("windowed");
        }
    }

    if(m_Client) m_Client->update(m_Client, dt);
    if(m_Server) m_Server->update(m_Server, dt);
    if(m_Menu)   m_Menu->update(dt);
}
void Core::render() {
    if(m_Menu)   m_Menu->render();
}

const GameState::State& Core::gameState() const {
    return m_GameState;
}