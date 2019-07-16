#include "Core.h"

#include "HUD.h"
#include "Server.h"
#include "Client.h"
#include "SolarSystem.h"
#include "Packet.h"
#include "ResourceManifest.h"

#include <core/engine/Engine.h>

using namespace std;
using namespace Engine;


Core::Core() {
    m_HUD               = nullptr;
    m_Server            = nullptr;
    m_Client            = nullptr;
    m_Initalized        = false;
    m_GameState         = GameState::Main_Menu;

    ResourceManifest::init();
    //const std::string& iconPath = ResourceManifest::BasePath + "data/Textures/icon.png";
    //Resources::getWindow().setIcon(iconPath);
}
Core::~Core() {
    shutdownClient();
    shutdownServer();
    SAFE_DELETE(m_HUD);
}
void Core::startServer(const unsigned short& port) {
    if (!m_Server) {
        m_Server = new Server(*this, port);
        m_Server->startup();
    }
}
void Core::shutdownServer() {
    if (m_Server) {
        m_Server->shutdown();
        SAFE_DELETE(m_Server);
    }
}

bool Core::startClient(const unsigned short& port, const std::string& name, const string& ip) {
    const auto& lamb = [&]() {
        Packet p;
        p.PacketType = PacketType::Client_To_Server_Request_Connection;
        p.data = name;
        const auto& status = m_Client->send(p);
        if (status == sf::Socket::Status::Done) {
            std::cout << "Client: requesting connection to the server..." << std::endl;
            return true;
        }else{
            m_HUD->setErrorText("Connection timed out");
        }
        return false;
    };
    if (!m_Client) {
        m_Client = new Client(*this, port, ip);
    }

    const auto fetchedIP   = m_Client->m_TcpSocket->ip();
    const auto fetchedPort = m_Client->m_TcpSocket->localPort();

    if (fetchedIP != ip && fetchedPort != port) {
        m_Client->changeConnectionDestination(port, ip);
        bool am_i_connected = m_Client->connect(6);
        return lamb();
    }
    if (!m_Client->connected()) {
        m_Client->changeConnectionDestination(port, ip);
        bool am_i_connected = m_Client->connect(6);
        return lamb();
    }
    if (!m_Client->m_Validated) {
        return lamb();
    }
    return false;
}
void Core::shutdownClient() {
    if (m_Client) {
        if (m_Client->connected()) {
            Packet p;
            p.PacketType = PacketType::Client_To_Server_Request_Disconnection;
            const auto status = m_Client->send(p);
            if (status == sf::Socket::Status::Done) {
                std::cout << "Told the server i am quitting" << std::endl;
            }else {
                std::cout << "Error: could not tell the server i am quitting" << std::endl;
            }
        }
        m_Client->disconnect();
        SAFE_DELETE(m_Client);
    }
}
void Core::enterMap(const string& mapFile) {
    SolarSystem* map = new SolarSystem(mapFile, ResourceManifest::BasePath + "data/Systems/" + mapFile + ".txt");
    auto ships = map->allowedShips();
    Resources::setCurrentScene(map);

    auto& window = Resources::getWindow();
    window.keepMouseInWindow(true);
    window.setMouseCursorVisible(false);
}
void Core::onResize(const uint& width, const uint& height) {
    m_HUD->onResize(width, height);
}
void Core::init() {
    if (m_Initalized) return;

    auto& window = Resources::getWindow();
    window.setKeyRepeatEnabled(false);
    window.setFramerateLimit(60);

    Scene* s = new Scene("Menu");
    Resources::setCurrentScene(s);
    
    m_HUD    = new HUD(m_GameState, *this);
    m_Initalized = true;
}
void Core::update(const double& dt) {
    if (Engine::isKeyDown(KeyboardKey::Escape)) {
        Engine::stop();
    }


    /*
    if (Engine::isKeyDownOnce(KeyboardKey::Space)) {
        //Engine::pause(!Engine::paused());
        m_GameState = GameState::Game;
        if (!m_Client->connected()) {
            m_Client->connect();
        }else{
            SolarSystem& scene = *static_cast<SolarSystem*>(Resources::getCurrentScene());
            PacketPhysicsUpdate p(*scene.getPlayer());
            p.PacketType = PacketType::Client_To_Server_Ship_Physics_Update;
            m_Client->send(p);
        }
    }
    */



    m_HUD->update(dt);
    epriv::ClientInternalPublicInterface::update(m_Client);
    epriv::ServerInternalPublicInterface::update(m_Server);
}
void Core::render() {
    m_HUD->render();
}

const GameState::State& Core::gameState() const {
    return m_GameState;
}