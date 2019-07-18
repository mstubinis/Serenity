#include <core/engine/Engine.h>
#include <core/Scene.h>
#include <glm/vec2.hpp>
#include <SFML/System.hpp>
#include <ecs/ECS.h>
#include <ecs/ComponentCamera.h>

#include <core/Camera.h>

using namespace Engine;

epriv::Core* epriv::Core::m_Engine = nullptr;

epriv::Core::Core(const EngineOptions& options) :
m_EventManager(options.window_title, options.width, options.height),
m_ResourceManager(options.window_title, options.width, options.height),
m_DebugManager(options.window_title, options.width, options.height),
m_SoundManager(options.window_title, options.width, options.height),
m_RenderManager(options.window_title, options.width, options.height),
m_PhysicsManager(options.window_title, options.width, options.height),
m_ThreadManager(options.window_title, options.width, options.height),
m_NoiseManager(options.window_title, options.width, options.height)
{
    m_Destroyed = m_Paused = false;
}

epriv::Core::~Core(){

}

bool Engine::paused(){ 
    return epriv::Core::m_Engine->m_Paused; 
}
void Engine::pause(const bool& b){
    Engine::Physics::pause(b);
    epriv::Core::m_Engine->m_Paused = b;
}
void Engine::unpause(){
    Engine::Physics::unpause();
    epriv::Core::m_Engine->m_Paused = false;
}

void Engine::init(const EngineOptions& options) {
    epriv::Core::m_Engine = new epriv::Core(options);
    auto& engine = *epriv::Core::m_Engine;

    engine.m_ResourceManager._init(options.window_title, options.width, options.height);

    auto& window = Resources::getWindow();
    window.setSize(options.width, options.height);
    window.setFullScreen(options.fullscreen);

    engine.m_DebugManager._init(options.window_title, options.width, options.height);
    engine.m_RenderManager._init(options.window_title, options.width, options.height);
    engine.m_PhysicsManager._init(options.window_title, options.width, options.height, engine.m_ThreadManager.cores());

    //init the game here
    Engine::setMousePosition(options.width / 2, options.height / 2);
    Game::initResources();
    epriv::threading::waitForAll();
    Game::initLogic();

    //the scene is the root of all games. create the default scene if 1 does not exist already
    if (engine.m_ResourceManager._numScenes() == 0) {
        Scene* defaultScene = new Scene("Default");
        Resources::setCurrentScene(defaultScene);
    }
    Scene& currentScene = *Resources::getCurrentScene();
    if (!currentScene.getActiveCamera()) {
        Camera* defaultCamera = new Camera(60, options.width / static_cast<float>(options.height), 0.01f, 1000.0f, &currentScene);
        currentScene.setActiveCamera(*defaultCamera);
    }

    Renderer::ssao::enable(options.ssao_enabled);
    Renderer::godRays::enable(options.god_rays_enabled);
    Renderer::hdr::enable(options.hdr_enabled);
    Renderer::fog::enable(options.fog_enabled);
    Renderer::Settings::setAntiAliasingAlgorithm(options.aa_algorithm);
}

void RESET_EVENTS(const double& dt){
    epriv::Core::m_Engine->m_EventManager.onResetEvents(dt);
}

const float PHYSICS_MIN_STEP = 0.016666666666666666f; // == 0.016666666666666666 at 1 fps
void updatePhysics(const double& dt) {
    auto& debugMgr = epriv::Core::m_Engine->m_DebugManager;
    debugMgr.stop_clock();
    //It's important that timeStep is always less than maxSubSteps * fixedTimeStep, otherwise you are losing time. dt < maxSubSteps * fixedTimeStep
    uint maxSubSteps = 0;
    while (true) {
        ++maxSubSteps;
        if (dt < static_cast<double>(maxSubSteps * PHYSICS_MIN_STEP))
            break;
    }
    epriv::Core::m_Engine->m_PhysicsManager._update(dt, maxSubSteps, PHYSICS_MIN_STEP);
    debugMgr.calculate_physics();
}
void updateLogic(const double& dt){
    auto& debugMgr = epriv::Core::m_Engine->m_DebugManager;
    // update logic   //////////////////////////////////////////
    debugMgr.stop_clock();
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

    debugMgr.calculate_logic();

    RESET_EVENTS(dt);
    ////////////////////////////////////////////////////////////
}
void updateSounds(const double& dt){
    // update sounds ///////////////////////////////////////////
    auto& debugMgr = epriv::Core::m_Engine->m_DebugManager;
    debugMgr.stop_clock();
    epriv::Core::m_Engine->m_SoundManager._update(dt);
    debugMgr.calculate_sounds();
    ////////////////////////////////////////////////////////////
}
void update(const double& dt){
    updateLogic(dt);
    updateSounds(dt);
}

