
#include <serenity/system/Engine.h>
#include <serenity/system/EngineOptions.h>
#include <serenity/system/EngineGameFunctions.h>
#include <serenity/system/Time.h>
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


#include <serenity/renderer/opengl/OpenGLContext.h>
#include <serenity/renderer/direct3d/DirectXContext.h>

namespace {
    void internal_cleanup_os_specific() {
        /*
        #ifdef _WIN32
            if (GetConsoleWindow() != NULL) {
                FreeConsole(); //erroring out for some reason
            }
        #endif
        */
    }
    void internal_init_os_specific(const EngineOptions& options) {
        //get args from shortcut, etc
        std::unordered_set<std::string> args;
        std::locale loc;
        args.reserve(options.argc);
        for (int i = 0; i < options.argc; i++) {
            const std::string key = std::string(options.argv[i]);
            std::string lowerKey;
            lowerKey.reserve(key.length());
            for (size_t j = 0; j < key.length(); ++j) {
                lowerKey += std::tolower(key[j], loc);
            }
            args.insert(std::move(lowerKey));
    }
#ifdef _WIN32
        //show console if applicable
        if (options.show_console) {
            if (GetConsoleWindow() == NULL) {
                AllocConsole();
            }
            ShowWindow(GetConsoleWindow(), SW_SHOW); //show console window
        } else if (!args.contains("console")) {
            ShowWindow(GetConsoleWindow(), SW_HIDE); //hide console window
        }
#endif
}
}

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
    void Game::onWindowClosed(Window& window) {}
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



Engine::priv::EngineRenderingContexts::EngineRenderingContexts(const EngineOptions& options) {
    switch (options.renderingAPI) {
        case RenderingAPI::OpenGL: {
            auto glContext = m_APIContexts.emplace_back(std::make_unique<Engine::priv::OpenGLContext>()).get();
            break;
        }
        case RenderingAPI::DirectX: {
            auto dXContext = m_APIContexts.emplace_back(std::make_unique<Engine::priv::DirectXContext>()).get();
            break;
        }
        case RenderingAPI::Vulkan: {
            break;
        }
        case RenderingAPI::Metal: {
            break;
        }
    }
}


Engine::priv::EngineWindows::EngineWindows(const EngineOptions& options) {
    auto& window = m_WindowsVector.emplace_back(options);
}
Engine::priv::EngineWindows::~EngineWindows() {
}




Engine::priv::EngineCore::Modules::Modules(const EngineOptions& options, EventDispatcher& eventDispatcher, ResourceManager& resourceManager, InputModule& inputModule)
    : m_RenderModule{ options, Engine::getWindowSize().x, Engine::getWindowSize().y }
    , m_Editor{ options, Engine::getWindow() }
    , m_EngineEventHandler{ m_Editor, inputModule, m_RenderModule, resourceManager, eventDispatcher }
{

}



