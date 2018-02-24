#include "Engine.h"
#include "Engine_Time.h"
#include "Engine_EventDispatcher.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Engine_Sounds.h"
#include "Engine_Events.h"
#include "Engine_Noise.h"
#include "Components.h"
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

epriv::Core* epriv::Core::m_Engine = nullptr;

bool m_IsKeyDown = false;

epriv::Core::Core(const char* name,uint w,uint h){
	m_EventManager = new epriv::EventManager(name,w,h);
	m_ResourceManager = new epriv::ResourceManager(name,w,h);
	m_TimeManager = new epriv::TimeManager(name,w,h);
	m_SoundManager = new epriv::SoundManager(name,w,h);
	m_PhysicsManager = new epriv::PhysicsManager(name,w,h);
	m_RenderManager = new epriv::RenderManager(name,w,h);
	m_EventDispatcher = new epriv::EventDispatcher(name,w,h);
	m_ComponentManager = new epriv::ComponentManager(name,w,h);
}
epriv::Core::~Core(){
	SAFE_DELETE(m_EventManager);
	SAFE_DELETE(m_SoundManager);
	SAFE_DELETE(m_ResourceManager);
	SAFE_DELETE(m_TimeManager);
	SAFE_DELETE(m_PhysicsManager);
	SAFE_DELETE(m_RenderManager);
	SAFE_DELETE(m_EventDispatcher);
	SAFE_DELETE(m_ComponentManager);
}

