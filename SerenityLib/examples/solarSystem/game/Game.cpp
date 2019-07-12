#include "SolarSystem.h"
#include "HUD.h"
#include "Ship.h"
#include "Planet.h"
#include "GameCamera.h"
#include "ResourceManifest.h"

#include "Server.h"
#include "Client.h"
#include "Packet.h"
#include <core/engine/Engine_Math.h>

#include <core/Material.h>

#include <core/engine/Engine.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/networking/Networking.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/lights/Lights.h>

#include <core/engine/lua/LuaScript.h>


#include <unordered_map>
#include <iostream>
#include <glm/vec2.hpp>

using namespace Engine;

HUD* m_HUD;
Server* m_Server;
Client* m_Client;

void Game::cleanup(){
    SAFE_DELETE(m_HUD);
    SAFE_DELETE(m_Client);
    SAFE_DELETE(m_Server);
}

void Game::initResources(){
    m_HUD    = nullptr;
    m_Server = nullptr;
    m_Client = nullptr;

    ResourceManifest::init();
    const std::string& iconPath = ResourceManifest::BasePath + "data/Textures/icon.png";
    Resources::getWindow().setIcon(iconPath);
}
void Game::initLogic(){
    auto& window = Resources::getWindow();
    window.keepMouseInWindow(true);
    window.setMouseCursorVisible(false);
    window.setKeyRepeatEnabled(false);
	window.setFramerateLimit(60);
    SolarSystem* sol = new SolarSystem("Sol", ResourceManifest::BasePath + "data/Systems/Sol.txt");

    Resources::setCurrentScene("Sol");

    m_HUD = new HUD();


    m_Server = new Server(55000);
    m_Server->startup();

	/*
    LuaScript script("../data/Scripts/test.lua");
    float posX = script.get<float>("player.position.x");
    float posY = script.get<float>("player.position.y");
    int hp = script.get<int>("player.HP");
    
    std::cout << "Position X = " << posX << ", Y = " << posY << std::endl;
    std::cout << "HP:" << hp << std::endl;


    // getting arrays
    std::vector<int> v = script.getIntVector("array");
    std::cout << "Contents of array:";
    for (auto& val : v) {
        std::cout << val << std::endl;
    }
    // get table keys
    std::vector<std::string> tblValues = script.getTableKeys("animations");
    std::cout << "Table Values:" << std::endl;
    for (auto& val : tblValues) {
        std::cout << val << std::endl;
    }
	*/
}
void Game::update(const double& dt){
    if (Engine::isKeyDownOnce(KeyboardKey::Space)) {
        //Engine::pause(!Engine::paused());


        if (!m_Client) {
            m_Client = new Client(55000, "127.0.0.1");
        }
        if (!m_Client->connected()) {
            m_Client->connect();
        }else{
            Packet packet;

            packet.PacketType = PacketType::ClientSendInfo;

            Engine::Math::Float16From32(&packet.x, 10.0f);
            Engine::Math::Float16From32(&packet.y, 20.0f);
            Engine::Math::Float16From32(&packet.z, 30.0f);

            Engine::Math::Float16From32(&packet.x1, 40.0f);
            Engine::Math::Float16From32(&packet.y1, 50.0f);
            Engine::Math::Float16From32(&packet.z1, 60.0f);

            Engine::Math::Float16From32(&packet.x2, 70.0f);
            Engine::Math::Float16From32(&packet.y2, 80.0f);
            Engine::Math::Float16From32(&packet.z2, 90.0f);

            Engine::Math::Float16From32(&packet.x3, 100.0f);
            Engine::Math::Float16From32(&packet.y3, 110.0f);
            Engine::Math::Float16From32(&packet.z3, 120.0f);

            m_Client->send(packet);
        }
    }
    if (Engine::isKeyDown(KeyboardKey::Escape)) {
        Engine::stop();
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

	Scene& scene = *Resources::getCurrentScene();
	Material& defMat = *((Material*)(ResourceManifest::DefiantSharkMaterial.get()));
	if (Engine::isKeyDown(KeyboardKey::V)) {
		defMat.setSmoothness(defMat.smoothness() - 0.01f);
	}else if (Engine::isKeyDown(KeyboardKey::B)) {
		defMat.setSmoothness(defMat.smoothness() + 0.01f);
	}
	if (Engine::isKeyDown(KeyboardKey::N)) {
		defMat.setMetalness(defMat.metalness() - 0.01f);
	}else if (Engine::isKeyDown(KeyboardKey::M)) {
		defMat.setMetalness(defMat.metalness() + 0.01f);
	}
	if (Engine::isKeyDown(KeyboardKey::O)) {
		Renderer::godRays::enable(!Renderer::godRays::enabled());
	}else if (Engine::isKeyDown(KeyboardKey::P)) {
		Renderer::godRays::enable(!Renderer::godRays::enabled());
	}


    if (Engine::isKeyDown(KeyboardKey::U)) {
        Renderer::Settings::Lighting::setGIContributionGlobal(Renderer::Settings::Lighting::getGIContributionGlobal() - 0.01f);
    }else if (Engine::isKeyDown(KeyboardKey::I)) {
        Renderer::Settings::Lighting::setGIContributionGlobal(Renderer::Settings::Lighting::getGIContributionGlobal() + 0.01f);
    }

    m_HUD->update(dt);
    epriv::ClientInternalPublicInterface::update(m_Client);
    epriv::ServerInternalPublicInterface::update(m_Server);
}
void Game::render(){
    m_HUD->render();
}

#pragma region EventHandlers
void Game::onResize(const uint& width, const uint& height){
}
void Game::onClose(){
}
void Game::onLostFocus(){
    Engine::getWindow().keepMouseInWindow(false);
    //Engine::getWindow().setMouseCursorVisible(true);
}
void Game::onGainedFocus(){
    Engine::getWindow().keepMouseInWindow(true);
    //Engine::getWindow().setMouseCursorVisible(false);
    auto& window = Resources::getWindow();
    const auto& size = window.getSize();
    const glm::vec2 halfRes(size.x / 2, size.y / 2);
    sf::Mouse::setPosition(sf::Vector2i(static_cast<int>(halfRes.x), static_cast<int>(halfRes.y)), window.getSFMLHandle());

    Engine::setMousePosition(halfRes, true);
}
void Game::onTextEntered(const uint& unicode){
}
void Game::onKeyPressed(const uint& key){
}
void Game::onKeyReleased(const uint& key){
}
void Game::onMouseWheelMoved(const int& delta){
}
void Game::onMouseButtonPressed(const uint& button){
}
void Game::onMouseButtonReleased(const uint& button){
}
void Game::onMouseMoved(const float& mouseX, const float& mouseY){
}
void Game::onMouseEntered(){
    Engine::getWindow().requestFocus();
    Engine::getWindow().keepMouseInWindow(true);
    //Engine::getWindow().setMouseCursorVisible(true);
}
void Game::onMouseLeft(){
    Engine::getWindow().keepMouseInWindow(false);
    //Engine::getWindow().setMouseCursorVisible(true);
}
void Game::onPreUpdate(const double& dt){
}
void Game::onPostUpdate(const double& dt){
    auto& window = Resources::getWindow();
    const auto& size = window.getSize();
    const glm::vec2 halfRes(size.x / 2, size.y / 2);
    if (window.hasFocus()) {
        glm::vec2 mousePos = Engine::getMousePosition();
        float mouseDistFromCenter = glm::distance(mousePos, halfRes);
        if (mouseDistFromCenter > 1.0f) {
            sf::Mouse::setPosition(sf::Vector2i(static_cast<int>(halfRes.x), static_cast<int>(halfRes.y)), window.getSFMLHandle());
            Engine::setMousePosition(halfRes, false, true);
        }
    }
}
void Game::onJoystickButtonPressed(){
}
void Game::onJoystickButtonReleased(){
}
void Game::onJoystickMoved(){
}
void Game::onJoystickConnected(){
}
void Game::onJoystickDisconnected(){
}
#pragma endregion
