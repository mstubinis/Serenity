#include "Engine.h"
#include "Engine_Time.h"
#include "Engine_EventDispatcher.h"
#include "Engine_ThreadManager.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Engine_Sounds.h"
#include "Engine_Events.h"
#include "Engine_Noise.h"
#include "Engine_Window.h"

#include "Components.h"
#include "Scene.h"

#include <glm/vec2.hpp>

#include <SFML/System.hpp>

using namespace Engine;

epriv::Core* epriv::Core::m_Engine = nullptr;
epriv::Core::Core(const char* name,uint w,uint h){
    m_Paused = false;
    m_Destroyed = false;
}
epriv::Core::~Core(){
    SAFE_DELETE(m_TimeManager);
    SAFE_DELETE(m_EventDispatcher);
    SAFE_DELETE(m_NoiseManager);
    SAFE_DELETE(m_ComponentManager);
    SAFE_DELETE(m_ThreadManager);
    SAFE_DELETE(m_EventManager);
    SAFE_DELETE(m_SoundManager);
    SAFE_DELETE(m_PhysicsManager);
    SAFE_DELETE(m_RenderManager);
    SAFE_DELETE(m_ResourceManager);
}

bool Engine::paused(){ return epriv::Core::m_Engine->m_Paused; }
void Engine::pause(bool b){
    epriv::Core::m_Engine->m_ComponentManager->_pause(b);
    Engine::Physics::pause(b);
    epriv::Core::m_Engine->m_Paused = b;
}
void Engine::unpause(){
    epriv::Core::m_Engine->m_ComponentManager->_unpause();
    Engine::Physics::unpause();
    epriv::Core::m_Engine->m_Paused = false;
}

void Engine::init(const char* name,uint w,uint h){
    epriv::Core::m_Engine = new epriv::Core(name,w,h);
	auto& engine = *epriv::Core::m_Engine;

	engine.m_EventManager      = new epriv::EventManager(name, w, h);
	engine.m_EventDispatcher   = new epriv::EventDispatcher(name, w, h);
	engine.m_ResourceManager   = new epriv::ResourceManager(name, w, h);
	engine.m_TimeManager       = new epriv::TimeManager(name, w, h);
	engine.m_SoundManager      = new epriv::SoundManager(name, w, h);
	engine.m_RenderManager     = new epriv::RenderManager(name, w, h);
	engine.m_PhysicsManager    = new epriv::PhysicsManager(name, w, h);
	engine.m_ComponentManager  = new epriv::ComponentManager(name, w, h);
	engine.m_ThreadManager     = new epriv::ThreadManager(name, w, h);
	engine.m_NoiseManager      = new epriv::NoiseManager(name, w, h);

	engine.m_ResourceManager->_init(name,w,h);
	engine.m_TimeManager->_init(name,w,h);
	engine.m_RenderManager->_init(name,w,h);
	engine.m_PhysicsManager->_init(name,w,h,engine.m_ThreadManager->cores());

    //init the game here
    Engine::setMousePosition(w/2,h/2);
    Game::initResources();
    epriv::threading::waitForAll();
    Game::initLogic();
    //the scene is the root of all games. create the default scene if 1 does not exist already
    if(engine.m_ResourceManager->_numScenes() == 0)
        new Scene("Default");
}
void RESET_EVENTS(){
    epriv::Core::m_Engine->m_EventManager->_onResetEvents();
}
void updateLogic(float dt){
    // update logic   //////////////////////////////////////////
    epriv::Core::m_Engine->m_TimeManager->stop_clock();
    Game::onPreUpdate(dt);
    Game::update(dt);
    Resources::getCurrentScene()->update(dt);
    epriv::Core::m_Engine->m_ComponentManager->_update(dt);
    epriv::Core::m_Engine->m_ThreadManager->_update(dt);
    epriv::Core::m_Engine->m_EventManager->_update(dt);
    RESET_EVENTS();
    Game::onPostUpdate(dt);

    epriv::Core::m_Engine->m_TimeManager->calculate_logic();
    ////////////////////////////////////////////////////////////
}
void updateSounds(float dt){
    // update sounds ///////////////////////////////////////////
    epriv::Core::m_Engine->m_TimeManager->stop_clock();
    epriv::Core::m_Engine->m_SoundManager->_update(dt);
    epriv::Core::m_Engine->m_TimeManager->calculate_sounds();
    ////////////////////////////////////////////////////////////
}
void update(const float& dt){
    updateLogic(dt);
    updateSounds(dt);
}