void Engine::init(const char* name,uint w,uint h){
	epriv::Core::m_Engine = new epriv::Core(name,w,h);

	epriv::Core::m_Engine->m_ResourceManager->_init(name,w,h);
	epriv::Core::m_Engine->m_RenderManager->_init(name,w,h);
	epriv::Core::m_Engine->m_EventManager->_init(name,w,h);
	epriv::Core::m_Engine->m_TimeManager->_init(name,w,h);
	epriv::Core::m_Engine->m_SoundManager->_init(name,w,h);
	epriv::Core::m_Engine->m_PhysicsManager->_init(name,w,h);
	epriv::Core::m_Engine->m_EventDispatcher->_init(name,w,h);
	epriv::Core::m_Engine->m_ComponentManager->_init(name,w,h);

	//init the game here
    Math::Noise::Detail::MathNoiseManagement::_initFromSeed(unsigned long long(time(0)));
	Engine::setMousePosition(w/2,h/2);
    Game::initResources();
    Game::initLogic();
    //the scene is the root of all games. create the default scene if 1 does not exist already
	if(epriv::Core::m_Engine->m_ResourceManager->_numScenes() == 0)
        new Scene("Default");
}
void RESET_EVENTS(){
    epriv::Core::m_Engine->m_EventManager->_onResetEvents();
}
void update(float dt){
	epriv::Core::m_Engine->m_TimeManager->stop_update();

    Game::onPreUpdate(dt);
    Game::update(dt);
    Resources::getCurrentScene()->update(dt);
	epriv::Core::m_Engine->m_ComponentManager->_update(dt);
    epriv::Core::m_Engine->m_EventManager->_update(dt);
    RESET_EVENTS();
    Game::onPostUpdate(dt);

	epriv::Core::m_Engine->m_TimeManager->calculate_update();

	// update physics //////////////////////////////////////////
	epriv::Core::m_Engine->m_TimeManager->stop_physics();
	epriv::Core::m_Engine->m_PhysicsManager->_update(dt);
	epriv::Core::m_Engine->m_TimeManager->calculate_physics();
	////////////////////////////////////////////////////////////
	// update sounds ///////////////////////////////////////////
	epriv::Core::m_Engine->m_TimeManager->stop_sounds();
	epriv::Core::m_Engine->m_SoundManager->_update(dt);
	epriv::Core::m_Engine->m_TimeManager->calculate_sounds();
	////////////////////////////////////////////////////////////
}
void render(){
	epriv::Core::m_Engine->m_TimeManager->stop_render();

    Game::render();
	glm::uvec2 winSize = Resources::getWindowSize();
	epriv::Core::m_Engine->m_RenderManager->_render(Resources::getCurrentScene()->getActiveCamera(),winSize.x,winSize.y);

	epriv::Core::m_Engine->m_TimeManager->stop_rendering_display();
    Resources::getWindow()->display();
	epriv::Core::m_Engine->m_TimeManager->calculate_rendering_display();

	epriv::Core::m_Engine->m_TimeManager->calculate_render();
}
void EVENT_RESIZE(uint w, uint h,bool saveSize){
	epriv::Core::m_Engine->m_RenderManager->_resize(w,h);

	epriv::Core::m_Engine->m_ResourceManager->_resizeCameras(w,h);
    Renderer::setViewport(0,0,w,h);
    if(saveSize) Engine::Resources::getWindow()->setSize(w,h);
    Game::onResize(w,h);

	epriv::EventWindowResized e;  e.width = w; e.height = h;
	Event ev; ev.eventWindowResized = e; ev.type = EventType::WindowResized;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::WindowResized,ev);
}
void EVENT_CLOSE(){
    Resources::getWindow()->close();
    Game::onClose();

	Event e; e.type = EventType::WindowClosed;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::WindowClosed,e);
}
void EVENT_LOST_FOCUS(){
    Game::onLostFocus();

	Event e; e.type = EventType::WindowLostFocus;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::WindowLostFocus,e);
}
void EVENT_GAINED_FOCUS(){ 
	Game::onGainedFocus();

	Event e; e.type = EventType::WindowGainedFocus;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::WindowGainedFocus,e);
}
void EVENT_TEXT_ENTERED(uint& unicode){ 
	Game::onTextEntered(unicode); 

	epriv::EventTextEntered e;  e.unicode = unicode;
	Event ev;  ev.eventTextEntered = e; ev.type = EventType::TextEntered;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::TextEntered,ev);
}
void EVENT_KEY_PRESSED(uint key){
	epriv::Core::m_Engine->m_EventManager->_onEventKeyPressed(key);
    Game::onKeyPressed(key);

	epriv::EventKeyboard e;  e.key = (KeyboardKey::Key)key;
	if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))  e.control = true;
	if(Engine::isKeyDown(KeyboardKey::LeftAlt) || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
	if(Engine::isKeyDown(KeyboardKey::LeftShift) || Engine::isKeyDown(KeyboardKey::RightShift))      e.shift = true;
	if(Engine::isKeyDown(KeyboardKey::LeftSystem) || Engine::isKeyDown(KeyboardKey::RightSystem))    e.system = true;
	Event ev;  ev.eventKeyboard = e; ev.type = EventType::KeyPressed;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::KeyPressed,ev);
}
void EVENT_KEY_RELEASED(uint key){
    epriv::Core::m_Engine->m_EventManager->_onEventKeyReleased(key);
    Game::onKeyReleased(key);

	epriv::EventKeyboard e;  e.key = (KeyboardKey::Key)key;
	if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))  e.control = true;
	if(Engine::isKeyDown(KeyboardKey::LeftAlt) || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
	if(Engine::isKeyDown(KeyboardKey::LeftShift) || Engine::isKeyDown(KeyboardKey::RightShift))      e.shift = true;
	if(Engine::isKeyDown(KeyboardKey::LeftSystem) || Engine::isKeyDown(KeyboardKey::RightSystem))    e.system = true;
	Event ev;  ev.eventKeyboard = e; ev.type = EventType::KeyReleased;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::KeyReleased,ev);
}
void EVENT_MOUSE_WHEEL_MOVED(int& delta){
    epriv::Core::m_Engine->m_EventManager->_onEventMouseWheelMoved(delta);
    Game::onMouseWheelMoved(delta);

	epriv::EventMouseWheel e;  e.delta = delta;
	Event ev;  ev.eventMouseWheel = e; ev.type = EventType::MouseWheelMoved;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::MouseWheelMoved,ev);
}
void EVENT_MOUSE_BUTTON_PRESSED(uint mouseButton){
	epriv::Core::m_Engine->m_EventManager->_onEventMouseButtonPressed(mouseButton);
    Game::onMouseButtonPressed(mouseButton);

	const glm::uvec2 mpos = Engine::getMousePosition();
	epriv::EventMouseButton e;  e.button = (MouseButton::Button)mouseButton;  e.x = mpos.x;  e.y = mpos.y;
	Event ev; ev.eventMouseButton = e; ev.type = EventType::MouseButtonPressed;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::MouseButtonPressed,ev);
}
void EVENT_MOUSE_BUTTON_RELEASED(uint mouseButton){
	epriv::Core::m_Engine->m_EventManager->_onEventMouseButtonReleased(mouseButton);
    Game::onMouseButtonReleased(mouseButton);

	const glm::uvec2 mpos = Engine::getMousePosition();
	epriv::EventMouseButton e;  e.button = (MouseButton::Button)mouseButton;  e.x = mpos.x;  e.y = mpos.y;
	Event ev; ev.eventMouseButton = e; ev.type = EventType::MouseButtonReleased;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::MouseButtonReleased,ev);
}
void EVENT_MOUSE_MOVED(float mouseX, float mouseY){
    if(Resources::getWindow()->hasFocus()){
        epriv::Core::m_Engine->m_EventManager->_setMousePosition(mouseX,mouseY,false,false);
    }
    Game::onMouseMoved(mouseX,mouseY);

	epriv::EventMouseMove e;  e.x = mouseX;  e.y = mouseY;
	Event ev; ev.eventMouseMoved = e; ev.type = EventType::MouseMoved;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::MouseMoved,ev);
}
void EVENT_MOUSE_ENTERED(){ 
	Game::onMouseEntered(); 

	const glm::uvec2 mpos = Engine::getMousePosition();
	epriv::EventMouseMove e;  e.x = mpos.x;  e.y = mpos.y;
	Event ev; ev.eventMouseMoved = e; ev.type = EventType::MouseEnteredWindow;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::MouseEnteredWindow,ev);
}
void EVENT_MOUSE_LEFT(){ 
	Game::onMouseLeft(); 

	const glm::uvec2 mpos = Engine::getMousePosition();
	epriv::EventMouseMove e;  e.x = mpos.x;  e.y = mpos.y;
	Event ev; ev.eventMouseMoved = e; ev.type = EventType::MouseLeftWindow;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::MouseLeftWindow,ev);
}
void EVENT_JOYSTICK_BUTTON_PRESSED(uint& button, uint& id){ 
	Game::onJoystickButtonPressed();

	epriv::EventJoystickButton e;  e.button = button;  e.joystickID = id;
	Event ev; ev.eventJoystickButton = e; ev.type = EventType::JoystickButtonPressed;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::JoystickButtonPressed,ev);
}
void EVENT_JOYSTICK_BUTTON_RELEASED(uint& button, uint& id){ 
	Game::onJoystickButtonReleased();

	epriv::EventJoystickButton e;  e.button = button;  e.joystickID = id;
	Event ev; ev.eventJoystickButton = e; ev.type = EventType::JoystickButtonReleased;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::JoystickButtonReleased,ev);
}
void EVENT_JOYSTICK_MOVED(uint& id,float& position,uint axis){
	Game::onJoystickMoved();

	epriv::EventJoystickMoved e;  e.axis = (JoystickAxis::Axis)axis;  e.joystickID = id;  e.position = position;
	Event ev; ev.eventJoystickMoved = e; ev.type = EventType::JoystickMoved;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::JoystickMoved,ev);
}
void EVENT_JOYSTICK_CONNECTED(uint& id){ 
	Game::onJoystickConnected(); 

	epriv::EventJoystickConnection e;  e.joystickID = id;
	Event ev; ev.eventJoystickConnection = e; ev.type = EventType::JoystickConnected;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::JoystickConnected,ev);
}
void EVENT_JOYSTICK_DISCONNECTED(uint& id){ 
	Game::onJoystickDisconnected(); 

	epriv::EventJoystickConnection e;  e.joystickID = id;
	Event ev; ev.eventJoystickConnection = e; ev.type = EventType::JoystickDisconnected;
	epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(EventType::JoystickDisconnected,ev);
}

