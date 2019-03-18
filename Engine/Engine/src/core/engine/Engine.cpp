#include "core/engine/Engine.h"
#include "core/Scene.h"
#include <glm/vec2.hpp>
#include <SFML/System.hpp>
#include <ecs/ECS.h>
#include "ecs/ComponentCamera.h"

#include <time.h>
#include <memory>

using namespace Engine;

#ifdef _WIN32
    #include <windows.h>
    #include <windowsx.h>
#endif
#ifdef _DEBUG
    #include <vld.h> //memory leak printing
#endif


epriv::Core* epriv::Core::m_Engine = nullptr;

epriv::Core::Core(const char* name,uint w,uint h):
m_EventManager(name,w,h),
m_ResourceManager(name,w,h),
m_DebugManager(name,w,h),
m_SoundManager(name,w,h),
m_RenderManager(name,w,h),
m_PhysicsManager(name,w,h),
m_ThreadManager(name,w,h),
m_NoiseManager(name,w,h)
{
    m_Paused = m_Destroyed = false;
}
epriv::Core::~Core(){

}

bool Engine::paused(){ return epriv::Core::m_Engine->m_Paused; }
void Engine::pause(bool b){
    Engine::Physics::pause(b);
    epriv::Core::m_Engine->m_Paused = b;
}
void Engine::unpause(){
    Engine::Physics::unpause();
    epriv::Core::m_Engine->m_Paused = false;
}

void Engine::init(const char* name,uint w,uint h){
    epriv::Core::m_Engine = new epriv::Core(name,w,h);
    auto& engine = *epriv::Core::m_Engine;

    engine.m_ResourceManager._init(name,w,h);
    engine.m_DebugManager._init(name,w,h);
    engine.m_RenderManager._init(name,w,h);
    engine.m_PhysicsManager._init(name,w,h,engine.m_ThreadManager.cores());

    //init the game here
    Engine::setMousePosition(w/2,h/2);
    Game::initResources();
    epriv::threading::waitForAll();
    Game::initLogic();
    //the scene is the root of all games. create the default scene if 1 does not exist already
    if (engine.m_ResourceManager._numScenes() == 0) {
        new Scene("Default");
        if (!Resources::getCurrentScene()) { Resources::setCurrentScene("Default"); }
    }
}
void RESET_EVENTS(){
    epriv::Core::m_Engine->m_EventManager.onResetEvents();
}
void updatePhysics(const float& dt) {
    epriv::Core::m_Engine->m_DebugManager.stop_clock();
    //It's important that timeStep is always less than maxSubSteps * fixedTimeStep, otherwise you are losing time. dt < maxSubSteps * fixedTimeStep
    float minStep = 0.0166666f; // == 0.0166666 at 1 fps
    uint maxSubSteps = 0;
    while (true) {
        ++maxSubSteps; if (dt < (maxSubSteps * minStep)) break;
    }
    epriv::Core::m_Engine->m_PhysicsManager._update(dt, maxSubSteps, minStep);
    epriv::Core::m_Engine->m_DebugManager.calculate_physics();
}
void updateLogic(const float& dt){
    // update logic   //////////////////////////////////////////
    epriv::Core::m_Engine->m_DebugManager.stop_clock();
    Game::onPreUpdate(dt);
    Game::update(dt);

    updatePhysics(dt);
    //update current scene
    Scene& scene = *Resources::getCurrentScene();
    auto& _ecs = epriv::InternalScenePublicInterface::GetECS(scene);
    _ecs.preUpdate(scene, dt);
    scene.update(dt);
    _ecs.update(dt, scene);
    _ecs.postUpdate(scene,dt);


    epriv::Core::m_Engine->m_ThreadManager._update(dt);
    Game::onPostUpdate(dt);

    epriv::Core::m_Engine->m_DebugManager.calculate_logic();

    RESET_EVENTS();

    ////////////////////////////////////////////////////////////
}
void updateSounds(const float& dt){
    // update sounds ///////////////////////////////////////////
    epriv::Core::m_Engine->m_DebugManager.stop_clock();
    epriv::Core::m_Engine->m_SoundManager._update(dt);
    epriv::Core::m_Engine->m_DebugManager.calculate_sounds();
    ////////////////////////////////////////////////////////////
}
void update(const float& dt){
    updateLogic(dt);
    updateSounds(dt);
}