void render(){
    //render
    epriv::Core::m_Engine->m_TimeManager->stop_clock();
    Game::render();
    glm::uvec2 winSize = Resources::getWindowSize();
    epriv::Core::m_Engine->m_RenderManager->_render(Resources::getCurrentScene()->getActiveCamera(),winSize.x,winSize.y);
    epriv::Core::m_Engine->m_TimeManager->calculate_render();

    //display
    epriv::Core::m_Engine->m_TimeManager->stop_clock();
    Resources::getWindow()->display();
    epriv::Core::m_Engine->m_TimeManager->calculate_display();	
}
void EVENT_RESIZE(uint w, uint h,bool saveSize){
    epriv::Core::m_Engine->m_RenderManager->_resize(w,h);

    epriv::Core::m_Engine->m_ComponentManager->_resize(w,h);
    if(saveSize) Engine::Resources::getWindow()->setSize(w,h);
    Game::onResize(w,h);

    epriv::EventWindowResized e;  e.width = w; e.height = h;
    Event ev; ev.eventWindowResized = e; ev.type = EventType::WindowResized;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_CLOSE(){
    Resources::getWindow()->close();
    Game::onClose();

    Event e; e.type = EventType::WindowClosed;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(e);
}
void EVENT_LOST_FOCUS(){
    Game::onLostFocus();

    Event e; e.type = EventType::WindowLostFocus;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(e);
}
void EVENT_GAINED_FOCUS(){ 
    Game::onGainedFocus();

    Event e; e.type = EventType::WindowGainedFocus;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(e);
}
void EVENT_TEXT_ENTERED(uint& unicode){ 
    Game::onTextEntered(unicode); 

    epriv::EventTextEntered e;  e.unicode = unicode;
    Event ev;  ev.eventTextEntered = e; ev.type = EventType::TextEntered;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
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
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
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
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_MOUSE_WHEEL_MOVED(int& delta){
    epriv::Core::m_Engine->m_EventManager->_onEventMouseWheelMoved(delta);
    Game::onMouseWheelMoved(delta);

    epriv::EventMouseWheel e;  e.delta = delta;
    Event ev;  ev.eventMouseWheel = e; ev.type = EventType::MouseWheelMoved;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_MOUSE_BUTTON_PRESSED(uint mouseButton){
    epriv::Core::m_Engine->m_EventManager->_onEventMouseButtonPressed(mouseButton);
    Game::onMouseButtonPressed(mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseButton e;  e.button = (MouseButton::Button)mouseButton;  e.x = (float)mpos.x;  e.y = (float)mpos.y;
    Event ev; ev.eventMouseButton = e; ev.type = EventType::MouseButtonPressed;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_MOUSE_BUTTON_RELEASED(uint mouseButton){
    epriv::Core::m_Engine->m_EventManager->_onEventMouseButtonReleased(mouseButton);
    Game::onMouseButtonReleased(mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseButton e;  e.button = (MouseButton::Button)mouseButton;  e.x = (float)mpos.x;  e.y = (float)mpos.y;
    Event ev; ev.eventMouseButton = e; ev.type = EventType::MouseButtonReleased;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_MOUSE_MOVED(int mouseX, int mouseY){
    float mX = (float)mouseX; float mY = (float)mouseY;
    if(Resources::getWindow()->hasFocus()){
        epriv::Core::m_Engine->m_EventManager->_setMousePosition(mX,mY,false,false);
    }
    Game::onMouseMoved(mX,mY);

    epriv::EventMouseMove e;  e.x = mX;  e.y = mY;
    Event ev; ev.eventMouseMoved = e; ev.type = EventType::MouseMoved;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_MOUSE_ENTERED(){ 
    Game::onMouseEntered(); 

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseMove e;  e.x = (float)mpos.x;  e.y = (float)mpos.y;
    Event ev; ev.eventMouseMoved = e; ev.type = EventType::MouseEnteredWindow;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_MOUSE_LEFT(){ 
    Game::onMouseLeft(); 

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseMove e;  e.x = (float)mpos.x;  e.y = (float)mpos.y;
    Event ev; ev.eventMouseMoved = e; ev.type = EventType::MouseLeftWindow;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_JOYSTICK_BUTTON_PRESSED(uint& button, uint& id){ 
    Game::onJoystickButtonPressed();

    epriv::EventJoystickButton e;  e.button = button;  e.joystickID = id;
    Event ev; ev.eventJoystickButton = e; ev.type = EventType::JoystickButtonPressed;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_JOYSTICK_BUTTON_RELEASED(uint& button, uint& id){ 
    Game::onJoystickButtonReleased();

    epriv::EventJoystickButton e;  e.button = button;  e.joystickID = id;
    Event ev; ev.eventJoystickButton = e; ev.type = EventType::JoystickButtonReleased;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_JOYSTICK_MOVED(uint& id,float& position,uint axis){
    Game::onJoystickMoved();

    epriv::EventJoystickMoved e;  e.axis = (JoystickAxis::Axis)axis;  e.joystickID = id;  e.position = position;
    Event ev; ev.eventJoystickMoved = e; ev.type = EventType::JoystickMoved;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_JOYSTICK_CONNECTED(uint& id){ 
    Game::onJoystickConnected(); 

    epriv::EventJoystickConnection e;  e.joystickID = id;
    Event ev; ev.eventJoystickConnection = e; ev.type = EventType::JoystickConnected;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}
void EVENT_JOYSTICK_DISCONNECTED(uint& id){ 
    Game::onJoystickDisconnected(); 

    epriv::EventJoystickConnection e;  e.joystickID = id;
    Event ev; ev.eventJoystickConnection = e; ev.type = EventType::JoystickDisconnected;
    epriv::Core::m_Engine->m_EventDispatcher->_dispatchEvent(ev);
}

const float Engine::getFPS(){ return 1.0f / Resources::dt(); }
Engine_Window* Engine::getWindow(){ return Engine::Resources::getWindow(); }
const glm::uvec2 Engine::getWindowSize(){ return Engine::Resources::getWindowSize(); }
void Engine::setWindowIcon(Texture* texture){ Resources::getWindow()->setIcon(texture); }
void Engine::showMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(true); }
void Engine::hideMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(false); }
void Engine::stop(){ epriv::Core::m_Engine->m_Destroyed = true; }
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
    const Engine_Window& window = *Resources::getWindow();
    while(!epriv::Core::m_Engine->m_Destroyed /*&& window.isOpen()*/){
        float& dt = epriv::Core::m_Engine->m_TimeManager->dt();
        handleEvents();
        update(dt);
        render();
        epriv::Core::m_Engine->m_TimeManager->calculate();
    }
    //destruct the engine here
    Game::cleanup();
    SAFE_DELETE(epriv::Core::m_Engine);
}
