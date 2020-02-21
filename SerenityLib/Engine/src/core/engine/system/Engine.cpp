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

#include <glm/vec2.hpp>
#include <SFML/System.hpp>

using namespace Engine;
using namespace Engine::priv;
using namespace std;


EngineCore* Core::m_Engine = nullptr;

EngineCore::EngineCore(const EngineOptions& options) : m_ResourceManager(options), m_RenderManager(options){
    m_Misc.m_Destroyed = m_Misc.m_Paused = false;
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
void Engine::pause(const bool& b){
    Engine::Physics::pause(b);
    Core::m_Engine->m_Misc.m_Paused = b;
}
void Engine::unpause(){
    Engine::Physics::unpause();
    Core::m_Engine->m_Misc.m_Paused = false;
}
void Engine::setTimeScale(const float timeScale) {
    Core::m_Engine->m_DebugManager.setTimeScale(timeScale);
}
void EngineCore::init_os_specific(const EngineOptions& options) {
    #ifdef _WIN32

        //get args from shortcut, etc
        unordered_set<string> args; std::locale loc;
        for (int i = 0; i < options.argc; i++) {
            const string key = string(options.argv[i]);
            string lowerKey = "";
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
        Camera* default_camera = NEW Camera(60, static_cast<float>(options.width) / static_cast<float>(options.height), 0.01f, 1000.0f, &scene);
        scene.setActiveCamera(*default_camera);
    }

    Engine::Renderer::ssao::enable(options.ssao_enabled);
    Engine::Renderer::godRays::enable(options.god_rays_enabled);
    Engine::Renderer::hdr::setAlgorithm(static_cast<HDRAlgorithm::Algorithm>(options.hdr));
    Engine::Renderer::fog::enable(options.fog_enabled);
    Engine::Renderer::Settings::setAntiAliasingAlgorithm(options.aa_algorithm);
}
void EngineCore::update_physics(Window& window, const float& dt) {
    m_DebugManager.stop_clock();

    //It's important that dt < actual_steps * PHYSICS_MIN_STEP / static_cast<float>(requested_steps), otherwise you are losing time. dt < maxSubSteps * fixedTimeStep
    const auto requested_steps = Physics::getNumberOfStepsPerFrame();
    int actual_steps           = requested_steps;
    const auto fixed_time_step = PHYSICS_MIN_STEP / static_cast<double>(requested_steps);

    while (true) {
        const double threshold = static_cast<double>(actual_steps) * fixed_time_step;
        if (actual_steps == 1 || dt >= threshold) {
            break;
        }
        --actual_steps;
    }
    m_PhysicsManager._update(dt, actual_steps, static_cast<float>(fixed_time_step));

    m_DebugManager.calculate_physics();
}
void EngineCore::update_logic(Window& window, const float& dt){
    m_DebugManager.stop_clock();
    window.on_dynamic_resize();
    Scene& scene = *Resources::getCurrentScene();
    auto& ecs = InternalScenePublicInterface::GetECS(scene); 
    ecs.preUpdate(scene, dt);
    Game::onPreUpdate(dt);
    Game::update(dt);
    scene.update(dt);
    InternalScenePublicInterface::UpdateParticleSystem(scene, dt);
    update_physics(window, dt);
    ecs.update(dt, scene);
    ecs.postUpdate(scene,dt);
    InternalScenePublicInterface::UpdateMaterials(scene, dt);
    m_ThreadManager._update(dt);
    Game::onPostUpdate(dt);
    m_DebugManager.calculate_logic();
    m_EventManager.onResetEvents(dt);

    window.m_Data.on_reset_events(dt);

    m_Misc.m_DiscordCore.update();
}
void EngineCore::update_sounds(Window& window, const float& dt){
    m_DebugManager.stop_clock();
    m_SoundManager._update(dt);
    m_DebugManager.calculate_sounds();
}
void EngineCore::update(Window& window, const float& dt){
    update_logic(window, dt);
    update_sounds(window, dt);
}
void EngineCore::render(Window& window, const float& dt){
    m_DebugManager.stop_clock();
    Game::render();
    auto& scene = *Resources::getCurrentScene();
    scene.render();
    m_RenderManager._sort2DAPICommands();
    auto& scene_viewports = InternalScenePublicInterface::GetViewports(scene);
    for (auto& viewport : scene_viewports) {
        if (viewport.isActive()) {
            m_RenderManager._render(dt, viewport, true, 0, 0);
        }
    }
    window.display();
    m_RenderManager._clear2DAPICommands();
    m_DebugManager.calculate_render();
}
void EngineCore::cleanup(Window& window, const float& dt) {
    m_ResourceManager.onPostUpdate();
}
void EngineCore::on_event_resize(Window& window, const unsigned int& newWindowWidth, const unsigned int& newWindowHeight, const bool& saveSize){
    m_EventManager.cleanup();
    m_RenderManager._resize(newWindowWidth, newWindowHeight);

    if (saveSize) {
        window.m_Data.m_VideoMode.width = newWindowWidth;
        window.m_Data.m_VideoMode.height = newWindowHeight;
    }
    //resize cameras and viewports here
    for (auto& scene : m_ResourceManager.scenes()) {
        if (scene) {
            scene->onResize(newWindowWidth, newWindowHeight);
            InternalScenePublicInterface::GetECS(*scene).onResize<ComponentCamera>(newWindowWidth, newWindowHeight);
            InternalScenePublicInterface::GetViewports(*scene)[0].setViewportDimensions(0.0f, 0.0f, static_cast<float>(newWindowWidth), static_cast<float>(newWindowHeight));
        }
    }

    Game::onResize(window, newWindowWidth, newWindowHeight);

    EventWindowResized e(newWindowWidth, newWindowHeight);
    Event ev(EventType::WindowResized);
    ev.eventWindowResized = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_window_requested_closed(Window& window){
    Event ev(EventType::WindowRequestedToBeClosed);
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);

    Game::onWindowRequestedToBeClosed(window);
    window.close();
}
void EngineCore::on_event_game_ended() {
    Game::onGameEnded();

    Event ev(EventType::GameEnded);
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_window_closed(Window& window) {
    Game::onWindowClosed(window);
    window.m_Data.on_close();

    Event ev(EventType::WindowHasClosed);
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_lost_focus(Window& window){
    m_EventManager.cleanup();
    Game::onLostFocus(window);

    Event ev(EventType::WindowLostFocus);
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_gained_focus(Window& window){
    m_EventManager.cleanup();
    Game::onGainedFocus(window);

    window.m_Data.on_reset_events(0.0);

    Event ev(EventType::WindowGainedFocus);
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_text_entered(Window& window, const unsigned int& unicode){
    Game::onTextEntered(window, unicode);

    EventTextEntered e(unicode);
    Event ev(EventType::TextEntered);
    ev.eventTextEntered = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_key_pressed(Window& window, const unsigned int& key){
    m_EventManager.onEventKeyPressed(key);
    Game::onKeyPressed(window, key);

    priv::EventKeyboard e;
    e.key = static_cast<KeyboardKey::Key>(key);
    if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))      e.control = true;
    if(Engine::isKeyDown(KeyboardKey::LeftAlt)     || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
    if(Engine::isKeyDown(KeyboardKey::LeftShift)   || Engine::isKeyDown(KeyboardKey::RightShift))        e.shift = true;
    if(Engine::isKeyDown(KeyboardKey::LeftSystem)  || Engine::isKeyDown(KeyboardKey::RightSystem))       e.system = true;

    Event ev(EventType::KeyPressed);
    ev.eventKeyboard = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_key_released(Window& window, const unsigned int& key){
    m_EventManager.onEventKeyReleased(key);
    Game::onKeyReleased(window, key);

    priv::EventKeyboard e;
    e.key = static_cast<KeyboardKey::Key>(key);
    if(Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl))      e.control = true;
    if(Engine::isKeyDown(KeyboardKey::LeftAlt)     || Engine::isKeyDown(KeyboardKey::RightAlt))          e.alt = true;
    if(Engine::isKeyDown(KeyboardKey::LeftShift)   || Engine::isKeyDown(KeyboardKey::RightShift))        e.shift = true;
    if(Engine::isKeyDown(KeyboardKey::LeftSystem)  || Engine::isKeyDown(KeyboardKey::RightSystem))       e.system = true;

    Event ev(EventType::KeyReleased);
    ev.eventKeyboard = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_wheel_scrolled(Window& window, const float& delta, const int& mouseWheelX, const int& mouseWheelY){
    window.m_Data.on_mouse_wheel_scrolled(delta, mouseWheelX, mouseWheelY);

    Game::onMouseWheelScrolled(window, delta, mouseWheelX, mouseWheelY);

    priv::EventMouseWheel e(delta, mouseWheelX, mouseWheelY);
    Event ev(EventType::MouseWheelMoved);
    ev.eventMouseWheel = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_button_pressed(Window& window, const unsigned int& mouseButton){
    m_EventManager.onEventMouseButtonPressed(mouseButton);
    Game::onMouseButtonPressed(window, mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition(window);
    EventMouseButton e(static_cast<MouseButton::Button>(mouseButton), static_cast<float>(mpos.x), static_cast<float>(mpos.y));
    Event ev(EventType::MouseButtonPressed);
    ev.eventMouseButton = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_button_released(Window& window, const unsigned int& mouseButton){
    m_EventManager.onEventMouseButtonReleased(mouseButton);
    Game::onMouseButtonReleased(window, mouseButton);

    const glm::uvec2 mpos = Engine::getMousePosition(window);
    EventMouseButton e(static_cast<MouseButton::Button>(mouseButton), static_cast<float>(mpos.x), static_cast<float>(mpos.y));
    Event ev(EventType::MouseButtonReleased);
    ev.eventMouseButton = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_moved(Window& window, const int& mouseX, const int& mouseY){
    const float& mX = static_cast<float>(mouseX);
    const float& mY = static_cast<float>(mouseY);
    if(window.hasFocus()){
        window.updateMousePosition(mX, mY, false, false);
    }
    Game::onMouseMoved(window, mX, mY);

    EventMouseMove e(mX,mY);
    Event ev(EventType::MouseMoved);
    ev.eventMouseMoved = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_entered(Window& window){
    Game::onMouseEntered(window);

    const glm::uvec2 mpos = Engine::getMousePosition(window);
    EventMouseMove e(static_cast<float>(mpos.x),static_cast<float>(mpos.y));
    Event ev(EventType::MouseEnteredWindow);
    ev.eventMouseMoved = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_mouse_left(Window& window){
    Game::onMouseLeft(window);

    const glm::uvec2 mpos = Engine::getMousePosition(window);
    EventMouseMove e(static_cast<float>(mpos.x),static_cast<float>(mpos.y));
    Event ev(EventType::MouseLeftWindow);
    ev.eventMouseMoved = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_button_pressed(Window& window, const unsigned int& button, const unsigned int& id){
    Game::onJoystickButtonPressed();

    EventJoystickButton e(id,button);
    Event ev(EventType::JoystickButtonPressed);
    ev.eventJoystickButton = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_button_released(Window& window, const unsigned int& button, const unsigned int& id){
    Game::onJoystickButtonReleased();

    EventJoystickButton e(id,button);
    Event ev(EventType::JoystickButtonReleased);
    ev.eventJoystickButton = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_moved(Window& window, const unsigned int& id, const float& position, const unsigned int axis){
    Game::onJoystickMoved();

    EventJoystickMoved e(id, (JoystickAxis::Axis)axis,position);
    Event ev(EventType::JoystickMoved);
    ev.eventJoystickMoved = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_connected(Window& window, const unsigned int& id){
    Game::onJoystickConnected(); 

    EventJoystickConnection e(id);
    Event ev(EventType::JoystickConnected);
    ev.eventJoystickConnection = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void EngineCore::on_event_joystick_disconnected(Window& window, const unsigned int& id){
    Game::onJoystickDisconnected(); 

    EventJoystickConnection e(id);
    Event ev(EventType::JoystickDisconnected);
    ev.eventJoystickConnection = e;
    m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}

const float Engine::getFPS(){ 
    return static_cast<float>(1.0 / Resources::dt()); 
}
Window& Engine::getWindow(){
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
const bool Engine::setFullscreen(const bool& b){ 
    return Resources::getWindow().setFullscreen(b); 
}
const bool Engine::setFullscreenWindowed(const bool& b) {
    return Resources::getWindow().setFullscreenWindowed(b);
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
    while(!m_Misc.m_Destroyed){
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