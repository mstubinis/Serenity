#include <core/engine/system/EngineGameFunctions.h>
#include <core/engine/system/window/Engine_Window.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/physics/Engine_Physics.h>

#include <glm/vec2.hpp>

#include "Core.h"
#include "config/ConfigFile.h"
#include <core/engine/system/Engine.h>

#include "ResourceManifest.h"
#include <core/engine/discord/Discord.h>

#include <chrono>
#include <thread>

#include "database/Database.h"


using namespace Engine;
using namespace std;

Core* m_Core = nullptr;

void Game::initResources(){
    auto& window = Resources::getWindow();
    window.setIcon("../data/Textures/icon.png");

    Discord::activate(661384805786845214);
    
    m_Core = NEW Core();
    ResourceManifest::init();
}
void Game::initLogic(){
    auto& window = Resources::getWindow();
    Physics::setNumberOfStepsPerFrame(3);

    m_Core->init();
    Game::onResize(window, window.getSize().x, window.getSize().y);
}
void Game::cleanup() {
    Discord::clear_activity();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    SAFE_DELETE(m_Core);
    ResourceManifest::destruct();
}
void Game::update(const double& dt){
    if (Engine::isKeyDown(KeyboardKey::Escape)) {
        Engine::stop();
    }
    m_Core->update(dt);
}
void Game::render(){
    m_Core->render();
}

#pragma region EventHandlers
void Game::onResize(Engine_Window& window, const uint& width, const uint& height){
    if (m_Core) {
        m_Core->onResize(width, height);

        ConfigFile config;
        config.updateWindowSize(width, height);
    }
}
void Game::onWindowRequestedToBeClosed(Engine_Window& window) {

}
void Game::onWindowClosed(Engine_Window& window) {

}
void Game::onGameEnded(){
    auto& window = Resources::getWindow();
    ConfigFile config;
    const bool isMaximized = window.isMaximized();
    config.updateWindowMaximized(isMaximized);
}
void Game::onLostFocus(Engine_Window& window){
}
void Game::onGainedFocus(Engine_Window& window){
    if (m_Core && m_Core->gameState() == GameState::Game) {
        window.keepMouseInWindow(true);
        window.setMouseCursorVisible(false);

        const auto& size = window.getSize();
        const glm::vec2 halfRes(size.x / 2, size.y / 2);

        Engine::setMousePosition(halfRes, true);

    }
}
void Game::onTextEntered(Engine_Window& window, const uint& unicode){
}
void Game::onKeyPressed(Engine_Window& window, const uint& key){
}
void Game::onKeyReleased(Engine_Window& window, const uint& key){
}
void Game::onMouseWheelScrolled(Engine_Window& window, const int& delta, const int& x, const int& y){
}
void Game::onMouseButtonPressed(Engine_Window& window, const uint& button){
}
void Game::onMouseButtonReleased(Engine_Window& window, const uint& button){
}
void Game::onMouseMoved(Engine_Window& window, const float& mouseX, const float& mouseY){
}
void Game::onMouseEntered(Engine_Window& window){
    window.requestFocus();
    if (m_Core && m_Core->gameState() == GameState::Game) {  
        window.keepMouseInWindow(true);
        window.setMouseCursorVisible(false);
    }
}
void Game::onMouseLeft(Engine_Window& window){
    if (m_Core && m_Core->gameState() == GameState::Game) {
        window.keepMouseInWindow(false);
        window.setMouseCursorVisible(true);
    }
}
void Game::onPreUpdate(const double& dt){
}
void Game::onPostUpdate(const double& dt){
    if (m_Core && m_Core->gameState() == GameState::Game) {
        auto& window = Resources::getWindow();
        const auto& size = window.getSize();
        const glm::vec2 halfRes(size.x / 2, size.y / 2);
        if (window.hasFocus()) {
            const glm::vec2& mousePos = Engine::getMousePosition();
            const float& mouseDistFromCenter = glm::distance(mousePos, halfRes);
            if (mouseDistFromCenter > 1.0f) {
                Engine::setMousePosition(halfRes, false, true);
            }
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
