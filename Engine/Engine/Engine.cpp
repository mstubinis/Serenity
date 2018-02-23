#include "Engine.h"
#include "Engine_Time.h"
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

#include <glm/vec2.hpp>

#include <SFML/System.hpp>

#ifdef _DEBUG
#include <iostream>
#endif

using namespace Engine;

class epriv::Core::impl final{
    public:
		void _init(){
		}
		void _destruct(){
		}
};


epriv::Core* epriv::Core::m_Engine = nullptr;
epriv::Core::Core(const char* name,uint w,uint h):m_i(new impl){
	m_EventManager = new epriv::EventManager();
	m_ResourceManager = new epriv::ResourceManager(name,w,h);
	m_TimeManager = new epriv::TimeManager();
	m_SoundManager = new epriv::SoundManager();
	m_PhysicsManager = new epriv::PhysicsManager();
	m_RenderManager = new epriv::RenderManager(name,w,h);
	m_i->_init();
}
epriv::Core::~Core(){
	SAFE_DELETE(m_EventManager);
	SAFE_DELETE(m_SoundManager);
	SAFE_DELETE(m_ResourceManager);
	SAFE_DELETE(m_TimeManager);
	SAFE_DELETE(m_PhysicsManager);
	SAFE_DELETE(m_RenderManager);
	m_i->_destruct();
}

void Engine::init(const char* name,uint w,uint h){
	epriv::Core::m_Engine = new epriv::Core(name,w,h);

	epriv::Core::m_Engine->m_ResourceManager->_init(name,w,h);
	epriv::Core::m_Engine->m_RenderManager->_init(w,h);
	
	Detail::EngineClass::initGame(name,w,h);
}

