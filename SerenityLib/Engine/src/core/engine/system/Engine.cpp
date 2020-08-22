#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/system/Engine.h>
#include <core/engine/system/EngineOptions.h>
#include <core/engine/system/EngineGameFunctions.h>
#include <core/engine/system/window/Window.h>
#include <ecs/ECS.h>
#include <ecs/ComponentCamera.h>

#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Viewport.h>

#include <core/engine/renderer/postprocess/SSAO.h>
#include <core/engine/renderer/postprocess/HDR.h>
#include <core/engine/renderer/postprocess/GodRays.h>
#include <core/engine/renderer/postprocess/Fog.h>

#include <SFML/System.hpp>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

EngineCore* Core::m_Engine = nullptr;

EngineCore::EngineCore(const EngineOptions& options) : m_ResourceManager(options), m_RenderManager(options){
}
EngineCore::~EngineCore(){
    on_event_game_ended();
    Game::cleanup();
    cleanup_os_specific();

    m_Misc.m_BuiltInMeshes.cleanup();
}
bool Engine::paused(){ 
    return Core::m_Engine->m_Misc.m_Paused;
}
void Engine::pause(bool b){
    Engine::Physics::pause(b);
    Core::m_Engine->m_Misc.m_Paused = b;
}
void Engine::unpause(){
    Engine::Physics::unpause();
    Core::m_Engine->m_Misc.m_Paused = false;
}
void Engine::setTimeScale(float timeScale) {
    Core::m_Engine->m_DebugManager.setTimeScale(timeScale);
}
void EngineCore::init_os_specific(const EngineOptions& options) {
    #ifdef _WIN32

        //get args from shortcut, etc
        std::unordered_set<std::string> args;
        std::locale loc;
        for (int i = 0; i < options.argc; i++) {
            std::string key      = std::string(options.argv[i]);
            std::string lowerKey = "";
            for (size_t j = 0; j < key.length(); ++j)
                lowerKey += std::tolower(key[j], loc);
            args.insert(lowerKey);
        }

        //show console if applicable
        if (options.show_console) {
            if (GetConsoleWindow() == NULL) {
                AllocConsole();
            }
            freopen("CONIN$",  "r", stdin);
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
        }else if (!args.count("console")) {
            ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window
        }
    #endif
}
void EngineCore::cleanup_os_specific() {
    //#ifdef _WIN32
    //    if (GetConsoleWindow() != NULL) {
            //FreeConsole(); //erroring out for some reason
    //    }
    //#endif
}
void EngineCore::init(const EngineOptions& options) {
    init_os_specific(options);

    m_ResourceManager._init(options);
    m_DebugManager._init();
    m_Misc.m_BuiltInMeshes.init();
    m_RenderManager._init();
    m_PhysicsManager._init();

    //init the game here
    Engine::setMousePosition(options.width / 2, options.height / 2);
    Game::initResources();
    priv::threading::waitForAll();
    Game::initLogic();
    priv::threading::waitForAll();

    //the scene is the root of all games. create the default scene if 1 does not exist already
    if (m_ResourceManager.m_Scenes.size() == 0){
        Scene* defaultScene = NEW Scene("Default");
        Resources::setCurrentScene(defaultScene);
    }
    Scene& scene = *m_ResourceManager.m_CurrentScene;
    if (!scene.getActiveCamera()) {
        Camera* default_camera = scene.addCamera(60, (float)options.width / (float)options.height, 0.01f, 1000.0f);
    }

    Engine::Renderer::ssao::setLevel((SSAOLevel::Level)options.ssao_level);
    Engine::Renderer::godRays::enable(options.god_rays_enabled);
    Engine::Renderer::hdr::setAlgorithm((HDRAlgorithm::Algorithm)options.hdr);
    Engine::Renderer::fog::enable(options.fog_enabled);
    Engine::Renderer::Settings::setAntiAliasingAlgorithm(options.aa_algorithm);
}
void EngineCore::update_physics(Scene& scene, Window& window, const float timeStep) {
    m_DebugManager.stop_clock();

    unsigned int requestedStepsPerFrame  = Physics::getNumberOfStepsPerFrame();
    float fixed_time_step                = PHYSICS_MIN_STEP / (float)requestedStepsPerFrame;

    m_PhysicsManager._update(timeStep, 100, fixed_time_step);

    m_DebugManager.calculate_physics();
}
void EngineCore::update_logic(Scene& scene, Window& window, const float dt){
    m_DebugManager.stop_clock();
    window.internal_on_dynamic_resize();
    m_NetworkingModule.update(dt);
    Game::update(dt);
    scene.update(dt);
    Game::onPostUpdate(dt);
    scene.postUpdate(dt);
    m_ThreadingModule.update(dt);
    m_EventModule.onPostUpdate();

    window.m_Data.internal_on_reset_events(dt);

    m_DiscordModule.update();
    m_DebugManager.calculate_logic();
}
void EngineCore::update_sounds(Scene& scene, Window& window, const float dt){
    m_DebugManager.stop_clock();
    m_SoundModule.update(scene, dt);
    m_DebugManager.calculate_sounds();
}
void EngineCore::update(Window& window, const float dt){
    Scene& scene = *Resources::getCurrentScene();
    Game::onPreUpdate(dt);
    scene.preUpdate(dt);
    update_physics(scene, window, dt);
    update_logic(scene, window, dt);
    update_sounds(scene, window, dt);
}
void EngineCore::render(Window& window, const float dt){
    m_DebugManager.stop_clock();
    Game::render();
    auto& scene = *Resources::getCurrentScene();
    if (Engine::priv::InternalScenePublicInterface::IsSkipRenderThisFrame(scene)) {
        Engine::priv::InternalScenePublicInterface::SkipRenderThisFrame(scene, false);
        return;
    }
    scene.render();
    m_RenderManager._sort2DAPICommands();
    auto& scene_viewports = InternalScenePublicInterface::GetViewports(scene);
    for (auto& viewport : scene_viewports) {
        if (viewport.isActive()) {
            m_RenderManager._render( viewport, true );
        }
    }
    window.display();
    m_RenderManager._clear2DAPICommands();
    m_DebugManager.calculate_render();
}
void EngineCore::cleanup(Window& window, const float dt) {
    m_ResourceManager.onPostUpdate();
}
void EngineCore::on_event_resize(Window& window, unsigned int newWindowWidth, unsigned int newWindowHeight, bool saveSize){
    m_EventModule.onClearEvents();
    m_RenderManager._resize(newWindowWidth, newWindowHeight);

    if (saveSize) {
        window.m_Data.m_VideoMode.width  = newWindowWidth;
        window.m_Data.m_VideoMode.height = newWindowHeight;
    }
    //resize cameras and viewports here
    for (auto& scene : m_ResourceManager.scenes()) {
        if (scene) {
            scene->onResize(newWindowWidth, newWindowHeight);
            InternalScenePublicInterface::GetECS(*scene).onResize<ComponentCamera>(newWindowWidth, newWindowHeight);
            InternalScenePublicInterface::GetViewports(*scene)[0].setViewportDimensions(0.0f, 0.0f, (float)newWindowWidth, (float)newWindowHeight);
        }
    }

    Game::onResize(window, newWindowWidth, newWindowHeight);

    EventWindowResized e(newWindowWidth, newWindowHeight);
    Event ev(EventType::WindowResized);
    ev.eventWindowResized = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_window_requested_closed(Window& window){
    m_EventModule.m_EventDispatcher.dispatchEvent(EventType::WindowRequestedToBeClosed);

    Game::onWindowRequestedToBeClosed(window);
    window.close();
}
void EngineCore::on_event_game_ended() {
    Game::onGameEnded();

    m_EventModule.m_EventDispatcher.dispatchEvent(EventType::GameEnded);
}
void EngineCore::on_event_window_closed(Window& window) {
    Game::onWindowClosed(window);
    window.m_Data.internal_on_close();

    m_EventModule.m_EventDispatcher.dispatchEvent(EventType::WindowHasClosed);
}
void EngineCore::on_event_lost_focus(Window& window){
    m_EventModule.onClearEvents();
    Game::onLostFocus(window);

    m_EventModule.m_EventDispatcher.dispatchEvent(EventType::WindowLostFocus);
}
void EngineCore::on_event_gained_focus(Window& window){
    m_EventModule.onClearEvents();
    Game::onGainedFocus(window);

    window.m_Data.internal_on_reset_events(0.0f);

    m_EventModule.m_EventDispatcher.dispatchEvent(EventType::WindowGainedFocus);
}
void EngineCore::on_event_text_entered(Window& window, unsigned int unicode){
    Game::onTextEntered(window, unicode);

    EventTextEntered e(unicode);
    Event ev(EventType::TextEntered);
    ev.eventTextEntered = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_key_pressed(Window& window, unsigned int key){
    m_EventModule.onEventKeyPressed(key);
    Game::onKeyPressed(window, key);

    priv::EventKeyboard e;
    e.key = (KeyboardKey::Key)key;
    if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))      e.control = true;
    if(Engine::isKeyDown(KeyboardKey::LeftAlt)     || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
    if(Engine::isKeyDown(KeyboardKey::LeftShift)   || Engine::isKeyDown(KeyboardKey::RightShift))        e.shift = true;
    if(Engine::isKeyDown(KeyboardKey::LeftSystem)  || Engine::isKeyDown(KeyboardKey::RightSystem))       e.system = true;

    Event ev(EventType::KeyPressed);
    ev.eventKeyboard = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_key_released(Window& window, unsigned int key){
    m_EventModule.onEventKeyReleased(key);
    Game::onKeyReleased(window, key);

    priv::EventKeyboard e;
    e.key = (KeyboardKey::Key)key;
    if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))      e.control = true;
    if(Engine::isKeyDown(KeyboardKey::LeftAlt)     || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
    if(Engine::isKeyDown(KeyboardKey::LeftShift)   || Engine::isKeyDown(KeyboardKey::RightShift))        e.shift = true;
    if(Engine::isKeyDown(KeyboardKey::LeftSystem)  || Engine::isKeyDown(KeyboardKey::RightSystem))       e.system = true;

    Event ev(EventType::KeyReleased);
    ev.eventKeyboard = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_wheel_scrolled(Window& window, float delta, int mouseWheelX, int mouseWheelY){
    window.m_Data.internal_on_mouse_wheel_scrolled(delta, mouseWheelX, mouseWheelY);

    Game::onMouseWheelScrolled(window, delta, mouseWheelX, mouseWheelY);

    priv::EventMouseWheel e(delta, mouseWheelX, mouseWheelY);
    Event ev(EventType::MouseWheelMoved);
    ev.eventMouseWheel = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_button_pressed(Window& window, unsigned int mouseButton){
    m_EventModule.onEventMouseButtonPressed(mouseButton);
    Game::onMouseButtonPressed(window, mouseButton);

    const auto& mpos = Engine::getMousePosition(window);
    EventMouseButton e((MouseButton::Button)mouseButton, mpos.x, mpos.y);
    Event ev(EventType::MouseButtonPressed);
    ev.eventMouseButton = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_button_released(Window& window, unsigned int mouseButton){
    m_EventModule.onEventMouseButtonReleased(mouseButton);
    Game::onMouseButtonReleased(window, mouseButton);

    const auto& mpos = Engine::getMousePosition(window);
    EventMouseButton e((MouseButton::Button)mouseButton, mpos.x, mpos.y);
    Event ev(EventType::MouseButtonReleased);
    ev.eventMouseButton = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_moved(Window& window, int mouseX, int mouseY){
    float mX = (float)mouseX;
    float mY = (float)mouseY;
    if(window.hasFocus()){
        window.updateMousePosition(mX, mY, false, false);
    }
    Game::onMouseMoved(window, mX, mY);

    EventMouseMove e(mX, mY);
    Event ev(EventType::MouseMoved);
    ev.eventMouseMoved = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_entered(Window& window){
    Game::onMouseEntered(window);

    const auto& mpos = Engine::getMousePosition(window);
    EventMouseMove e(mpos.x, mpos.y);
    Event ev(EventType::MouseEnteredWindow);
    ev.eventMouseMoved = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_left(Window& window){
    Game::onMouseLeft(window);

    const auto& mpos = Engine::getMousePosition(window);
    EventMouseMove e(mpos.x, mpos.y);
    Event ev(EventType::MouseLeftWindow);
    ev.eventMouseMoved = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_button_pressed(Window& window, unsigned int button, unsigned int id){
    Game::onJoystickButtonPressed();

    EventJoystickButton e(id, button);
    Event ev(EventType::JoystickButtonPressed);
    ev.eventJoystickButton = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_button_released(Window& window, unsigned int button, unsigned int id){
    Game::onJoystickButtonReleased();

    EventJoystickButton e(id, button);
    Event ev(EventType::JoystickButtonReleased);
    ev.eventJoystickButton = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_moved(Window& window, unsigned int id, float position, unsigned int axis){
    Game::onJoystickMoved();

    EventJoystickMoved e(id, (JoystickAxis::Axis)axis, position);
    Event ev(EventType::JoystickMoved);
    ev.eventJoystickMoved = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_connected(Window& window, unsigned int id){
    Game::onJoystickConnected(); 

    EventJoystickConnection e(id);
    Event ev(EventType::JoystickConnected);
    ev.eventJoystickConnection = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_disconnected(Window& window, unsigned int id){
    Game::onJoystickDisconnected(); 

    EventJoystickConnection e(id);
    Event ev(EventType::JoystickDisconnected);
    ev.eventJoystickConnection = e;
    m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
float Engine::getFPS(){ 
    return 1.0f / Resources::dt(); 
}
Window& Engine::getWindow(){
    return Engine::Resources::getWindow();
}
glm::uvec2 Engine::getWindowSize(){ 
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
bool Engine::setFullscreen(bool fullscreen){ 
    return Resources::getWindow().setFullscreen(fullscreen);
}
bool Engine::setFullscreenWindowed(bool fullscreenWindowed) {
    return Resources::getWindow().setFullscreenWindowed(fullscreenWindowed);
}
void EngineCore::handle_events(Window& window){
    sf::Event e;
    while (window.pollEvents(e)) {
        switch (e.type) {
            case sf::Event::Closed: {
                on_event_window_requested_closed(window); break;
            }case sf::Event::LostFocus: {
                on_event_lost_focus(window); break;
            }case sf::Event::GainedFocus: {
                on_event_gained_focus(window); break;
            }case sf::Event::KeyReleased: {
                on_event_key_released(window, e.key.code); break;
            }case sf::Event::KeyPressed: {
                on_event_key_pressed(window, e.key.code); break;
            }case sf::Event::MouseButtonPressed: {
                on_event_mouse_button_pressed(window, e.mouseButton.button); break;
            }case sf::Event::MouseButtonReleased: {
                on_event_mouse_button_released(window, e.mouseButton.button); break;
            }case sf::Event::MouseEntered: {
                on_event_mouse_entered(window); break;
            }case sf::Event::MouseLeft: {
                on_event_mouse_left(window); break;
            }case sf::Event::MouseWheelScrolled: {
                on_event_mouse_wheel_scrolled(window, e.mouseWheelScroll.delta, e.mouseWheelScroll.x, e.mouseWheelScroll.y); break;
            }case sf::Event::MouseMoved: {
                on_event_mouse_moved(window, e.mouseMove.x, e.mouseMove.y); break;
            }case sf::Event::Resized: {
                on_event_resize(window, e.size.width, e.size.height, true); break;
            }case sf::Event::TextEntered: {
                on_event_text_entered(window, e.text.unicode); break;
            }case sf::Event::JoystickButtonPressed: {
                on_event_joystick_button_pressed(window, e.joystickButton.button, e.joystickButton.joystickId); break;
            }case sf::Event::JoystickButtonReleased: {
                on_event_joystick_button_released(window, e.joystickButton.button, e.joystickButton.joystickId); break;
            }case sf::Event::JoystickConnected: {
                on_event_joystick_connected(window, e.joystickConnect.joystickId); break;
            }case sf::Event::JoystickDisconnected: {
                on_event_joystick_disconnected(window, e.joystickConnect.joystickId); break;
            }case sf::Event::JoystickMoved: {
                on_event_joystick_moved(window, e.joystickMove.joystickId, e.joystickMove.position, e.joystickMove.axis); break;
            }default: {
                break;
            }
        }
    }
}

void EngineCore::run(){
    while (!m_Misc.m_Destroyed) {
        float dt = m_DebugManager.dt() * m_DebugManager.timeScale();
        for (auto& window_itr : m_ResourceManager.m_Windows) {
            auto& window = *window_itr;
            handle_events(window);
            update(window, dt);
            render(window, dt);
            cleanup(window, dt);
        }
        m_DebugManager.calculate();
    }
    SAFE_DELETE(Core::m_Engine);
}