void render(){
    //render
    epriv::Core::m_Engine->m_DebugManager.stop_clock();
    Game::render();
    glm::uvec2 winSize = Resources::getWindowSize();
    epriv::Core::m_Engine->m_RenderManager._render(*Resources::getCurrentScene()->getActiveCamera(),winSize.x,winSize.y);
    epriv::Core::m_Engine->m_DebugManager.calculate_render();

    //display
    epriv::Core::m_Engine->m_DebugManager.stop_clock();
    Resources::getWindow().display();
    epriv::Core::m_Engine->m_DebugManager.calculate_display();
}
void EVENT_RESIZE(uint w, uint h,bool saveSize){
    epriv::Core::m_Engine->m_RenderManager._resize(w,h);

    if(saveSize) Engine::Resources::getWindow().setSize(w,h);
    Game::onResize(w,h);
    //resize cameras here

    for (auto scene : epriv::Core::m_Engine->m_ResourceManager.scenes()) {
        epriv::InternalScenePublicInterface::GetECS(*scene).onResize<ComponentCamera>(w, h);
    }

    epriv::EventWindowResized e;  e.width = w; e.height = h;
    Event ev; ev.eventWindowResized = e; ev.type = EventType::WindowResized;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_CLOSE(){
    Resources::getWindow().close();
    Game::onClose();

    Event e; e.type = EventType::WindowClosed;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(e);
}
void EVENT_LOST_FOCUS(){
    Game::onLostFocus();

    Event e; e.type = EventType::WindowLostFocus;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(e);
}
void EVENT_GAINED_FOCUS(){ 
    Game::onGainedFocus();

    Event e; e.type = EventType::WindowGainedFocus;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(e);
}
void EVENT_TEXT_ENTERED(uint& unicode){ 
    Game::onTextEntered(unicode); 

    epriv::EventTextEntered e;  e.unicode = unicode;
    Event ev;  ev.eventTextEntered = e; ev.type = EventType::TextEntered;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_KEY_PRESSED(uint key){
    epriv::Core::m_Engine->m_EventManager.onEventKeyPressed(key);
    Game::onKeyPressed(key);

    epriv::EventKeyboard e;  e.key = (KeyboardKey::Key)key;
    if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))  e.control = true;
    if(Engine::isKeyDown(KeyboardKey::LeftAlt) || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
    if(Engine::isKeyDown(KeyboardKey::LeftShift) || Engine::isKeyDown(KeyboardKey::RightShift))      e.shift = true;
    if(Engine::isKeyDown(KeyboardKey::LeftSystem) || Engine::isKeyDown(KeyboardKey::RightSystem))    e.system = true;
    Event ev;  ev.eventKeyboard = e; ev.type = EventType::KeyPressed;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_KEY_RELEASED(uint key){
    epriv::Core::m_Engine->m_EventManager.onEventKeyReleased(key);
    Game::onKeyReleased(key);

    epriv::EventKeyboard e;  e.key = (KeyboardKey::Key)key;
    if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))  e.control = true;
    if(Engine::isKeyDown(KeyboardKey::LeftAlt) || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
    if(Engine::isKeyDown(KeyboardKey::LeftShift) || Engine::isKeyDown(KeyboardKey::RightShift))      e.shift = true;
    if(Engine::isKeyDown(KeyboardKey::LeftSystem) || Engine::isKeyDown(KeyboardKey::RightSystem))    e.system = true;
    Event ev;  ev.eventKeyboard = e; ev.type = EventType::KeyReleased;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_MOUSE_WHEEL_MOVED(int& delta){
    epriv::Core::m_Engine->m_EventManager.onEventMouseWheelMoved(delta);
    Game::onMouseWheelMoved(delta);

    epriv::EventMouseWheel e;  e.delta = delta;
    Event ev;  ev.eventMouseWheel = e; ev.type = EventType::MouseWheelMoved;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_MOUSE_BUTTON_PRESSED(uint mouseButton){
    epriv::Core::m_Engine->m_EventManager.onEventMouseButtonPressed(mouseButton);
    Game::onMouseButtonPressed(mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseButton e;  e.button = (MouseButton::Button)mouseButton;  e.x = (float)mpos.x;  e.y = (float)mpos.y;
    Event ev; ev.eventMouseButton = e; ev.type = EventType::MouseButtonPressed;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_MOUSE_BUTTON_RELEASED(uint mouseButton){
    epriv::Core::m_Engine->m_EventManager.onEventMouseButtonReleased(mouseButton);
    Game::onMouseButtonReleased(mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseButton e;  e.button = (MouseButton::Button)mouseButton;  e.x = (float)mpos.x;  e.y = (float)mpos.y;
    Event ev; ev.eventMouseButton = e; ev.type = EventType::MouseButtonReleased;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_MOUSE_MOVED(int mouseX, int mouseY){
    float mX = (float)mouseX; float mY = (float)mouseY;
    if(Resources::getWindow().hasFocus()){
        epriv::Core::m_Engine->m_EventManager.setMousePositionInternal(mX,mY,false,false);
    }
    Game::onMouseMoved(mX,mY);

    epriv::EventMouseMove e;  e.x = mX;  e.y = mY;
    Event ev; ev.eventMouseMoved = e; ev.type = EventType::MouseMoved;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_MOUSE_ENTERED(){ 
    Game::onMouseEntered(); 

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseMove e;  e.x = (float)mpos.x;  e.y = (float)mpos.y;
    Event ev; ev.eventMouseMoved = e; ev.type = EventType::MouseEnteredWindow;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_MOUSE_LEFT(){ 
    Game::onMouseLeft(); 

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseMove e;  e.x = (float)mpos.x;  e.y = (float)mpos.y;
    Event ev; ev.eventMouseMoved = e; ev.type = EventType::MouseLeftWindow;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_JOYSTICK_BUTTON_PRESSED(uint& button, uint& id){ 
    Game::onJoystickButtonPressed();

    epriv::EventJoystickButton e;  e.button = button;  e.joystickID = id;
    Event ev; ev.eventJoystickButton = e; ev.type = EventType::JoystickButtonPressed;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_JOYSTICK_BUTTON_RELEASED(uint& button, uint& id){ 
    Game::onJoystickButtonReleased();

    epriv::EventJoystickButton e;  e.button = button;  e.joystickID = id;
    Event ev; ev.eventJoystickButton = e; ev.type = EventType::JoystickButtonReleased;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_JOYSTICK_MOVED(uint& id,float& position,uint axis){
    Game::onJoystickMoved();

    epriv::EventJoystickMoved e;  e.axis = (JoystickAxis::Axis)axis;  e.joystickID = id;  e.position = position;
    Event ev; ev.eventJoystickMoved = e; ev.type = EventType::JoystickMoved;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_JOYSTICK_CONNECTED(uint& id){ 
    Game::onJoystickConnected(); 

    epriv::EventJoystickConnection e;  e.joystickID = id;
    Event ev; ev.eventJoystickConnection = e; ev.type = EventType::JoystickConnected;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}
void EVENT_JOYSTICK_DISCONNECTED(uint& id){ 
    Game::onJoystickDisconnected(); 

    epriv::EventJoystickConnection e;  e.joystickID = id;
    Event ev; ev.eventJoystickConnection = e; ev.type = EventType::JoystickDisconnected;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher._dispatchEvent(ev);
}

const float Engine::getFPS(){ return (float)(1.0 / Resources::dt()); }
Engine_Window& Engine::getWindow(){ return Engine::Resources::getWindow(); }
const glm::uvec2 Engine::getWindowSize(){ return Engine::Resources::getWindowSize(); }
void Engine::setWindowIcon(const Texture& texture){ Resources::getWindow().setIcon(texture); }
void Engine::showMouseCursor(){ Resources::getWindow().setMouseCursorVisible(true); }
void Engine::hideMouseCursor(){ Resources::getWindow().setMouseCursorVisible(false); }
void Engine::stop(){ epriv::Core::m_Engine->m_Destroyed = true; }
void Engine::setFullScreen(bool b){ Engine::Resources::getWindow().setFullScreen(b); }

void handleEvents(){
    sf::Event e;
    while(Resources::getWindow().getSFMLHandle().pollEvent(e)){
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
    while(!epriv::Core::m_Engine->m_Destroyed /*&& Resources::getWindow().isOpen()*/){
        float dt = (float)epriv::Core::m_Engine->m_DebugManager.dt();
        handleEvents();
        update(dt);
        render();
        epriv::Core::m_Engine->m_DebugManager.calculate();
    }
    //destruct the engine here
    Game::cleanup();
    SAFE_DELETE(epriv::Core::m_Engine);
}


int main(int argc, char* argv[]) {
    srand((unsigned)time(0));
#ifdef _WIN32
    if (GetConsoleWindow() == NULL) { AllocConsole(); }
    freopen("CONIN$", "r", stdin); freopen("CONOUT$", "w", stdout); freopen("CONOUT$", "w", stderr);
#ifndef _DEBUG
    //ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window if in release mode
#endif
#endif
    Engine::init("Engine", 1024, 768);
    Engine::run();

#ifdef _WIN32
    FreeConsole();
#endif
    return 0;
}