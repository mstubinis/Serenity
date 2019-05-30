#include "SolarSystem.h"
#include "HUD.h"
#include "Ship.h"
#include "Planet.h"
#include "GameCamera.h"
#include "ResourceManifest.h"

#include <core/Material.h>

#include <core/engine/Engine.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/Engine_Networking.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/lights/Light.h>

#include <core/engine/lua/LuaScript.h>


#include <unordered_map>
#include <iostream>
#include <glm/vec2.hpp>

using namespace Engine;

HUD* m_HUD;
void Game::cleanup(){
    SAFE_DELETE(m_HUD);
}

void Game::initResources(){
    ResourceManifest::init();
    const std::string& iconPath = ResourceManifest::BasePath + "data/Textures/icon.png";
    Resources::getWindow().setIcon(iconPath);
}
void Game::initLogic(){
    auto& window = Resources::getWindow();
    window.keepMouseInWindow(true);
    window.setMouseCursorVisible(false);
    window.setKeyRepeatEnabled(false);
    SolarSystem* sol = new SolarSystem("Sol", ResourceManifest::BasePath + "data/Systems/Sol.txt");

    Resources::setCurrentScene("Sol");

    m_HUD = new HUD();


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
}
void Game::update(const float& dt){
    if (Engine::isKeyDownOnce(KeyboardKey::Space)) {
        Engine::pause(!Engine::paused());
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
        Renderer::Settings::GodRays::enable(!Renderer::Settings::GodRays::enabled()); 
    }
    m_HUD->update(dt);
}
void Game::render(){
    m_HUD->render();
}

#pragma region EventHandlers
void Game::onResize(uint width,uint height){
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
    const glm::vec2 halfRes = glm::vec2(size.x / 2, size.y / 2);
    sf::Mouse::setPosition(sf::Vector2i(int(halfRes.x), int(halfRes.y)), window.getSFMLHandle());

    Engine::setMousePosition(halfRes, true);
}
void Game::onTextEntered(uint unicode){
}
void Game::onKeyPressed(uint key){
}
void Game::onKeyReleased(uint key){
}
void Game::onMouseWheelMoved(int delta){
}
void Game::onMouseButtonPressed(uint button){
}
void Game::onMouseButtonReleased(uint button){
}
void Game::onMouseMoved(float mouseX,float mouseY){
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
void Game::onPreUpdate(float dt){
}
void Game::onPostUpdate(float dt){
    auto& window = Resources::getWindow();
    const auto& size = window.getSize();
    const glm::vec2 halfRes = glm::vec2(size.x / 2, size.y / 2);
    if (window.hasFocus()) {
        glm::vec2 mousePos = Engine::getMousePosition();
        float mouseDistFromCenter = glm::distance(mousePos, halfRes);
        if (mouseDistFromCenter > 1.0f) {
            sf::Mouse::setPosition(sf::Vector2i(int(halfRes.x), int(halfRes.y)), window.getSFMLHandle());
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
