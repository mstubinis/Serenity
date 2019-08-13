#include <core/engine/Engine.h>
#include <ecs/ECS.h>
#include <ecs/ComponentCamera.h>

#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Viewport.h>

#include <glm/vec2.hpp>
#include <SFML/System.hpp>

using namespace Engine;
using namespace Engine::epriv;

EngineCore* Core::m_Engine = nullptr;

EngineCore::EngineCore(const EngineOptions& options) :
m_EventManager(options.window_title, options.width, options.height),
m_ResourceManager(options.window_title, options.width, options.height),
m_DebugManager(options.window_title, options.width, options.height),
m_SoundManager(options.window_title, options.width, options.height),
m_RenderManager(options.window_title, options.width, options.height),
m_PhysicsManager(options.window_title, options.width, options.height),
m_ThreadManager(options.window_title, options.width, options.height)
{
    m_Misc.m_Destroyed = m_Misc.m_Paused = false;
}

EngineCore::~EngineCore(){

}

bool Engine::paused(){ 
    return Core::m_Engine->m_Misc.m_Paused;
}
void Engine::pause(const bool& b){
    Engine::Physics::pause(b);
    Core::m_Engine->m_Misc.m_Paused = b;
}
void Engine::unpause(){
    Engine::Physics::unpause();
    Core::m_Engine->m_Misc.m_Paused = false;
}

void Engine::init(const EngineOptions& options) {
    Core::m_Engine = new EngineCore(options);
    auto& engine = *Core::m_Engine;

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
        Camera* default_camera = new Camera(60, options.width / static_cast<float>(options.height), 0.01f, 1000.0f, &currentScene);
        currentScene.setActiveCamera(*default_camera);
    }

    Renderer::ssao::enable(options.ssao_enabled);
    Renderer::godRays::enable(options.god_rays_enabled);
    Renderer::hdr::enable(options.hdr_enabled);
    Renderer::fog::enable(options.fog_enabled);
    Renderer::Settings::setAntiAliasingAlgorithm(options.aa_algorithm);
}

void RESET_EVENTS(const double& dt){
    Core::m_Engine->m_EventManager.onResetEvents(dt);
}