const float Engine::getFPS(){ return 1.0f / Resources::dt(); }
Engine_Window* Engine::getWindow(){ return Engine::Resources::getWindow(); }
const glm::uvec2 Engine::getWindowSize(){ return Engine::Resources::getWindowSize(); }
void Engine::setWindowIcon(Texture* texture){ Resources::getWindow()->setIcon(texture); }
void Engine::showMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(true); }
void Engine::hideMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(false); }
void Engine::stop(){ Resources::getWindow()->close(); }
void Engine::setFullScreen(bool b){ Engine::Resources::getWindow()->setFullScreen(b); }

void handleEvents(){
	sf::Event e;
    while(Resources::getWindow()->getSFMLHandle()->pollEvent(e)){
        switch(e.type){
		    case sf::Event::Closed:{                    EVENT_CLOSE();break;}
			case sf::Event::KeyReleased:{               EVENT_KEY_RELEASED(e.key.code);break;}
			case sf::Event::KeyPressed:{                EVENT_KEY_PRESSED(e.key.code);break;}
			case sf::Event::MouseButtonPressed:{        EVENT_MOUSE_BUTTON_PRESSED(e.mouseButton.button);break;}
			case sf::Event::MouseButtonReleased:{       EVENT_MOUSE_BUTTON_RELEASED(e.mouseButton.button);break;}
			case sf::Event::MouseEntered:{              EVENT_MOUSE_ENTERED();break;}
			case sf::Event::MouseLeft:{                 EVENT_MOUSE_LEFT();break;}
			case sf::Event::MouseWheelMoved:{           EVENT_MOUSE_WHEEL_MOVED(e.mouseWheel.delta);break;}
			case sf::Event::MouseMoved:{                EVENT_MOUSE_MOVED(e.mouseMove.x,e.mouseMove.y);break;}
			case sf::Event::Resized:{                   EVENT_RESIZE(e.size.width,e.size.height,true);break;}
			case sf::Event::TextEntered:{               EVENT_TEXT_ENTERED(e.text.unicode);break;}
			case sf::Event::JoystickButtonPressed:{     EVENT_JOYSTICK_BUTTON_PRESSED(e.joystickButton.button,e.joystickButton.joystickId);break;}
			case sf::Event::JoystickButtonReleased:{    EVENT_JOYSTICK_BUTTON_RELEASED(e.joystickButton.button,e.joystickButton.joystickId);break;}
			case sf::Event::JoystickConnected:{         EVENT_JOYSTICK_CONNECTED(e.joystickConnect.joystickId);break;}
			case sf::Event::JoystickDisconnected:{      EVENT_JOYSTICK_DISCONNECTED(e.joystickConnect.joystickId);break;}
			case sf::Event::JoystickMoved:{             EVENT_JOYSTICK_MOVED(e.joystickMove.joystickId,e.joystickMove.position,e.joystickMove.axis);break;}
			default:{                                   break;}
        }
    }
}

void Engine::run(){
    while(Resources::getWindow()->isOpen()){
		handleEvents();

		float dt = epriv::Core::m_Engine->m_TimeManager->dt();
        update(dt);
        render();
		
		epriv::Core::m_Engine->m_TimeManager->calculate();
    }
	//destruct the engine here
    Game::cleanup();
	SAFE_DELETE(epriv::Core::m_Engine);
}
