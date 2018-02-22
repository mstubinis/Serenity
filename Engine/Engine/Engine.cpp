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

class Engine::impl::Core::impl final{
    public:
		void _init(){
		}
		void _destruct(){
		}
};


Engine::impl::Core* Engine::impl::Core::m_Engine = nullptr;
Engine::impl::Core::Core(const char* name,uint w,uint h):m_i(new impl){
	m_EventManager = new Engine::impl::EventManager();
	m_ResourceManager = new Engine::impl::ResourceManager(name,w,h);
	m_TimeManager = new Engine::impl::TimeManager();
	m_i->_init();
}
Engine::impl::Core::~Core(){
	delete m_EventManager;
	delete m_ResourceManager;
	delete m_TimeManager;
	m_i->_destruct();
}

void Engine::init(const char* name,uint w,uint h){
	Engine::impl::Core::m_Engine = new Engine::impl::Core(name,w,h);
	Detail::EngineClass::initGame(name,w,h);
}
void Engine::destruct(){
    Game::cleanup();
	delete Engine::impl::Core::m_Engine;
	Engine::Resources::Detail::ResourceManagement::destruct();
	Engine::Physics::Detail::PhysicsManagement::destruct();
	Engine::Sound::Detail::SoundManagement::destruct();
    Engine::Renderer::Detail::RenderManagement::destruct();
}
void Engine::Detail::EngineClass::initGame(const char* name,uint w,uint h){
    Math::Noise::Detail::MathNoiseManagement::_initFromSeed(unsigned long long(time(0)));
    Physics::Detail::PhysicsManagement::init();
    Sound::Detail::SoundManagement::init();
	Resources::initResources(name,w,h);
    Renderer::Detail::RenderManagement::init();

    Renderer::Detail::RenderManagement::postInit();
	Engine::setMousePosition(glm::uvec2(Resources::getWindowSize().x/2,Resources::getWindowSize().y/2));
    Game::initResources();
    Game::initLogic();

    //the scene is the root of all games. create the default scene if 1 does not exist already
	if(Engine::impl::Core::m_Engine->m_ResourceManager->_numScenes() == 0)
        new Scene("Default");
}
void Engine::Detail::EngineClass::RESET_EVENTS(){
    impl::Core::m_Engine->m_EventManager->_onResetEvents();
}
void Engine::Detail::EngineClass::update(){
	Engine::impl::Core::m_Engine->m_TimeManager->stop_update();

	float dt = Engine::impl::Core::m_Engine->m_TimeManager->dt();
    Game::onPreUpdate(dt);
    Game::update(dt);
    Resources::getCurrentScene()->update(dt);
    impl::Core::m_Engine->m_EventManager->_onUpdate(dt);
    RESET_EVENTS();
    Game::onPostUpdate(dt);

	Engine::impl::Core::m_Engine->m_TimeManager->calculate_update();
}
void Engine::Detail::EngineClass::updatePhysics(){
	Engine::impl::Core::m_Engine->m_TimeManager->stop_physics();
	Physics::Detail::PhysicsManagement::update(Resources::dt());
	Engine::impl::Core::m_Engine->m_TimeManager->calculate_physics();
}
void Engine::Detail::EngineClass::updateSounds(){
	Engine::impl::Core::m_Engine->m_TimeManager->stop_sounds();
	Sound::Detail::SoundManagement::update(Resources::dt());
	Engine::impl::Core::m_Engine->m_TimeManager->calculate_sounds();
}
void Engine::Detail::EngineClass::render(){
	Engine::impl::Core::m_Engine->m_TimeManager->stop_render();

    Game::render(); uint x = Resources::getWindowSize().x; uint y = Resources::getWindowSize().y;
    Renderer::Detail::RenderManagement::render(Renderer::Detail::RenderManagement::m_gBuffer,Resources::getActiveCamera(),x,y);

	Engine::impl::Core::m_Engine->m_TimeManager->stop_rendering_display();
    Resources::getWindow()->display();
	Engine::impl::Core::m_Engine->m_TimeManager->calculate_rendering_display();

	Engine::impl::Core::m_Engine->m_TimeManager->calculate_render();
}
#pragma region Event Handler Methods
void Engine::Detail::EngineClass::EVENT_RESIZE(uint w, uint h,bool saveSize){
    Renderer::Detail::RenderManagement::m_2DProjectionMatrix = glm::ortho(0.0f,(float)w,0.0f,(float)h,0.005f,1000.0f);
	Engine::impl::Core::m_Engine->m_ResourceManager->_resizeCameras(w,h);
    Renderer::setViewport(0,0,w,h);
    Renderer::Detail::RenderManagement::m_gBuffer->resize(w,h);
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
	impl::Core::m_Engine->m_EventManager->_onEventKeyPressed(key);
    Game::onKeyPressed(key);
}
void Engine::Detail::EngineClass::EVENT_KEY_RELEASED(uint key){
    impl::Core::m_Engine->m_EventManager->_onEventKeyReleased(key);
    Game::onKeyReleased(key);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_WHEEL_MOVED(int delta){
    impl::Core::m_Engine->m_EventManager->_onEventMouseWheelMoved(delta);
    Game::onMouseWheelMoved(delta);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_BUTTON_PRESSED(uint mouseButton){
	impl::Core::m_Engine->m_EventManager->_onEventMouseButtonPressed(mouseButton);
    Game::onMouseButtonPressed(mouseButton);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_BUTTON_RELEASED(uint mouseButton){
	impl::Core::m_Engine->m_EventManager->_onEventMouseButtonReleased(mouseButton);
    Game::onMouseButtonReleased(mouseButton);
}
void Engine::Detail::EngineClass::EVENT_MOUSE_MOVED(float mouseX, float mouseY){
    if(Resources::getWindow()->hasFocus()){
        impl::Core::m_Engine->m_EventManager->_setMousePosition(mouseX,mouseY,false,false);
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

        Detail::EngineClass::update();
		Detail::EngineClass::updatePhysics();
		Detail::EngineClass::updateSounds();
        Detail::EngineClass::render();
		
		Engine::impl::Core::m_Engine->m_TimeManager->calculate();
    }
	Engine::destruct();
}