void render(){
    //render
    auto& debugMgr = epriv::Core::m_Engine->m_DebugManager;
    debugMgr.stop_clock();
    Game::render();
    const auto& winSize = Resources::getWindowSize();
    epriv::Core::m_Engine->m_RenderManager._render(
        *Resources::getCurrentScene()->getActiveCamera(),
        winSize.x,
        winSize.y,
        true,
        nullptr,
        true,
        0,
        0
    );
    debugMgr.calculate_render();

    //display
    debugMgr.stop_clock();
    Resources::getWindow().display();
    debugMgr.calculate_display();
}
void EVENT_RESIZE(const uint& w, const uint& h, const bool& saveSize){
    epriv::Core::m_Engine->m_RenderManager._resize(w,h);

    if(saveSize) Engine::Resources::getWindow().setSize(w,h);
    Game::onResize(w,h);
    //resize cameras here

    for (auto& scene : epriv::Core::m_Engine->m_ResourceManager.scenes()) {
        epriv::InternalScenePublicInterface::GetECS(*scene).onResize<ComponentCamera>(w, h);
    }

    epriv::EventWindowResized e(w,h);
    Event ev;
    ev.eventWindowResized = e;
    ev.type = EventType::WindowResized;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_CLOSE(){
    Resources::getWindow().close();
    Game::onClose();

    Event ev;
    ev.type = EventType::WindowClosed;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_LOST_FOCUS(){
    Game::onLostFocus();

    Event ev;
    ev.type = EventType::WindowLostFocus;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_GAINED_FOCUS(){ 
    Game::onGainedFocus();

    Event ev;
    ev.type = EventType::WindowGainedFocus;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_TEXT_ENTERED(const uint& unicode){
    Game::onTextEntered(unicode); 

    epriv::EventTextEntered e(unicode);
    Event ev;
    ev.eventTextEntered = e;
    ev.type = EventType::TextEntered;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_KEY_PRESSED(const uint& key){
    epriv::Core::m_Engine->m_EventManager.onEventKeyPressed(key);
    Game::onKeyPressed(key);

    epriv::EventKeyboard e;
    e.key = (KeyboardKey::Key)key;
    if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))  e.control = true;
    if(Engine::isKeyDown(KeyboardKey::LeftAlt) || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
    if(Engine::isKeyDown(KeyboardKey::LeftShift) || Engine::isKeyDown(KeyboardKey::RightShift))      e.shift = true;
    if(Engine::isKeyDown(KeyboardKey::LeftSystem) || Engine::isKeyDown(KeyboardKey::RightSystem))    e.system = true;

    Event ev;
    ev.eventKeyboard = e;
    ev.type = EventType::KeyPressed;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_KEY_RELEASED(const uint& key){
    epriv::Core::m_Engine->m_EventManager.onEventKeyReleased(key);
    Game::onKeyReleased(key);

    epriv::EventKeyboard e;
    e.key = (KeyboardKey::Key)key;
    if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))  e.control = true;
    if(Engine::isKeyDown(KeyboardKey::LeftAlt) || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
    if(Engine::isKeyDown(KeyboardKey::LeftShift) || Engine::isKeyDown(KeyboardKey::RightShift))      e.shift = true;
    if(Engine::isKeyDown(KeyboardKey::LeftSystem) || Engine::isKeyDown(KeyboardKey::RightSystem))    e.system = true;
    Event ev;
    ev.eventKeyboard = e;
    ev.type = EventType::KeyReleased;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_WHEEL_MOVED(const int& delta){
    epriv::Core::m_Engine->m_EventManager.onEventMouseWheelMoved(delta);
    Game::onMouseWheelMoved(delta);

    epriv::EventMouseWheel e(delta);
    Event ev;
    ev.eventMouseWheel = e;
    ev.type = EventType::MouseWheelMoved;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_BUTTON_PRESSED(const uint& mouseButton){
    epriv::Core::m_Engine->m_EventManager.onEventMouseButtonPressed(mouseButton);
    Game::onMouseButtonPressed(mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseButton e((MouseButton::Button)mouseButton, (float)mpos.x, (float)mpos.y);
    Event ev;
    ev.eventMouseButton = e;
    ev.type = EventType::MouseButtonPressed;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_BUTTON_RELEASED(const uint& mouseButton){
    epriv::Core::m_Engine->m_EventManager.onEventMouseButtonReleased(mouseButton);
    Game::onMouseButtonReleased(mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseButton e((MouseButton::Button)mouseButton, (float)mpos.x, (float)mpos.y);
    Event ev;
    ev.eventMouseButton = e;
    ev.type = EventType::MouseButtonReleased;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_MOVED(const int& mouseX, const int& mouseY){
    const float& mX = static_cast<float>(mouseX);
    const float& mY = static_cast<float>(mouseY);
    if(Resources::getWindow().hasFocus()){
        epriv::Core::m_Engine->m_EventManager.setMousePositionInternal(mX,mY,false,false);
    }
    Game::onMouseMoved(mX,mY);

    epriv::EventMouseMove e(mX,mY);
    Event ev;
    ev.eventMouseMoved = e;
    ev.type = EventType::MouseMoved;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_ENTERED(){ 
    Game::onMouseEntered(); 

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseMove e(
        static_cast<float>(mpos.x),
        static_cast<float>(mpos.y)
    );
    Event ev;
    ev.eventMouseMoved = e;
    ev.type = EventType::MouseEnteredWindow;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_LEFT(){ 
    Game::onMouseLeft(); 

    const glm::uvec2 mpos = Engine::getMousePosition();
    epriv::EventMouseMove e(
        static_cast<float>(mpos.x),
        static_cast<float>(mpos.y)
    );
    Event ev;
    ev.eventMouseMoved = e;
    ev.type = EventType::MouseLeftWindow;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_JOYSTICK_BUTTON_PRESSED(const uint& button, const uint& id){
    Game::onJoystickButtonPressed();

    epriv::EventJoystickButton e(id,button);
    Event ev;
    ev.eventJoystickButton = e;
    ev.type = EventType::JoystickButtonPressed;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_JOYSTICK_BUTTON_RELEASED(const uint& button, const uint& id){
    Game::onJoystickButtonReleased();

    epriv::EventJoystickButton e(id,button);
    Event ev;
    ev.eventJoystickButton = e;
    ev.type = EventType::JoystickButtonReleased;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_JOYSTICK_MOVED(const uint& id, const float& position, const uint axis){
    Game::onJoystickMoved();

    epriv::EventJoystickMoved e(id, (JoystickAxis::Axis)axis,position);
    Event ev;
    ev.eventJoystickMoved = e;
    ev.type = EventType::JoystickMoved;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_JOYSTICK_CONNECTED(const uint& id){
    Game::onJoystickConnected(); 

    epriv::EventJoystickConnection e(id);
    Event ev;
    ev.eventJoystickConnection = e;
    ev.type = EventType::JoystickConnected;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_JOYSTICK_DISCONNECTED(const uint& id){
    Game::onJoystickDisconnected(); 

    epriv::EventJoystickConnection e(id);
    Event ev;
    ev.eventJoystickConnection = e;
    ev.type = EventType::JoystickDisconnected;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}

const float Engine::getFPS(){ 
    return static_cast<float>(1.0 / Resources::dt()); 
}
Engine_Window& Engine::getWindow(){ 
    return Engine::Resources::getWindow(); 
}
const glm::uvec2 Engine::getWindowSize(){ 
    return Engine::Resources::getWindowSize(); 
}
void Engine::setWindowIcon(const Texture& texture){ 
    Resources::getWindow().setIcon(texture); 
}
void Engine::showMouseCursor(){ 
    Resources::getWindow().setMouseCursorVisible(true); 
}
void Engine::hideMouseCursor(){ 
    Resources::getWindow().setMouseCursorVisible(false); 
}
void Engine::stop(){ 
    epriv::Core::m_Engine->m_Destroyed = true; 
}
void Engine::setFullScreen(const bool& b){ 
    Engine::Resources::getWindow().setFullScreen(b); 
}

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
        auto& debugMgr = epriv::Core::m_Engine->m_DebugManager;
        double dt = debugMgr.dt();
        handleEvents();
        update(dt);
        render();
        debugMgr.calculate();
    }
    //destruct the engine here
    Game::cleanup();
    SAFE_DELETE(epriv::Core::m_Engine);
}