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

#include <core/engine/Engine.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/lights/SunLight.h>
#include <ecs/Components.h>

using namespace std;
using namespace Engine;

Core::Core() {
    m_Menu              = nullptr;
    m_Server            = nullptr;
    m_Client            = nullptr;
    m_Initalized        = false;
    m_GameTime          = 0.0;
    m_GameState         = GameState::Main_Menu;

    ResourceManifest::init();
    //const std::string& iconPath = ResourceManifest::BasePath + "data/Textures/icon.png";
    //Resources::getWindow().setIcon(iconPath);
}
Core::~Core() {
    ResourceManifest::destruct();
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

void Core::startServer(const unsigned short& port, const string& mapname) {
    if (!m_Server) {
        m_Server = new Server(*this, port);
        m_Server->startup(mapname);
    }
}
void Core::shutdownServer() {
    if (m_Server) {
        m_Server->shutdownMap();
        m_Server->shutdown();
        SAFE_DELETE(m_Server);
    }
}
void Core::startClient(GameplayMode* mode, Team* team, const unsigned short& port, const string& name, const string& ip) {
    if (!m_Client) {
        m_Client = new Client(team, *this, port, ip, 0);
    }
    auto& client = *m_Client;
    client.m_MapSpecificData.m_GameplayMode = mode;
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
        Resources::setCurrentScene("Menu");
        m_Menu->go_to_main_menu();
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
    auto& window = Resources::getWindow();
    Resources::setCurrentScene(mapFile);

    auto& map = *m_Client->m_MapSpecificData.m_Map;

    Ship* playerShip = map.createShip(AIType::Player_You, playerTeam, *m_Client, playerShipClass, playername, glm::vec3(x, y, z));
    map.setPlayer(playerShip);
    GameCamera* playerCamera = (GameCamera*)map.getActiveCamera();
    playerCamera->setPlayer(playerShip);
    playerCamera->setTarget(playerShip);
    playerCamera->setState(CameraState::Cockpit);

    window.keepMouseInWindow(true);
    window.setMouseCursorVisible(false);
}
void Core::onResize(const uint& width, const uint& height) {
    m_Menu->onResize(width, height);
}
void Core::init() {
    if (m_Initalized)
        return;

    auto& window = Resources::getWindow();
    window.setKeyRepeatEnabled(false);
    window.setFramerateLimit(60);

    Scene* menuScene = new Scene("Menu");
    Resources::setCurrentScene(menuScene);
    Camera* main_camera = new Camera(60,Resources::getWindowSize().x / static_cast<float>(Resources::getWindowSize().y), 0.1f, 15000.0f, menuScene);
    GameCamera* ship_camera = new GameCamera(0.1f, 50.0f, menuScene);
    menuScene->setActiveCamera(*main_camera);
    menuScene->getMainViewport().removeRenderFlag(ViewportRenderingFlag::Skybox);

    SunLight* light = new SunLight(glm::vec3(0.0f), LightType::Sun, menuScene);
    light->setColor(1.55f, 1.55f, 1.3f);
    light->setPosition(0.0f, 3000.0f, -10000.0f);

    m_Menu = new Menu(*menuScene, *ship_camera, m_GameState, *this);
    m_Menu->go_to_main_menu();

    ModelInstance::setDefaultViewportFlag(ViewportFlag::_1); //for now on, all objects render in the first viewport only unless otherwise specified

    m_Initalized = true;
}
void Core::update(const double& dt) {
    m_GameTime += dt;
    if (Engine::isKeyDown(KeyboardKey::Escape)) {
        Engine::stop();
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F6)) {
        Resources::getWindow().setFullScreen(!Resources::getWindow().isFullscreen());
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F7)) {
        Resources::getWindow().setFullScreen(!Resources::getWindow().isFullscreen(), true);
    }

    if(m_Client) m_Client->update(m_Client, dt);
    if(m_Server) m_Server->update(m_Server, dt);
    m_Menu->update(dt);
}
void Core::render() {
    m_Menu->render();
}

const GameState::State& Core::gameState() const {
    return m_GameState;
}