Engine::priv::EngineCore::EngineCore(const EngineOptions& options, Window& window)
    : m_RenderingAPIManager{ options, window }
    , m_ResourceManager{ options }
{ 
    std::srand(static_cast<uint32_t>(std::time(nullptr)));
    m_Misc.m_MainThreadID = std::this_thread::get_id();
}
Engine::priv::EngineCore::~EngineCore() {
    internal_cleanup_os_specific();
}
void Engine::priv::EngineCore::init(const EngineOptions& options, Engine::view_ptr<GameCore> gameCore) {
    m_GameCore = gameCore;
    internal_init_os_specific(options);
    m_Modules  = std::make_unique<Modules>(options, m_EventDispatcher, m_ResourceManager, m_InputModule);
    m_Misc.m_BuiltInMeshes.init();
    m_Modules->m_RenderModule.init(Engine::getWindowSize().x, Engine::getWindowSize().y);
    Game::initResources(options, *m_GameCore);
    Game::initLogic(options, *m_GameCore);
    if (m_ResourceManager.m_Scenes.empty()) {
        Scene& defaultScene = Engine::Resources::addScene<Scene>("Default");
        defaultScene.addCamera<Camera>(60.0f, float(options.width) / float(options.height), 0.01f, 1000.0f);
        Engine::Resources::setCurrentScene(std::addressof(defaultScene));
    }
}
void Engine::priv::EngineCore::internal_post_input_update(int frameIteration) {
    m_InputModule.m_KeyboardModule.postUpdate();
    m_InputModule.m_MouseModule.postUpdate();
}
void Engine::priv::EngineCore::internal_update_logic(GameCore& gameCore, int frameIteration, Scene& scene, const float dt) {
#ifndef ENGINE_PRODUCTION
    const auto start = Engine::time::now();
#endif
    m_NetworkingModule.update(dt);

    scene.update(dt);

    m_ThreadingModule.update(dt);

    m_DiscordModule.update();

    Game::onPostUpdate(dt, gameCore);
    scene.postUpdate(dt);
#ifndef ENGINE_PRODUCTION
    m_Modules->m_DebugManager.calculate(DebugTimerTypes::Logic, Engine::time::difference_as_nanoseconds(start));
#endif
}
void Engine::priv::EngineCore::internal_update_sounds(GameCore& gameCore, Scene& scene, const float dt) {
#ifndef ENGINE_PRODUCTION
    const auto start = Engine::time::now();
#endif
    m_SoundModule.update(scene, dt);
#ifndef ENGINE_PRODUCTION
    m_Modules->m_DebugManager.calculate(DebugTimerTypes::Sound, Engine::time::difference_as_nanoseconds(start));
#endif
}
void Engine::priv::EngineCore::internal_pre_update(GameCore& gameCore, int frameIteration, Scene& scene, const float dt) {
    Game::onPreUpdate(dt, gameCore);
    scene.preUpdate(dt);
}
void Engine::priv::EngineCore::internal_post_update(Scene& scene, Window& window, const float dt) {
    m_EventDispatcher.postUpdate();
    window.m_Data.internal_update_on_reset_events(dt);
}
void Engine::priv::EngineCore::internal_render(GameCore& gameCore, Scene& scene, Window& window, const float dt, const double alpha) {
#ifndef ENGINE_PRODUCTION
    const auto start = Engine::time::now();
#endif
    Game::render(gameCore);
    if (!scene.m_SkipRenderThisFrame) {
        m_Modules->m_RenderModule._sort2DAPICommands();
        m_Modules->m_RenderModule.m_Pipeline->renderInitFrame(m_Modules->m_RenderModule);
        scene.render();
        auto& scene_viewports = Engine::priv::PublicScene::GetViewports(scene);
        for (auto& viewport : scene_viewports) {
            if (viewport.isActive()) {
                if (viewport.isAspectRatioSynced()) {
                    const auto& viewportDimensions = viewport.getViewportDimensions();
                    viewport.getCamera().setAspectRatio(viewportDimensions.z / viewportDimensions.w);
                }
                m_Modules->m_Editor.renderLightIcons(scene, viewport);
                Engine::priv::ViewportPriv::render(m_Modules->m_RenderModule, viewport, true);
            }
        }
        m_Modules->m_Editor.render(window);
        window.display();
    }
    m_Modules->m_RenderModule._clear2DAPICommands();
    m_Misc.m_FPS.incrementFPSCount();
#ifndef ENGINE_PRODUCTION
    m_Modules->m_DebugManager.calculate(DebugTimerTypes::Render, Engine::time::difference_as_nanoseconds(start));
#endif
}
void Engine::priv::EngineCore::internal_cleanup() {
    m_ResourceManager.postUpdate();
}
void Engine::priv::EngineCore::run(const EngineOptions& options, Engine::view_ptr<GameCore> gameCore) {
    init(options, gameCore);


    double accumulator = ENGINE_FIXED_TIMESTEP_VALUE_D;
    auto   currentTime = Engine::time::now();

    auto   updateOncePerFrame = [this](const float timeElasped, Scene& scene) {
        for (auto& window : *Engine::priv::Core::m_Windows) {
            internal_post_update(scene, window, timeElasped);
            m_Modules->m_EngineEventHandler.poll_events(window, *m_GameCore);
            m_Modules->m_Editor.update(window, timeElasped);
            window.internal_update_dynamic_resize();
        }
    };
    auto   updateSimulation = [this](int& frameIteration, const float timeElasped, Scene& scene) {
        m_Misc.m_DtSimulation = timeElasped;
        internal_pre_update(*m_GameCore, frameIteration, scene, timeElasped);
        internal_update_logic(*m_GameCore, frameIteration, scene, timeElasped);
        internal_post_input_update(frameIteration);
        ++frameIteration;
    };
    auto   renderSimulation = [this, accumulator](Scene& scene) {
        const double alpha = accumulator / ENGINE_FIXED_TIMESTEP_VALUE_D;
        //State state = currentState * alpha + previousState * (1.0 - alpha);
        for (auto& window : *Engine::priv::Core::m_Windows) {
            internal_render(*m_GameCore, scene, window, float(m_Misc.m_Dt), alpha/*, state*/);
        }
    };
    int frameIteration = 0;
    updateSimulation(frameIteration, ENGINE_FIXED_TIMESTEP_VALUE, *Engine::Resources::getCurrentScene()); //initially call update once to get user code in place before the first render.
    while (!m_Misc.m_Destroyed) {
        const auto newTime = Engine::time::now();
        std::chrono::duration<double> duration = newTime - currentTime;
        const double dt = std::min(duration.count(), 0.25); //not sure if this is needed or what this purpose is... if alot of time was "lost" due to a slow hardware fault, we should not throw away that time
        const float dtF = float(dt);
        m_Misc.m_Dt = dt;
        currentTime = newTime;
        accumulator += dt;
        auto& scene = *Engine::Resources::getCurrentScene();
        scene.m_SkipRenderThisFrame = false;
#ifndef ENGINE_PRODUCTION
        m_Modules->m_DebugManager.reset_timers();
#endif
        frameIteration = 0;
        while (accumulator >= ENGINE_FIXED_TIMESTEP_VALUE_D) {
            if (frameIteration == 0) {
                updateOncePerFrame(dtF, scene);
            }
            updateSimulation(frameIteration, ENGINE_FIXED_TIMESTEP_VALUE, scene);
            accumulator -= ENGINE_FIXED_TIMESTEP_VALUE_D;
        }
        if (frameIteration > 0) {
            Engine::priv::PublicScene::UpdateParticles(scene, dtF);
            internal_update_sounds(*m_GameCore, scene, dtF);
        }
        if (!m_Misc.m_Destroyed) {
            renderSimulation(scene);
        }

        internal_cleanup();
#ifndef ENGINE_PRODUCTION
        m_Modules->m_DebugManager.calculate();
#endif
        m_Misc.m_FPS.update(dt);
    }
    m_Modules->m_EngineEventHandler.internal_on_event_game_ended();
}








