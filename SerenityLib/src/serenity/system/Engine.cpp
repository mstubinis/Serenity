
#include <serenity/system/Engine.h>
#include <serenity/system/EngineOptions.h>
#include <serenity/system/EngineGameFunctions.h>
#include <serenity/ecs/ECS.h>
#include <serenity/ecs/components/ComponentCamera.h>

#include <serenity/scene/Scene.h>
#include <serenity/scene/Camera.h>
#include <serenity/scene/Viewport.h>
#include <serenity/system/Macros.h>

#include <serenity/renderer/postprocess/SSAO.h>
#include <serenity/renderer/postprocess/HDR.h>
#include <serenity/renderer/postprocess/GodRays.h>
#include <serenity/renderer/postprocess/Fog.h>
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
    void Game::initResources(const EngineOptions& options) {}
    void Game::initLogic(const EngineOptions& options) {}
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

EngineCore::EngineCore(const EngineOptions& options) 
    : m_ResourceManager{ options }
    , m_RenderModule{ options }
    , m_Editor{ options }
    , m_EngineEventHandler{ m_Editor, m_EventModule, m_RenderModule, m_ResourceManager }
{
    std::srand(static_cast<uint32_t>(std::time(0)));
    m_Misc.m_MainThreadID = std::this_thread::get_id();
}
EngineCore::~EngineCore() {
    m_EngineEventHandler.internal_on_event_game_ended();
    Game::cleanup();
    internal_cleanup_os_specific();
}
void EngineCore::internal_init_os_specific(const EngineOptions& options) {
    #ifdef _WIN32
        //get args from shortcut, etc
        std::unordered_set<std::string> args;
        std::locale loc;
        args.reserve(options.argc);
        for (int i = 0; i < options.argc; i++) {
            std::string key      = std::string(options.argv[i]);
            std::string lowerKey;
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
            ShowWindow(GetConsoleWindow(), SW_SHOW); //show console window
        }else if (!args.contains("console")) {
            ShowWindow(GetConsoleWindow(), SW_HIDE); //hide console window
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
    m_Editor.init(options, m_ResourceManager);

    //init the game here
    Engine::setMousePosition(options.width / 2, options.height / 2);
    Game::initResources(options);
    Game::initLogic(options);

    //the scene is the root of all games. create the default scene if 1 does not exist already
    if (m_ResourceManager.m_Scenes.size() == 0) {
        Scene* defaultScene = Engine::Resources::addScene<Scene>("Default");
        Engine::Resources::setCurrentScene(defaultScene);
    }
    Scene& scene = *m_ResourceManager.m_CurrentScene;
    if (!scene.getActiveCamera()) {
        auto default_camera = scene.addCamera<Camera>(60.0f, (float)options.width / (float)options.height, 0.01f, 1000.0f);
    }

    Engine::Renderer::ssao::setLevel((SSAOLevel::Level)options.ssao_level);
    Engine::Renderer::godRays::enable(options.god_rays_enabled);
    Engine::Renderer::hdr::setAlgorithm((HDRAlgorithm::Algorithm)options.hdr);
    Engine::Renderer::fog::enable(options.fog_enabled);
    Engine::Renderer::Settings::setAntiAliasingAlgorithm(options.aa_algorithm);
}
void EngineCore::internal_pre_input_update(Window& window) {
    m_EventModule.m_KeyboardModule.update();
    m_EventModule.m_MouseModule.update();
}
void EngineCore::internal_update_logic(Scene& scene, Window& window, const float dt) {
    m_DebugManager.stop_clock();
    window.internal_on_dynamic_resize();
    m_NetworkingModule.update(dt);

    m_Misc.m_QueuedCommands.for_each_and_clear([](std::function<void()>& item) mutable { item(); });

    scene.update(dt);
    m_ThreadingModule.update(dt);

    m_DiscordModule.update();
    m_DebugManager.calculate_logic();
}
void EngineCore::internal_update_sounds(Scene& scene, Window& window, const float dt) {
    m_DebugManager.stop_clock();
    m_SoundModule.update(scene, dt);
    m_DebugManager.calculate_sounds();
}
void EngineCore::internal_pre_update(Scene& scene, Window& window, const float dt) {
    if (Engine::priv::PublicScene::IsSkipRenderThisFrame(scene)) {
        scene.m_SkipRenderThisFrame = false;
    }
    Game::onPreUpdate(dt);
    scene.preUpdate(dt);
}
void EngineCore::internal_post_update(Scene& scene, Window& window, const float dt) {
    Game::onPostUpdate(dt);
    scene.postUpdate(dt);
    m_EventModule.postUpdate();
    window.m_Data.internal_on_reset_events(dt);
}
void EngineCore::internal_render(Scene& scene, Window& window, const float dt, const double alpha) {
    m_DebugManager.stop_clock();
    Game::render();
    if (Engine::priv::PublicScene::IsSkipRenderThisFrame(scene)) {
        return;
    }
    scene.render();
    m_Editor.renderLightIcons(scene);
    m_RenderModule._sort2DAPICommands();
    auto& scene_viewports = Engine::priv::PublicScene::GetViewports(scene);
    for (auto& viewport : scene_viewports) {
        if (viewport.isActive()) {
            viewport.render(m_RenderModule, viewport, true);
        }
    }

    m_Editor.render(window);

    window.display();
    m_RenderModule._clear2DAPICommands();
    m_DebugManager.calculate_render();

    m_Misc.m_FPS.incrementFPSCount();
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
double Engine::getFPS() noexcept { 
    return 1.0 / Core::m_Engine->m_Misc.m_Dt;
}
void Engine::stop() noexcept {
    Core::m_Engine->m_Misc.m_Destroyed = true;
}
void EngineCore::run() {
    using  millisecs         = std::chrono::duration<double, std::milli>;
    double accumulator       = ENGINE_FIXED_TIMESTEP_VALUE_D;
    auto   currentTime       = m_Misc.m_Timer.now();
    auto   updateSimulation  = [this](const float timeElasped) {
        for (const auto& window_itr : m_ResourceManager.m_Windows) {
            auto& window = *window_itr;
            auto& scene = *Engine::Resources::getCurrentScene();
            internal_pre_input_update(window);
            m_EngineEventHandler.poll_events(window);
            internal_pre_update(scene, window, timeElasped);
            m_Editor.update(window, timeElasped);
            internal_update_logic(scene, window, timeElasped);
            internal_update_sounds(scene, window, timeElasped);
            internal_post_update(scene, window, timeElasped);
            internal_cleanup(window, timeElasped);
        }
    };
    auto   renderSimulation  = [this, accumulator]() {
        const double alpha = accumulator / ENGINE_FIXED_TIMESTEP_VALUE_D;
        //State state = currentState * alpha + previousState * (1.0 - alpha);
        for (const auto& window_itr : m_ResourceManager.m_Windows) {
            auto& window = *window_itr;
            auto& scene  = *Engine::Resources::getCurrentScene();
            internal_render(scene, window, (float)m_Misc.m_Dt, alpha/*, state*/);
        }
    };
    updateSimulation(ENGINE_FIXED_TIMESTEP_VALUE); //initially call update once to get user code in place before the first render.
    while (!m_Misc.m_Destroyed) {
        auto newTime  = m_Misc.m_Timer.now();
        m_Misc.m_Dt   = std::chrono::duration_cast<millisecs>(newTime - currentTime).count() * 0.001; // * 0.001 converts from millisecs to secs
        m_Misc.m_Dt   = std::min(m_Misc.m_Dt, 0.25); //not sure if this is needed or what this purpose is... if alot of time was "lost" due to a slow hardware fault, we should not throw away that time
        currentTime   = newTime;
        accumulator  += m_Misc.m_Dt;
        while (accumulator >= ENGINE_FIXED_TIMESTEP_VALUE_D) {
            updateSimulation(ENGINE_FIXED_TIMESTEP_VALUE);
            accumulator -= ENGINE_FIXED_TIMESTEP_VALUE_D;
        }
        renderSimulation();

        m_DebugManager.calculate();
        m_Misc.m_FPS.update(m_Misc.m_Dt);  
    }
}