const float PHYSICS_MIN_STEP = 0.016666666666666666f; // == 0.016666666666666666 at 1 fps
void updatePhysics(const double& dt) {
    auto& debugMgr = Core::m_Engine->m_DebugManager;
    debugMgr.stop_clock();
    //It's important that timeStep is always less than maxSubSteps * fixedTimeStep, otherwise you are losing time. dt < maxSubSteps * fixedTimeStep
    uint maxSubSteps = 0;
    while (true) {
        ++maxSubSteps;
        if (dt < static_cast<double>(maxSubSteps * PHYSICS_MIN_STEP))
            break;
    }
    Core::m_Engine->m_PhysicsManager._update(dt, maxSubSteps, PHYSICS_MIN_STEP);
    debugMgr.calculate_physics();
}
void updateLogic(const double& dt){
    auto& debugMgr = Core::m_Engine->m_DebugManager;
    // update logic   //////////////////////////////////////////
    debugMgr.stop_clock();
    Game::onPreUpdate(dt);
    Game::update(dt);
    //update current scene
    Scene& scene = *Resources::getCurrentScene();
    auto& _ecs = InternalScenePublicInterface::GetECS(scene);
    _ecs.preUpdate(scene, dt);
    scene.update(dt);
    updatePhysics(dt);
    _ecs.update(dt, scene);
    _ecs.postUpdate(scene,dt);

    Core::m_Engine->m_ThreadManager._update(dt);
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

void render(const double& dt){
    auto& debugMgr = Core::m_Engine->m_DebugManager;
    debugMgr.stop_clock();
    Game::render();
    auto& viewports = InternalScenePublicInterface::GetViewports(*Resources::getCurrentScene());
    for (auto& viewport : viewports) {
        if (viewport->isActive()) {
            Core::m_Engine->m_RenderManager._render(dt, *viewport, true, 0, 0);
        }
    }
    Resources::getWindow().display();
    Core::m_Engine->m_RenderManager._clear2DAPICommands();
    debugMgr.calculate_render();
}
void EVENT_RESIZE(const uint& w, const uint& h, const bool& saveSize){
    Core::m_Engine->m_RenderManager._resize(w,h);

    if(saveSize) 
        Resources::getWindow().setSize(w,h);
    Game::onResize(w,h);
    //resize cameras here

    for (auto& scene : Core::m_Engine->m_ResourceManager.scenes()) {
        InternalScenePublicInterface::GetECS(*scene).onResize<ComponentCamera>(w, h);
        InternalScenePublicInterface::GetViewports(*scene)[0]->setViewportDimensions(0, 0, w, h);
    }

    EventWindowResized e(w,h);
    Event ev;
    ev.eventWindowResized = e;
    ev.type = EventType::WindowResized;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_CLOSE(){
    Resources::getWindow().close();
    Game::onClose();

    Event ev;
    ev.type = EventType::WindowClosed;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_LOST_FOCUS(){
    Game::onLostFocus();

    Event ev;
    ev.type = EventType::WindowLostFocus;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_GAINED_FOCUS(){ 
    Game::onGainedFocus();

    Event ev;
    ev.type = EventType::WindowGainedFocus;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_TEXT_ENTERED(const uint& unicode){
    Game::onTextEntered(unicode); 

    EventTextEntered e(unicode);
    Event ev;
    ev.eventTextEntered = e;
    ev.type = EventType::TextEntered;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_KEY_PRESSED(const uint& key){
    Core::m_Engine->m_EventManager.onEventKeyPressed(key);
    Game::onKeyPressed(key);

    epriv::EventKeyboard e;
    e.key = static_cast<KeyboardKey::Key>(key);
    if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))      e.control = true;
    if(Engine::isKeyDown(KeyboardKey::LeftAlt)     || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
    if(Engine::isKeyDown(KeyboardKey::LeftShift)   || Engine::isKeyDown(KeyboardKey::RightShift))        e.shift = true;
    if(Engine::isKeyDown(KeyboardKey::LeftSystem)  || Engine::isKeyDown(KeyboardKey::RightSystem))       e.system = true;

    Event ev;
    ev.eventKeyboard = e;
    ev.type = EventType::KeyPressed;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_KEY_RELEASED(const uint& key){
    Core::m_Engine->m_EventManager.onEventKeyReleased(key);
    Game::onKeyReleased(key);

    epriv::EventKeyboard e;
    e.key = static_cast<KeyboardKey::Key>(key);
    if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))      e.control = true;
    if(Engine::isKeyDown(KeyboardKey::LeftAlt)     || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
    if(Engine::isKeyDown(KeyboardKey::LeftShift)   || Engine::isKeyDown(KeyboardKey::RightShift))        e.shift = true;
    if(Engine::isKeyDown(KeyboardKey::LeftSystem)  || Engine::isKeyDown(KeyboardKey::RightSystem))       e.system = true;
    Event ev;
    ev.eventKeyboard = e;
    ev.type = EventType::KeyReleased;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_WHEEL_MOVED(const int& delta){
    Core::m_Engine->m_EventManager.onEventMouseWheelMoved(delta);
    Game::onMouseWheelMoved(delta);

    epriv::EventMouseWheel e(delta);
    Event ev;
    ev.eventMouseWheel = e;
    ev.type = EventType::MouseWheelMoved;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_BUTTON_PRESSED(const uint& mouseButton){
    Core::m_Engine->m_EventManager.onEventMouseButtonPressed(mouseButton);
    Game::onMouseButtonPressed(mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition();
    EventMouseButton e(static_cast<MouseButton::Button>(mouseButton), static_cast<float>(mpos.x), static_cast<float>(mpos.y));
    Event ev;
    ev.eventMouseButton = e;
    ev.type = EventType::MouseButtonPressed;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_BUTTON_RELEASED(const uint& mouseButton){
    Core::m_Engine->m_EventManager.onEventMouseButtonReleased(mouseButton);
    Game::onMouseButtonReleased(mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition();
    EventMouseButton e(static_cast<MouseButton::Button>(mouseButton), static_cast<float>(mpos.x), static_cast<float>(mpos.y));
    Event ev;
    ev.eventMouseButton = e;
    ev.type = EventType::MouseButtonReleased;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_MOVED(const int& mouseX, const int& mouseY){
    const float& mX = static_cast<float>(mouseX);
    const float& mY = static_cast<float>(mouseY);
    if(Resources::getWindow().hasFocus()){
        Core::m_Engine->m_EventManager.setMousePositionInternal(mX,mY,false,false);
    }
    Game::onMouseMoved(mX,mY);

    EventMouseMove e(mX,mY);
    Event ev;
    ev.eventMouseMoved = e;
    ev.type = EventType::MouseMoved;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_ENTERED(){ 
    Game::onMouseEntered(); 

    const glm::uvec2 mpos = Engine::getMousePosition();
    EventMouseMove e(static_cast<float>(mpos.x),static_cast<float>(mpos.y));
    Event ev;
    ev.eventMouseMoved = e;
    ev.type = EventType::MouseEnteredWindow;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_MOUSE_LEFT(){ 
    Game::onMouseLeft(); 

    const glm::uvec2 mpos = Engine::getMousePosition();
    EventMouseMove e(static_cast<float>(mpos.x),static_cast<float>(mpos.y));
    Event ev;
    ev.eventMouseMoved = e;
    ev.type = EventType::MouseLeftWindow;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_JOYSTICK_BUTTON_PRESSED(const uint& button, const uint& id){
    Game::onJoystickButtonPressed();

    EventJoystickButton e(id,button);
    Event ev;
    ev.eventJoystickButton = e;
    ev.type = EventType::JoystickButtonPressed;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_JOYSTICK_BUTTON_RELEASED(const uint& button, const uint& id){
    Game::onJoystickButtonReleased();

    EventJoystickButton e(id,button);
    Event ev;
    ev.eventJoystickButton = e;
    ev.type = EventType::JoystickButtonReleased;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_JOYSTICK_MOVED(const uint& id, const float& position, const uint axis){
    Game::onJoystickMoved();

    EventJoystickMoved e(id, (JoystickAxis::Axis)axis,position);
    Event ev;
    ev.eventJoystickMoved = e;
    ev.type = EventType::JoystickMoved;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_JOYSTICK_CONNECTED(const uint& id){
    Game::onJoystickConnected(); 

    EventJoystickConnection e(id);
    Event ev;
    ev.eventJoystickConnection = e;
    ev.type = EventType::JoystickConnected;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EVENT_JOYSTICK_DISCONNECTED(const uint& id){
    Game::onJoystickDisconnected(); 

    EventJoystickConnection e(id);
    Event ev;
    ev.eventJoystickConnection = e;
    ev.type = EventType::JoystickDisconnected;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
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
    Core::m_Engine->m_Misc.m_Destroyed = true;
}
void Engine::setFullScreen(const bool& b){ 
    Engine::Resources::getWindow().setFullScreen(b); 
}

void handleEvents(){
    sf::Event e;
    while(Resources::getWindow().getSFMLHandle().pollEvent(e)){
        switch(e.type){
            case sf::Event::Closed:{
                EVENT_CLOSE(); break;
            }case sf::Event::KeyReleased:{
                EVENT_KEY_RELEASED(e.key.code); break;
            }case sf::Event::KeyPressed:{
                EVENT_KEY_PRESSED(e.key.code); break;
            }case sf::Event::MouseButtonPressed:{
                EVENT_MOUSE_BUTTON_PRESSED(e.mouseButton.button); break;
            }case sf::Event::MouseButtonReleased:{
                EVENT_MOUSE_BUTTON_RELEASED(e.mouseButton.button); break;
            }case sf::Event::MouseEntered:{
                EVENT_MOUSE_ENTERED(); break;
            }case sf::Event::MouseLeft:{
                EVENT_MOUSE_LEFT(); break;
            }case sf::Event::MouseWheelMoved:{
                EVENT_MOUSE_WHEEL_MOVED(e.mouseWheel.delta); break;
            }case sf::Event::MouseMoved:{
                EVENT_MOUSE_MOVED(e.mouseMove.x,e.mouseMove.y); break;
            }case sf::Event::Resized:{
                EVENT_RESIZE(e.size.width,e.size.height,true); break;
            }case sf::Event::TextEntered:{
                EVENT_TEXT_ENTERED(e.text.unicode); break;
            }case sf::Event::JoystickButtonPressed:{
                EVENT_JOYSTICK_BUTTON_PRESSED(e.joystickButton.button,e.joystickButton.joystickId); break;
            }case sf::Event::JoystickButtonReleased:{
                EVENT_JOYSTICK_BUTTON_RELEASED(e.joystickButton.button,e.joystickButton.joystickId); break;
            }case sf::Event::JoystickConnected:{
                EVENT_JOYSTICK_CONNECTED(e.joystickConnect.joystickId); break;
            }case sf::Event::JoystickDisconnected:{
                EVENT_JOYSTICK_DISCONNECTED(e.joystickConnect.joystickId); break;
            }case sf::Event::JoystickMoved:{
                EVENT_JOYSTICK_MOVED(e.joystickMove.joystickId,e.joystickMove.position,e.joystickMove.axis); break;
            }default:{
                break;
            }
        }
    }
}

void Engine::run(){
    while(!Core::m_Engine->m_Misc.m_Destroyed /*&& Resources::getWindow().isOpen()*/){
        auto& debugMgr = Core::m_Engine->m_DebugManager;
        double dt = debugMgr.dt();
        handleEvents();
        update(dt);
        render(dt);
        debugMgr.calculate();
    }
    Game::cleanup();
    SAFE_DELETE(Core::m_Engine);
}