
#include <serenity/core/engine/system/Engine.h>
#include <serenity/core/engine/system/EngineOptions.h>
#include <serenity/core/engine/system/EngineGameFunctions.h>
#include <serenity/core/engine/system/window/Window.h>
#include <serenity/ecs/ECS.h>
#include <serenity/ecs/ComponentCamera.h>

#include <serenity/core/engine/scene/Scene.h>
#include <serenity/core/engine/scene/Camera.h>
#include <serenity/core/engine/scene/Viewport.h>

#include <serenity/core/engine/renderer/postprocess/SSAO.h>
#include <serenity/core/engine/renderer/postprocess/HDR.h>
#include <serenity/core/engine/renderer/postprocess/GodRays.h>
#include <serenity/core/engine/renderer/postprocess/Fog.h>
#include <unordered_set>

using namespace Engine;
using namespace Engine::priv;

#if defined(ENGINE_PRODUCTION)
    #ifdef _WIN32
        #pragma comment(linker, "/SUBSYSTEM:WINDOWS" /*+ " /ENTRY:mainCRTStartup"*/) //forcibly hide the console
    #endif
#endif

//faux game functions if test suite enabled
#if defined(ENGINE_TESTS)
    void Game::initResources() {}
    void Game::initLogic() {}
    void Game::update(const float dt) {}
    void Game::render() {}
    void Game::cleanup() {}

    void Game::onResize(Window& window, uint32_t width, uint32_t height) {}
    void Game::onWindowRequestedToBeClosed(Window& window) {}
    void Game::onWindowClosed(Window& window) {}
    void Game::onGameEnded() {}
    void Game::onLostFocus(Window& window) {}
    void Game::onGainedFocus(Window& window) {}
    void Game::onTextEntered(Window& window, uint32_t unicode) {}
    void Game::onKeyPressed(Window& window, uint32_t key) {}
    void Game::onKeyReleased(Window& window, uint32_t key) {}
    void Game::onMouseWheelScrolled(Window& window, float delta, int x, int y) {}
    void Game::onMouseButtonPressed(Window& window, uint32_t button) {}
    void Game::onMouseButtonReleased(Window& window, uint32_t button) {}
    void Game::onMouseMoved(Window& window, float mouseX, float mouseY) {}
    void Game::onMouseEntered(Window& window) {}
    void Game::onMouseLeft(Window& window) {}
    void Game::onPreUpdate(const float dt) {}
    void Game::onPostUpdate(const float dt) {}
    void Game::onJoystickButtonPressed() {}
    void Game::onJoystickButtonReleased() {}
    void Game::onJoystickMoved() {}
    void Game::onJoystickConnected() {}
    void Game::onJoystickDisconnected() {}
#endif

EngineCore* Core::m_Engine = nullptr;

