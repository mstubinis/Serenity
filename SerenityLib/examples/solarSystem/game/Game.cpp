#include "Core.h"
#include <core/engine/Engine.h>
#include <core/engine/lua/LuaScript.h>

#include <unordered_map>
#include <iostream>
#include <glm/vec2.hpp>

using namespace Engine;

Core* m_Core;
void Game::cleanup(){
    SAFE_DELETE(m_Core);
}
void Game::initResources(){
    m_Core = new Core();
}
void Game::initLogic(){
    m_Core->init();
}
void Game::update(const double& dt){
    m_Core->update(dt);
}
void Game::render(){
    m_Core->render();
}

#pragma region EventHandlers
void Game::onResize(const uint& width, const uint& height){
    if(m_Core)
        m_Core->onResize(width, height);
}
void Game::onClose(){
}
void Game::onLostFocus(){
}
void Game::onGainedFocus(){

    if (m_Core && m_Core->gameState() == GameState::Game) {
        const auto& size = Resources::getWindow().getSize();
        const glm::vec2 halfRes(size.x / 2, size.y / 2);
        Engine::setMousePosition(halfRes, true);
    }
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
    if (m_Core && m_Core->gameState() == GameState::Game) {
        Engine::getWindow().requestFocus();
        Engine::getWindow().keepMouseInWindow(true);
    }
}
void Game::onMouseLeft(){
    if (m_Core && m_Core->gameState() == GameState::Game) {
        Engine::getWindow().keepMouseInWindow(false);
    }
}
void Game::onPreUpdate(const double& dt){
}
void Game::onPostUpdate(const double& dt){
    if (m_Core->gameState() == GameState::Game) {
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
