#include <core/engine/system/EngineGameFunctions.h>
#include <core/engine/system/window/Engine_Window.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/physics/Engine_Physics.h>

#include <glm/vec2.hpp>

#include "Core.h"
#include <core/engine/system/Engine.h>

#include "ResourceManifest.h"
#include <core/engine/discord/Discord.h>

using namespace Engine;
using namespace std;

Core* m_Core = nullptr;

void Game::initResources(){
    Discord::activate(661384805786845214);
    
    Discord::DiscordActivity activity;
    activity.setDetail("At Main Menu");
    activity.setType(discord::ActivityType::Playing);
    activity.setInstance(false);
    activity.setState("Playing Solo");
    activity.setTimestampStart(0);
    activity.setTimestampEnd(0);
    activity.setImageLarge("large_icon");
    activity.setImageLargeText("");
    activity.setImageSmallText("");

    Discord::update_activity(activity);

    m_Core = NEW Core();

    ResourceManifest::init();
    //const std::string& iconPath = ResourceManifest::BasePath + "data/Textures/icon.png";
    //Resources::getWindow().setIcon(iconPath);
}
void Game::initLogic(){
    Physics::setNumberOfStepsPerFrame(3);
    m_Core->init();

}
void Game::cleanup() {
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
void Game::onResize(const uint& width, const uint& height){
    if (m_Core) {
        m_Core->onResize(width, height);
    }
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
        Resources::getWindow().requestFocus();
        Resources::getWindow().keepMouseInWindow(true);
    }
}
void Game::onMouseLeft(){
    if (m_Core && m_Core->gameState() == GameState::Game) {
        Resources::getWindow().keepMouseInWindow(false);
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
