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
m_EventManager(),
m_ResourceManager(options.window_title, options.width, options.height),
m_DebugManager(),
m_SoundManager(),
m_RenderManager(options.window_title, options.width, options.height),
m_PhysicsManager(),
m_ThreadManager()
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

void EngineCore::init(const EngineOptions& options) {
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
    epriv::threading::waitForAll();

    //the scene is the root of all games. create the default scene if 1 does not exist already
    if (engine.m_ResourceManager._numScenes() == 0) {
        Scene* defaultScene = new Scene("Default");
        Resources::setCurrentScene(defaultScene);
    }
    Scene& scene = *Resources::getCurrentScene();
    if (!scene.getActiveCamera()) {
        Camera* default_camera = new Camera(60, static_cast<float>(options.width) / static_cast<float>(options.height), 0.01f, 1000.0f, &scene);
        scene.setActiveCamera(*default_camera);
    }

    Renderer::ssao::enable(options.ssao_enabled);
    Renderer::godRays::enable(options.god_rays_enabled);
    Renderer::hdr::enable(options.hdr_enabled);
    Renderer::fog::enable(options.fog_enabled);
    Renderer::Settings::setAntiAliasingAlgorithm(options.aa_algorithm);
}
void EngineCore::update_physics(const double& dt) {
    m_DebugManager.stop_clock();
    //It's important that timeStep is always less than maxSubSteps * fixedTimeStep, otherwise you are losing time. dt < maxSubSteps * fixedTimeStep
    uint maxSubSteps = 0;
    while (true) {
        ++maxSubSteps;
        if (dt < static_cast<double>(maxSubSteps * PHYSICS_MIN_STEP))
            break;
    }
    m_PhysicsManager._update(dt, maxSubSteps, PHYSICS_MIN_STEP);
    m_DebugManager.calculate_physics();
}
void EngineCore::update_logic(const double& dt){
    // update logic   //////////////////////////////////////////
    m_DebugManager.stop_clock();
    //update current scene
    Scene& scene = *Resources::getCurrentScene();
    auto& ecs = InternalScenePublicInterface::GetECS(scene);
    update_physics(dt);
    ecs.preUpdate(scene, dt);
    Game::onPreUpdate(dt);

    Game::update(dt);

    scene.update(dt);
    ecs.update(dt, scene);
    ecs.postUpdate(scene,dt);

    m_ThreadManager._update(dt);
    Game::onPostUpdate(dt);

    m_DebugManager.calculate_logic();

    m_EventManager.onResetEvents(dt);
    ////////////////////////////////////////////////////////////
}
void EngineCore::update_sounds(const double& dt){
    m_DebugManager.stop_clock();
    m_SoundManager._update(dt);
    m_DebugManager.calculate_sounds();
}
void EngineCore::update(const double& dt){
    update_logic(dt);
    update_sounds(dt);
}

