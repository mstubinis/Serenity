#include "Engine.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Engine_Sounds.h"
#include "Engine_Events.h"
#include "Engine_Noise.h"
#include "Camera.h"
#include "GBuffer.h"
#include "Scene.h"
#include "Texture.h"
#include "ObjectDynamic.h"
#include "Light.h"
#include "Engine_Window.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SFML/System.hpp>

#ifdef _DEBUG
#include <iostream>
#endif

sf::Clock Engine::Detail::EngineClass::clock = sf::Clock();

void Engine::Detail::EngineClass::init(uint api, const char* name,uint w,uint h){
    Resources::Detail::ResourceManagement::m_Window = new Engine_Window(name,w,h,api);
    initGame(api);
}
void Engine::Detail::EngineClass::destruct(){
    Game::cleanup();
    Engine::Resources::Detail::ResourceManagement::destruct();
    Engine::Physics::Detail::PhysicsManagement::destruct();
    Engine::Renderer::Detail::RenderManagement::destruct();
    Engine::Sound::Detail::SoundManagement::destruct();
}
void Engine::Detail::EngineClass::initGame(uint api){
    if(api == ENGINE_RENDERING_API_DIRECTX)
        return;
	const glm::uvec2 halfRes = glm::uvec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2);
	Events::Mouse::setMousePosition(halfRes);
	Events::Mouse::MouseProcessing::m_Difference = glm::vec2(0);

	Math::Noise::Detail::MathNoiseManagement::_initFromSeed(unsigned long long(time(0)));
	Renderer::Detail::RenderManagement::init();
    Physics::Detail::PhysicsManagement::init();
    Sound::Detail::SoundManagement::init();

    Resources::initResources();

    Game::initResources();
    Game::initLogic();

    //the scene is the root of all games. create the default scene if 1 does not exist already
    if(Resources::Detail::ResourceManagement::m_Scenes.size() == 0)
        new Scene("Default");

    Resources::initRenderingContexts(api);
}
void Engine::Detail::EngineClass::RESET_EVENTS(){
    Events::Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
    Events::Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;

    for(auto iterator:Engine::Events::Keyboard::KeyProcessing::m_KeyStatus){ iterator.second = false; }
    for(auto iterator:Engine::Events::Mouse::MouseProcessing::m_MouseStatus){ iterator.second = false; }

    Events::Mouse::MouseProcessing::m_Delta *= 0.97f * (1.0f-Resources::dt());
}
void Engine::Detail::EngineClass::update(uint api){
	float dt = Resources::dt();
	Game::onPreUpdate(dt);
    Game::update(dt);
    Resources::getCurrentScene()->update(dt);
    Engine::Physics::Detail::PhysicsManagement::update(dt);
    Events::Mouse::MouseProcessing::m_Difference *= (0.975f);
    RESET_EVENTS();
	Game::onPostUpdate(dt);
}
void Engine::Detail::EngineClass::render(uint api){
    if(api == ENGINE_RENDERING_API_OPENGL){
        Game::render();
        Engine::Renderer::Detail::RenderManagement::render();
        Resources::getWindow()->display();
    }
    else if(api == ENGINE_RENDERING_API_DIRECTX){
        //Game::render();
        Engine::Renderer::Detail::RenderManagement::renderDirectX();
        //Resources::getWindow()->display();
    }
}
#pragma region Event Handler Methods
void Engine::Detail::EngineClass::EVENT_RESIZE(uint api, unsigned int w, unsigned int h,bool saveSize){
    Renderer::Detail::RenderManagement::m_2DProjectionMatrix = glm::ortho(0.0f,(float)w,0.0f,(float)h,0.005f,1000.0f);
    for(auto camera:Resources::Detail::ResourceManagement::m_Cameras){ camera.second.get()->resize(w,h); }
    if(api == ENGINE_RENDERING_API_OPENGL){
        glViewport(0,0,w,h);
        Renderer::Detail::RenderManagement::m_gBuffer->resizeBaseBuffer(w,h);
        for(unsigned int i = 0; i < BUFFER_TYPE_NUMBER; i++){
            Renderer::Detail::RenderManagement::m_gBuffer->resizeBuffer(i,w,h);
        }
    }
    else if(api == ENGINE_RENDERING_API_DIRECTX){
    }

    if(saveSize) Engine::Resources::getWindow()->setSize(w,h);
	Game::onResize(w,h);
}
void Engine::Detail::EngineClass::EVENT_CLOSE(){
    Resources::getWindow()->close();
	Game::onClose();
}
void Engine::Detail::EngineClass::EVENT_LOST_FOCUS(){
	Game::onLostFocus();
}
void Engine::Detail::EngineClass::EVENT_GAINED_FOCUS(){
	Game::onGainedFocus();
}
void Engine::Detail::EngineClass::EVENT_TEXT_ENTERED(sf::Event::TextEvent text){
	Game::onTextEntered(text);
}
void Engine::Detail::EngineClass::EVENT_KEY_PRESSED(uint key){
    Events::Keyboard::KeyProcessing::m_previousKey = Events::Keyboard::KeyProcessing::m_currentKey;
    Events::Keyboard::KeyProcessing::m_currentKey = key;
    Events::Keyboard::KeyProcessing::m_KeyStatus[key] = true;
	Game::onKeyPressed(key);
}
void Engine::Detail::EngineClass::EVENT_KEY_RELEASED(uint key){
    Events::Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
    Events::Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;
    Events::Keyboard::KeyProcessing::m_KeyStatus[key] = false;
	Game::onKeyReleased(key);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_WHEEL_MOVED(sf::Event::MouseWheelEvent mouseWheel){
    Events::Mouse::MouseProcessing::m_Delta += (mouseWheel.delta * 10);
	Game::onMouseWheelMoved(mouseWheel);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_BUTTON_PRESSED(sf::Event::MouseButtonEvent mouseButton){
    Events::Mouse::MouseProcessing::m_previousButton = Engine::Events::Mouse::MouseProcessing::m_currentButton;
    Events::Mouse::MouseProcessing::m_currentButton = mouseButton.button;
    Events::Mouse::MouseProcessing::m_MouseStatus[mouseButton.button] = true;
	Game::onMouseButtonPressed(mouseButton);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_BUTTON_RELEASED(sf::Event::MouseButtonEvent mouseButton){
    Events::Mouse::MouseProcessing::m_previousButton = 100; //we will use 100 as the "none" key
    Events::Mouse::MouseProcessing::m_currentButton = 100;  //we will use 100 as the "none" key
    Events::Mouse::MouseProcessing::m_MouseStatus[mouseButton.button] = false;
	Game::onMouseButtonReleased(mouseButton);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_MOVED(sf::Event::MouseMoveEvent mouse){
    if(Resources::getWindow()->hasFocus()){
		Events::Mouse::MouseProcessing::_SetMousePositionInternal(float(mouse.x),float(mouse.y));
    }
	Game::onMouseMoved(mouse);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_ENTERED(){
	Game::onMouseEntered();
}
void Engine::Detail::EngineClass::EVENT_MOUSE_LEFT(){
	Game::onMouseLeft();
}
void Engine::Detail::EngineClass::EVENT_JOYSTICK_BUTTON_PRESSED(){
	Game::onJoystickButtonPressed();
}
void Engine::Detail::EngineClass::EVENT_JOYSTICK_BUTTON_RELEASED(){
	Game::onJoystickButtonReleased();
}
void Engine::Detail::EngineClass::EVENT_JOYSTICK_MOVED(){
	Game::onJoystickMoved();
}
void Engine::Detail::EngineClass::EVENT_JOYSTICK_CONNECTED(){
	Game::onJoystickConnected();
}
void Engine::Detail::EngineClass::EVENT_JOYSTICK_DISCONNECTED(){
	Game::onJoystickDisconnected();
}
#pragma endregion
const float Engine::getFPS(){ return 1.0f / Resources::dt(); }
Engine_Window* Engine::getWindow(){ return Resources::Detail::ResourceManagement::m_Window; }
const sf::Vector2u& Engine::getWindowSize(){ return Resources::Detail::ResourceManagement::m_Window->getSize(); }
void Engine::setWindowIcon(Texture* texture){ Resources::getWindow()->setIcon(texture); }
void Engine::showMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(true); }
void Engine::hideMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(false); }
void Engine::stop(){ Resources::getWindow()->close(); }
void Engine::setFullScreen(bool b){ Engine::Resources::Detail::ResourceManagement::m_Window->setFullScreen(b); }
void Engine::Detail::EngineClass::run(){
    while(Resources::getWindow()->isOpen()){
        sf::Event e;
        Resources::Detail::ResourceManagement::m_DeltaTime = clock.restart().asSeconds();
        while(Resources::getWindow()->getSFMLHandle()->pollEvent(e)){
            switch (e.type){
                case sf::Event::Closed:               EVENT_CLOSE();break;
                case sf::Event::KeyReleased:          EVENT_KEY_RELEASED(e.key.code);break;
                case sf::Event::KeyPressed:           EVENT_KEY_PRESSED(e.key.code);break;
                case sf::Event::MouseButtonPressed:   EVENT_MOUSE_BUTTON_PRESSED(e.mouseButton);break;
                case sf::Event::MouseButtonReleased:  EVENT_MOUSE_BUTTON_RELEASED(e.mouseButton);break;
                case sf::Event::MouseEntered:         EVENT_MOUSE_ENTERED();break;
                case sf::Event::MouseLeft:            EVENT_MOUSE_LEFT();break;
                case sf::Event::MouseWheelMoved:      EVENT_MOUSE_WHEEL_MOVED(e.mouseWheel);break;
                case sf::Event::MouseMoved:           EVENT_MOUSE_MOVED(e.mouseMove);break;
                case sf::Event::Resized:              EVENT_RESIZE(Resources::Detail::ResourceManagement::m_RenderingAPI,e.size.width,e.size.height);break;
                case sf::Event::TextEntered:          EVENT_TEXT_ENTERED(e.text);break;
                default:                              break;
            }
        }
        update(Resources::Detail::ResourceManagement::m_RenderingAPI);
        render(Resources::Detail::ResourceManagement::m_RenderingAPI);
    }
}