EngineCore::EngineCore(const EngineOptions& options) 
    : m_ResourceManager{ options }
    , m_RenderModule{ options }
    , m_EngineEventHandler{ m_EventModule, m_RenderModule, m_ResourceManager }
{
    m_Misc.m_MainThreadID = std::this_thread::get_id();
}
EngineCore::~EngineCore(){
    m_EngineEventHandler.internal_on_event_game_ended();
    Game::cleanup();
    internal_cleanup_os_specific();
}
void EngineCore::internal_init_os_specific(const EngineOptions& options) {
    #ifdef _WIN32
        //get args from shortcut, etc
        std::unordered_set<std::string> args;
        std::locale loc;
        for (int i = 0; i < options.argc; i++) {
            std::string key      = std::string(options.argv[i]);
            std::string lowerKey = "";
            for (size_t j = 0; j < key.length(); ++j) {
                lowerKey += std::tolower(key[j], loc);
            }
            args.insert(std::move(lowerKey));
        }
        //show console if applicable
        if (options.show_console) {
            if (GetConsoleWindow() == NULL) {
                AllocConsole();
            }
        }else if (!args.contains("console")) {
            ShowWindow(GetConsoleWindow(), SW_HIDE);//hide console window
        }
    #endif
}
void EngineCore::internal_cleanup_os_specific() {
    //#ifdef _WIN32
    //    if (GetConsoleWindow() != NULL) {
            //FreeConsole(); //erroring out for some reason
    //    }
    //#endif
}
void EngineCore::init(const EngineOptions& options) {
    internal_init_os_specific(options);

    m_ResourceManager.init(options);
    m_DebugManager._init();
    m_Misc.m_BuiltInMeshes.init();
    m_RenderModule._init();
    m_PhysicsModule.init();

    //init the game here
    Engine::setMousePosition(options.width / 2, options.height / 2);
    Game::initResources();
    Game::initLogic();

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
void EngineCore::internal_update_physics(Scene& scene, Window& window, const float timeStep) {
    m_DebugManager.stop_clock();
    float fixed_time_step = PHYSICS_MIN_STEP / (float)Physics::getNumberOfStepsPerFrame();
    m_PhysicsModule.update(timeStep, 100, fixed_time_step);
    m_DebugManager.calculate_physics();
}
void EngineCore::internal_update_logic(Scene& scene, Window& window, const float dt){
    m_DebugManager.stop_clock();
    window.internal_on_dynamic_resize();
    m_NetworkingModule.update(dt);

    m_Misc.m_QueuedCommands.for_each_and_clear([](std::function<void()>& item) mutable { item(); });

    Game::update(dt);
    scene.update(dt);
    m_ThreadingModule.update(dt);

    m_DiscordModule.update();
    m_DebugManager.calculate_logic();
}
void EngineCore::internal_update_sounds(Scene& scene, Window& window, const float dt){
    m_DebugManager.stop_clock();
    m_SoundModule.update(scene, dt);
    m_DebugManager.calculate_sounds();
}
void EngineCore::internal_pre_update(Scene& scene, Window& window, const float dt) {
    m_PhysicsModule.preUpdate(dt);
    Game::onPreUpdate(dt);
    scene.preUpdate(dt);
}
void EngineCore::internal_post_update(Scene& scene, Window& window, const float dt) {
    Game::onPostUpdate(dt);
    scene.postUpdate(dt);
    m_EventModule.postUpdate();
    window.m_Data.internal_on_reset_events(dt);
}
void EngineCore::internal_render(Scene& scene, Window& window, const float dt){
    m_DebugManager.stop_clock();
    Game::render();
    if (Engine::priv::InternalScenePublicInterface::IsSkipRenderThisFrame(scene)) {
        Engine::priv::InternalScenePublicInterface::SkipRenderThisFrame(scene, false);
        return;
    }
    scene.render();
    m_RenderModule._sort2DAPICommands();
    auto& scene_viewports = InternalScenePublicInterface::GetViewports(scene);
    for (auto& viewport : scene_viewports) {
        if (viewport.isActive()) {
            m_RenderModule._render( viewport, true );
        }
    }
    window.display();
    m_RenderModule._clear2DAPICommands();
    m_DebugManager.calculate_render();
}
void EngineCore::internal_cleanup(Window& window, const float dt) {
    m_ResourceManager.postUpdate();
}
bool Engine::paused() noexcept {
    return Core::m_Engine->m_Misc.m_Paused;
}
void Engine::pause(bool b) noexcept {
    Engine::Physics::pause(b);
    Core::m_Engine->m_Misc.m_Paused = b;
}
void Engine::unpause() noexcept {
    Engine::Physics::unpause();
    Core::m_Engine->m_Misc.m_Paused = false;
}
void Engine::setTimeScale(float timeScale) noexcept {
    Core::m_Engine->m_DebugManager.setTimeScale(timeScale);
}
float Engine::getFPS() noexcept { 
    return 1.0f / Resources::dt(); 
}
Window& Engine::getWindow() noexcept {
    return Engine::Resources::getWindow();
}
glm::uvec2 Engine::getWindowSize() noexcept {
    return Engine::Resources::getWindowSize(); 
}
void Engine::setWindowIcon(const Texture& texture){ 
    Resources::getWindow().setIcon(texture); 
}
void Engine::setWindowIcon(Handle textureHandle) {
    Resources::getWindow().setIcon(*textureHandle.get<Texture>());
}
void Engine::showMouseCursor() noexcept {
    Resources::getWindow().setMouseCursorVisible(true); 
}
void Engine::hideMouseCursor() noexcept {
    Resources::getWindow().setMouseCursorVisible(false); 
}
void Engine::stop() noexcept {
    Core::m_Engine->m_Misc.m_Destroyed = true;
}
bool Engine::setFullscreen(bool fullscreen) noexcept {
    return Resources::getWindow().setFullscreen(fullscreen);
}
bool Engine::setFullscreenWindowed(bool fullscreenWindowed) noexcept {
    return Resources::getWindow().setFullscreenWindowed(fullscreenWindowed);
}


void EngineCore::run(){
    while (!m_Misc.m_Destroyed) {
        const float dt = m_DebugManager.dt() * m_DebugManager.timeScale();
        for (const auto& window_itr : m_ResourceManager.m_Windows) {
            auto& window = *window_itr;
            auto& scene  = *Engine::Resources::getCurrentScene();
            m_EngineEventHandler.poll_events(window);
            internal_pre_update(scene, window, dt);
            internal_update_physics(scene, window, dt);
            internal_update_logic(scene, window, dt);
            internal_update_sounds(scene, window, dt);
            internal_post_update(scene, window, dt);
            internal_render(scene, window, dt);
            internal_cleanup(window, dt);
        }
        m_DebugManager.calculate();
    }
}