void EngineCore::render(const double& dt){
    m_DebugManager.stop_clock();
    Game::render();
    auto& scene = *Resources::getCurrentScene();
    scene.render();
    m_RenderManager._sort2DAPICommands();
    auto& viewports = InternalScenePublicInterface::GetViewports(scene);
    for (auto& viewport : viewports) {
        if (viewport->isActive()) {
            m_RenderManager._render(dt, *viewport, true, 0, 0);
        }
    }
    Resources::getWindow().display();
    m_RenderManager._clear2DAPICommands();
    m_DebugManager.calculate_render();
}
void EngineCore::cleanup(const double& dt) {
    m_ResourceManager.onPostUpdate();
}
void EngineCore::on_event_resize(const uint& w, const uint& h, const bool& saveSize){
    Core::m_Engine->m_RenderManager._resize(w,h);

    if(saveSize) 
        Resources::getWindow().setSize(w,h);
    Game::onResize(w,h);
    //resize cameras here

    for (auto& scene : Core::m_Engine->m_ResourceManager.scenes()) {
        scene->onResize(w, h);
        InternalScenePublicInterface::GetECS(*scene).onResize<ComponentCamera>(w, h);
        InternalScenePublicInterface::GetViewports(*scene)[0]->setViewportDimensions(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));
    }

    EventWindowResized e(w,h);
    Event ev;
    ev.eventWindowResized = e;
    ev.type = EventType::WindowResized;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_close(){
    Resources::getWindow().close();
    Game::onClose();

    Event ev;
    ev.type = EventType::WindowClosed;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_lost_focus(){
    Game::onLostFocus();

    Event ev;
    ev.type = EventType::WindowLostFocus;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_gained_focus(){
    Game::onGainedFocus();

    Event ev;
    ev.type = EventType::WindowGainedFocus;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_text_entered(const uint& unicode){
    Game::onTextEntered(unicode); 

    EventTextEntered e(unicode);
    Event ev;
    ev.eventTextEntered = e;
    ev.type = EventType::TextEntered;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_key_pressed(const uint& key){
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
void EngineCore::on_event_key_released(const uint& key){
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
void EngineCore::on_event_mouse_wheel_moved(const int& delta){
    Core::m_Engine->m_EventManager.onEventMouseWheelMoved(delta);
    Game::onMouseWheelMoved(delta);

    epriv::EventMouseWheel e(delta);
    Event ev;
    ev.eventMouseWheel = e;
    ev.type = EventType::MouseWheelMoved;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_button_pressed(const uint& mouseButton){
    Core::m_Engine->m_EventManager.onEventMouseButtonPressed(mouseButton);
    Game::onMouseButtonPressed(mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition();
    EventMouseButton e(static_cast<MouseButton::Button>(mouseButton), static_cast<float>(mpos.x), static_cast<float>(mpos.y));
    Event ev;
    ev.eventMouseButton = e;
    ev.type = EventType::MouseButtonPressed;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_button_released(const uint& mouseButton){
    Core::m_Engine->m_EventManager.onEventMouseButtonReleased(mouseButton);
    Game::onMouseButtonReleased(mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition();
    EventMouseButton e(static_cast<MouseButton::Button>(mouseButton), static_cast<float>(mpos.x), static_cast<float>(mpos.y));
    Event ev;
    ev.eventMouseButton = e;
    ev.type = EventType::MouseButtonReleased;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_moved(const int& mouseX, const int& mouseY){
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
void EngineCore::on_event_mouse_entered(){
    Game::onMouseEntered(); 

    const glm::uvec2 mpos = Engine::getMousePosition();
    EventMouseMove e(static_cast<float>(mpos.x),static_cast<float>(mpos.y));
    Event ev;
    ev.eventMouseMoved = e;
    ev.type = EventType::MouseEnteredWindow;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_left(){
    Game::onMouseLeft(); 

    const glm::uvec2 mpos = Engine::getMousePosition();
    EventMouseMove e(static_cast<float>(mpos.x),static_cast<float>(mpos.y));
    Event ev;
    ev.eventMouseMoved = e;
    ev.type = EventType::MouseLeftWindow;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_button_pressed(const uint& button, const uint& id){
    Game::onJoystickButtonPressed();

    EventJoystickButton e(id,button);
    Event ev;
    ev.eventJoystickButton = e;
    ev.type = EventType::JoystickButtonPressed;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_button_released(const uint& button, const uint& id){
    Game::onJoystickButtonReleased();

    EventJoystickButton e(id,button);
    Event ev;
    ev.eventJoystickButton = e;
    ev.type = EventType::JoystickButtonReleased;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_moved(const uint& id, const float& position, const uint axis){
    Game::onJoystickMoved();

    EventJoystickMoved e(id, (JoystickAxis::Axis)axis,position);
    Event ev;
    ev.eventJoystickMoved = e;
    ev.type = EventType::JoystickMoved;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_connected(const uint& id){
    Game::onJoystickConnected(); 

    EventJoystickConnection e(id);
    Event ev;
    ev.eventJoystickConnection = e;
    ev.type = EventType::JoystickConnected;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_disconnected(const uint& id){
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

void EngineCore::handle_events(){
    sf::Event e;
    while(Resources::getWindow().getSFMLHandle().pollEvent(e)){
        switch(e.type){
            case sf::Event::Closed:{
                on_event_close(); break;
            }case sf::Event::KeyReleased:{
                on_event_key_released(e.key.code); break;
            }case sf::Event::KeyPressed:{
                on_event_key_pressed(e.key.code); break;
            }case sf::Event::MouseButtonPressed:{
                on_event_mouse_button_pressed(e.mouseButton.button); break;
            }case sf::Event::MouseButtonReleased:{
                on_event_mouse_button_released(e.mouseButton.button); break;
            }case sf::Event::MouseEntered:{
                on_event_mouse_entered(); break;
            }case sf::Event::MouseLeft:{
                on_event_mouse_left(); break;
            }case sf::Event::MouseWheelMoved:{
                on_event_mouse_wheel_moved(e.mouseWheel.delta); break;
            }case sf::Event::MouseMoved:{
                on_event_mouse_moved(e.mouseMove.x,e.mouseMove.y); break;
            }case sf::Event::Resized:{
                on_event_resize(e.size.width,e.size.height,true); break;
            }case sf::Event::TextEntered:{
                on_event_text_entered(e.text.unicode); break;
            }case sf::Event::JoystickButtonPressed:{
                on_event_joystick_button_pressed(e.joystickButton.button,e.joystickButton.joystickId); break;
            }case sf::Event::JoystickButtonReleased:{
                on_event_joystick_button_released(e.joystickButton.button,e.joystickButton.joystickId); break;
            }case sf::Event::JoystickConnected:{
                on_event_joystick_connected(e.joystickConnect.joystickId); break;
            }case sf::Event::JoystickDisconnected:{
                on_event_joystick_disconnected(e.joystickConnect.joystickId); break;
            }case sf::Event::JoystickMoved:{
                on_event_joystick_moved(e.joystickMove.joystickId,e.joystickMove.position,e.joystickMove.axis); break;
            }default:{
                break;
            }
        }
    }
}

void EngineCore::run(){
    while(!m_Misc.m_Destroyed /*&& Resources::getWindow().isOpen()*/){
        double dt = m_DebugManager.dt();
        handle_events();
        update(dt);
        render(dt);
        cleanup(dt);
        m_DebugManager.calculate();
    }
    Game::cleanup();
    SAFE_DELETE(Core::m_Engine);
}