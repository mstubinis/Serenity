#include "core/engine/Engine.h"
#include "core/engine/Engine_Window.h"
#include "core/engine/Engine_Networking.h"
#include "SolarSystem.h"
#include "HUD.h"
#include "Ship.h"
#include "Planet.h"
#include "GameCamera.h"
#include "CapsuleSpace.h"
#include "core/Material.h"
#include "core/engine/mesh/Mesh.h"
#include "core/engine/textures/Texture.h"
#include "core/engine/lights/Light.h"

#include <unordered_map>
#include <iostream>
#include <glm/vec2.hpp>

#include "ResourceManifest.h"

using namespace Engine;

HUD* m_HUD;
void Game::cleanup(){
    delete m_HUD;
}


void Game::initResources(){
    Resources::getWindow().setIcon("data/Textures/icon.png");

    ResourceManifest::init();
}
void Game::initLogic(){
    Engine_Window& window = Resources::getWindow();
    window.keepMouseInWindow(true);
    window.setMouseCursorVisible(false);
    window.setKeyRepeatEnabled(false);
    //apparently these 2 should not be used together, but i have not found any issues with it so far
    //window.setVerticalSyncEnabled(true);
    //window.setFramerateLimit(60);
    SolarSystem* sol = new SolarSystem("Sol", "data/Systems/Sol.txt");
    CapsuleSpace* cap = new CapsuleSpace();

    Resources::setCurrentScene("Sol");
    //Resources::Settings::enableDynamicMemory();

    m_HUD = new HUD();
}
void Game::update(const float& dt){
    if (Engine::isKeyDownOnce(KeyboardKey::Space)) {
        Engine::pause(!Engine::paused());
    }
    if (Engine::isKeyDown(KeyboardKey::Escape)) {
        Engine::stop();
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F4)) {
        Resources::setCurrentScene("Sol");
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F5)) {
        Resources::setCurrentScene("CapsuleSpace");
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F6)) {
        Resources::getWindow().setFullScreen(!Resources::getWindow().isFullscreen());
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F7)) { Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::None); }
    if (Engine::isKeyDownOnce(KeyboardKey::F8)) { Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::SMAA); }
    if (Engine::isKeyDownOnce(KeyboardKey::F9)) { Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::FXAA); }
    if (Engine::isKeyDownOnce(KeyboardKey::F10)) { Renderer::Settings::SSAO::enable(!Renderer::Settings::SSAO::enabled()); }
    if (Engine::isKeyDownOnce(KeyboardKey::F11)) { 
        Renderer::Settings::General::enable1(!Renderer::Settings::General::enabled1()); 
        //Renderer::Settings::Bloom::enable(!Renderer::Settings::Bloom::enabled());
    }
    if (Engine::isKeyDownOnce(KeyboardKey::F12)) { Renderer::Settings::GodRays::enable(!Renderer::Settings::GodRays::enabled()); }

    if (Engine::isKeyDown(KeyboardKey::N)) { Renderer::Settings::Lighting::setGIContributionDiffuse(Renderer::Settings::Lighting::getGIContributionDiffuse() - 0.002f); }
    if (Engine::isKeyDown(KeyboardKey::M)) { Renderer::Settings::Lighting::setGIContributionDiffuse(Renderer::Settings::Lighting::getGIContributionDiffuse() + 0.002f); }
    if (Engine::isKeyDown(KeyboardKey::V)) { Renderer::Settings::Lighting::setGIContributionSpecular(Renderer::Settings::Lighting::getGIContributionSpecular() - 0.002f); }
    if (Engine::isKeyDown(KeyboardKey::B)) { Renderer::Settings::Lighting::setGIContributionSpecular(Renderer::Settings::Lighting::getGIContributionSpecular() + 0.002f); }
    if (Engine::isKeyDown(KeyboardKey::O)) { Renderer::Settings::Lighting::setGIContributionGlobal(Renderer::Settings::Lighting::getGIContributionGlobal() - 0.002f); }
    if (Engine::isKeyDown(KeyboardKey::P)) { Renderer::Settings::Lighting::setGIContributionGlobal(Renderer::Settings::Lighting::getGIContributionGlobal() + 0.002f); }
    //if (Engine::isKeyDown(KeyboardKey::U)) { Renderer::Settings::SSAO::setIntensity(Renderer::Settings::SSAO::getIntensity() - 0.02f); }
    //if (Engine::isKeyDown(KeyboardKey::I)) { Renderer::Settings::SSAO::setIntensity(Renderer::Settings::SSAO::getIntensity() + 0.02f); }

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