Window& Engine::getWindow() {
    return Engine::getWindow(0);
}
glm::uvec2 Engine::getWindowSize() {
    return Engine::getWindowSize(0);
}
Window& Engine::getWindow(uint32_t index) {
    return (*Engine::priv::Core::m_Windows)[index];
}
glm::uvec2 Engine::getWindowSize(uint32_t index) {
    return Engine::getWindow(index).getSize();
}
size_t Engine::getNumWindows() {
    return Engine::priv::Core::m_Windows->size();
}


bool Engine::paused() noexcept {
    return Engine::priv::Core::m_Engine->m_Misc.m_Paused;
}
void Engine::pause(bool b) noexcept {
    Engine::Physics::pause(b);
    Engine::priv::Core::m_Engine->m_Misc.m_Paused = b;
}
void Engine::unpause() noexcept {
    Engine::Physics::unpause();
    Engine::priv::Core::m_Engine->m_Misc.m_Paused = false;
}
void Engine::setTimeScale(float timeScale) noexcept {
    Engine::priv::Core::m_Engine->m_Modules->m_DebugManager.setTimeScale(timeScale);
}
double Engine::getFPS() noexcept { 
    return 1.0 / Engine::priv::Core::m_Engine->m_Misc.m_Dt;
}
void Engine::stop() noexcept {
    Engine::priv::threading::waitForAll();
    Engine::priv::Core::m_Engine->m_Misc.m_Destroyed = true;
}
