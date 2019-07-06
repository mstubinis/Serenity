#include "HUD.h"
#include "Ship.h"
#include "GameCamera.h"
#include "CapsuleSpace.h"
#include "ResourceManifest.h"

#include <core/engine/Engine.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/Engine_Networking.h>
#include <core/Material.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/lights/Lights.h>

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
	//window.setFramerateLimit(60);

    CapsuleSpace* cap = new CapsuleSpace();
    Resources::setCurrentScene("CapsuleSpace");
    m_HUD = new HUD();
    
    Renderer::hdr::disable();
    Renderer::godRays::disable();
}
void Game::update(const double& dt){
    if (Engine::isKeyDownOnce(KeyboardKey::Space)) {
        Engine::pause(!Engine::paused());
    }
    if (Engine::isKeyDown(KeyboardKey::Escape)) {
        Engine::stop();
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F6)) {
        Resources::getWindow().setFullScreen(!Resources::getWindow().isFullscreen());
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F7)) { Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::None); }
    if (Engine::isKeyDownOnce(KeyboardKey::F8)) { Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::SMAA); }
    if (Engine::isKeyDownOnce(KeyboardKey::F9)) { Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::FXAA); }
    if (Engine::isKeyDownOnce(KeyboardKey::F10)) { Renderer::ssao::enable(!Renderer::ssao::enabled()); }

    m_HUD->update(dt);
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