void Engine::Detail::EngineClass::initGame(const char* name,uint w,uint h){
    Math::Noise::Detail::MathNoiseManagement::_initFromSeed(unsigned long long(time(0)));
	
	Engine::setMousePosition(w/2,h/2);
    Game::initResources();
    Game::initLogic();

    //the scene is the root of all games. create the default scene if 1 does not exist already
	if(epriv::Core::m_Engine->m_ResourceManager->_numScenes() == 0)
        new Scene("Default");
}
void Engine::Detail::EngineClass::RESET_EVENTS(){
    epriv::Core::m_Engine->m_EventManager->_onResetEvents();
}
void Engine::Detail::EngineClass::update(float dt){
	epriv::Core::m_Engine->m_TimeManager->stop_update();

    Game::onPreUpdate(dt);
    Game::update(dt);
    Resources::getCurrentScene()->update(dt);

    epriv::Core::m_Engine->m_EventManager->_update(dt);
    RESET_EVENTS();
    Game::onPostUpdate(dt);

	epriv::Core::m_Engine->m_TimeManager->calculate_update();
}
void Engine::Detail::EngineClass::updatePhysics(float dt){
	epriv::Core::m_Engine->m_TimeManager->stop_physics();

	epriv::Core::m_Engine->m_PhysicsManager->_update(dt);

	epriv::Core::m_Engine->m_TimeManager->calculate_physics();
}
void Engine::Detail::EngineClass::updateSounds(float dt){
	epriv::Core::m_Engine->m_TimeManager->stop_sounds();

	epriv::Core::m_Engine->m_SoundManager->_update(dt);

	epriv::Core::m_Engine->m_TimeManager->calculate_sounds();
}
void Engine::Detail::EngineClass::render(){
	epriv::Core::m_Engine->m_TimeManager->stop_render();

    Game::render();
	glm::uvec2 winSize = Resources::getWindowSize();
	epriv::Core::m_Engine->m_RenderManager->_render(Resources::getCurrentScene()->getActiveCamera(),winSize.x,winSize.y);

	epriv::Core::m_Engine->m_TimeManager->stop_rendering_display();
    Resources::getWindow()->display();
	epriv::Core::m_Engine->m_TimeManager->calculate_rendering_display();

	epriv::Core::m_Engine->m_TimeManager->calculate_render();
}
#pragma region Event Handler Methods
void Engine::Detail::EngineClass::EVENT_RESIZE(uint w, uint h,bool saveSize){
	epriv::Core::m_Engine->m_RenderManager->_resize(w,h);

	epriv::Core::m_Engine->m_ResourceManager->_resizeCameras(w,h);
    Renderer::setViewport(0,0,w,h);
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
void Engine::Detail::EngineClass::EVENT_TEXT_ENTERED(uint unicode){
    Game::onTextEntered(unicode);
}
void Engine::Detail::EngineClass::EVENT_KEY_PRESSED(uint key){
	epriv::Core::m_Engine->m_EventManager->_onEventKeyPressed(key);
    Game::onKeyPressed(key);
}
void Engine::Detail::EngineClass::EVENT_KEY_RELEASED(uint key){
    epriv::Core::m_Engine->m_EventManager->_onEventKeyReleased(key);
    Game::onKeyReleased(key);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_WHEEL_MOVED(int delta){
    epriv::Core::m_Engine->m_EventManager->_onEventMouseWheelMoved(delta);
    Game::onMouseWheelMoved(delta);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_BUTTON_PRESSED(uint mouseButton){
	epriv::Core::m_Engine->m_EventManager->_onEventMouseButtonPressed(mouseButton);
    Game::onMouseButtonPressed(mouseButton);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_BUTTON_RELEASED(uint mouseButton){
	epriv::Core::m_Engine->m_EventManager->_onEventMouseButtonReleased(mouseButton);
    Game::onMouseButtonReleased(mouseButton);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_MOVED(float mouseX, float mouseY){
    if(Resources::getWindow()->hasFocus()){
        epriv::Core::m_Engine->m_EventManager->_setMousePosition(mouseX,mouseY,false,false);
    }
    Game::onMouseMoved(mouseX,mouseY);
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
Engine_Window* Engine::getWindow(){ return Engine::Resources::getWindow(); }
const glm::uvec2 Engine::getWindowSize(){ return Engine::Resources::getWindowSize(); }
void Engine::setWindowIcon(Texture* texture){ Resources::getWindow()->setIcon(texture); }
void Engine::showMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(true); }
void Engine::hideMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(false); }
void Engine::stop(){ Resources::getWindow()->close(); }
void Engine::setFullScreen(bool b){ Engine::Resources::getWindow()->setFullScreen(b); }

void Engine::Detail::EngineClass::handleEvents(){
	sf::Event e;
    while(Resources::getWindow()->getSFMLHandle()->pollEvent(e)){
        switch (e.type){
            case sf::Event::Closed:               EVENT_CLOSE();break;
            case sf::Event::KeyReleased:          EVENT_KEY_RELEASED(e.key.code);break;
            case sf::Event::KeyPressed:           EVENT_KEY_PRESSED(e.key.code);break;
            case sf::Event::MouseButtonPressed:   EVENT_MOUSE_BUTTON_PRESSED(e.mouseButton.button);break;
            case sf::Event::MouseButtonReleased:  EVENT_MOUSE_BUTTON_RELEASED(e.mouseButton.button);break;
            case sf::Event::MouseEntered:         EVENT_MOUSE_ENTERED();break;
            case sf::Event::MouseLeft:            EVENT_MOUSE_LEFT();break;
			case sf::Event::MouseWheelMoved:      EVENT_MOUSE_WHEEL_MOVED(e.mouseWheel.delta);break;
			case sf::Event::MouseMoved:           EVENT_MOUSE_MOVED(e.mouseMove.x,e.mouseMove.y);break;
            case sf::Event::Resized:              EVENT_RESIZE(e.size.width,e.size.height);break;
            case sf::Event::TextEntered:          EVENT_TEXT_ENTERED(e.text.unicode);break;
            default:                              break;
        }
    }
}

void Engine::run(){
    while(Resources::getWindow()->isOpen()){
		Detail::EngineClass::handleEvents();

		float dt = epriv::Core::m_Engine->m_TimeManager->dt();
        Detail::EngineClass::update(dt);
		Detail::EngineClass::updatePhysics(dt);
		Detail::EngineClass::updateSounds(dt);

        Detail::EngineClass::render();
		
		epriv::Core::m_Engine->m_TimeManager->calculate();
    }
	//destruct the engine here
    Game::cleanup();
	SAFE_DELETE(epriv::Core::m_Engine);
}
