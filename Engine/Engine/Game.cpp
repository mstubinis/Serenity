#include "Engine.h"
#include "Engine_Events.h"
#include "Engine_Renderer.h"
#include "Engine_Sounds.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include "Engine_Noise.h"
#include "Engine_Networking.h"
#include "Engine_ThreadManager.h"
#include "SolarSystem.h"
#include "HUD.h"
#include "Ship.h"
#include "Planet.h"
#include "Lagrange.h"
#include "GameCamera.h"
#include "CapsuleSpace.h"
#include "Material.h"
#include "Texture.h"
#include "Light.h"

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
	Engine_Window& window = *Resources::getWindow();
    window.setIcon("data/Textures/icon.png");

	ResourceManifest::init();
}
void Game::initLogic(){
	Engine_Window& window = *Resources::getWindow();
    window.keepMouseInWindow(true);
    window.setMouseCursorVisible(false);
    window.setKeyRepeatEnabled(false);
	//apparently these 2 should not be used together, but i have not found any issues with it so far
	//window.setVerticalSyncEnabled(true);
	//window.setFramerateLimit(60);

    SolarSystem* sol = new SolarSystem("Sol","data/Systems/Sol.txt");
    CapsuleSpace* cap = new CapsuleSpace();

	Resources::setCurrentScene("Sol");
    m_HUD = new HUD();
}

void Game::update(float dt){
	if(Engine::isKeyDown(KeyboardKey::Escape)){
        Engine::stop();
    }
	if(Engine::isKeyDownOnce(KeyboardKey::F4)){
        Resources::setCurrentScene("Sol");
    }
	if(Engine::isKeyDownOnce(KeyboardKey::F5)){
        Resources::setCurrentScene("CapsuleSpace");
    }
    if(Engine::isKeyDownOnce("f7")){ Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::None); }
    if(Engine::isKeyDownOnce("f8")){ Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::SMAA); }
    if(Engine::isKeyDownOnce("f9")){ Renderer::Settings::setAntiAliasingAlgorithm(AntiAliasingAlgorithm::FXAA); }
	if(Engine::isKeyDownOnce("f10")){ Renderer::Settings::SSAO::enable(!Renderer::Settings::SSAO::enabled()); }

	if(Engine::isKeyDownOnce("space")){

	}
	/*
    if(Engine::isKeyDown("z")){
        Renderer::Settings::HDR::setExposure(Renderer::Settings::HDR::getExposure() - 0.03f);
    }
    else if(Engine::isKeyDown("x")){
        Renderer::Settings::HDR::setExposure(Renderer::Settings::HDR::getExposure() + 0.03f);
    }
    if(Engine::isKeyDown("c")){
        Renderer::Settings::setGamma(Renderer::Settings::getGamma() - 0.02f);
    }
    else if(Engine::isKeyDown("k")){
        Renderer::Settings::setGamma(Renderer::Settings::getGamma() + 0.02f);
    }
	*/
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
    Engine::getWindow()->keepMouseInWindow(false);
    //Engine::getWindow()->setMouseCursorVisible(true);
}
void Game::onGainedFocus(){
    Engine::getWindow()->keepMouseInWindow(true);
    //Engine::getWindow()->setMouseCursorVisible(false);
    const glm::vec2 halfRes = glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
    sf::Mouse::setPosition(sf::Vector2i(int(halfRes.x),int(halfRes.y)),*Resources::getWindow()->getSFMLHandle());

	Engine::setMousePosition(halfRes,true);
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
    Engine::getWindow()->requestFocus();
    Engine::getWindow()->keepMouseInWindow(true);
    //Engine::getWindow()->setMouseCursorVisible(true);
}
void Game::onMouseLeft(){
    Engine::getWindow()->keepMouseInWindow(false);
    //Engine::getWindow()->setMouseCursorVisible(true);
}
void Game::onPreUpdate(float dt){
}
void Game::onPostUpdate(float dt){
    const glm::vec2 halfRes = glm::vec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
    if(Resources::getWindow()->hasFocus()){
        glm::vec2 mousePos = Engine::getMousePosition();
        float mouseDistFromCenter = glm::distance(mousePos,halfRes);
        if(mouseDistFromCenter > 1.0f){
            sf::Mouse::setPosition(sf::Vector2i(int(halfRes.x),int(halfRes.y)),*Resources::getWindow()->getSFMLHandle());
			Engine::setMousePosition(halfRes,false,true);
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
