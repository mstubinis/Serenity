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
    const std::string& iconPath = ResourceManifest::BasePath + "data/Textures/icon.png";
    Resources::getWindow().setIcon(iconPath);
}
Core::~Core() {

}
void Core::startServer(const unsigned short& port) {
    shutdownServer();
    m_Server = new Server(port);
    m_Server->startup();
}
void Core::shutdownServer() {
    if (m_Server) {
        m_Server->shutdown();
        SAFE_DELETE(m_Server);
    }
}

void Core::startClient(const unsigned short& port, const string& ip) {
    m_Client = new Client(port, ip);
    m_Client->connect();
}
void Core::shutdownClient() {
    if (m_Client) {
        m_Client->disconnect();
        SAFE_DELETE(m_Client);
    }
}

void Core::enterMap(const string& mapFile) {
    SolarSystem* map = new SolarSystem(mapFile, ResourceManifest::BasePath + "data/Systems/" + mapFile + ".txt");
    Resources::setCurrentScene(map);

    auto& window = Resources::getWindow();
    window.keepMouseInWindow(true);
    window.setMouseCursorVisible(false);

    Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::SMAA);
    Renderer::smaa::setQuality(SMAAQualityLevel::Ultra);
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
    if (Engine::isKeyDownOnce(KeyboardKey::F6)) {
        Resources::getWindow().setFullScreen(!Resources::getWindow().isFullscreen());
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F7)) {
        Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::None);
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F8)) {
        Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::SMAA);
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F9)) {
        Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::FXAA);
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F10)) {
        Renderer::ssao::enable(!Renderer::ssao::enabled());
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F11)) {
        Renderer::hdr::enable(!Renderer::hdr::enabled());
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F12)) {
        Renderer::godRays::enable(!Renderer::godRays::enabled());
    }
    if (Engine::isKeyDown(KeyboardKey::U)) {
        Renderer::Settings::Lighting::setGIContributionGlobal(Renderer::Settings::Lighting::getGIContributionGlobal() - 0.01f);
    }else if (Engine::isKeyDown(KeyboardKey::I)) {
        Renderer::Settings::Lighting::setGIContributionGlobal(Renderer::Settings::Lighting::getGIContributionGlobal() + 0.01f);
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