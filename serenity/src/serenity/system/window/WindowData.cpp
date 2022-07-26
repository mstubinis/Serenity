
#include <serenity/system/window/WindowData.h>
#include <serenity/system/window/Window.h>
#include <serenity/system/EngineOptions.h>

#include <serenity/system/Engine.h>
#include <serenity/events/Event.h>

namespace {

}

Engine::priv::WindowData::WindowData(sf::ContextSettings ctxSettings, sf::ContextSettings::Attribute attr, int bitsPerPixel)
    : m_SFContextSettings{ std::move(ctxSettings) }
{
    m_SFContextSettings.attributeFlags = attr;
    m_VideoMode.bitsPerPixel           = bitsPerPixel;
}
Engine::priv::WindowData::WindowData(WindowData&& other) noexcept 
    : m_SFContextSettings{ std::move(other.m_SFContextSettings) }
    , m_MousePosition{ std::move(other.m_MousePosition) }
    , m_MousePosition_Previous{ std::move(other.m_MousePosition_Previous) }
    , m_MouseDifference{ std::move(other.m_MouseDifference) }
    , m_OldWindowSize{ std::move(other.m_OldWindowSize) }
    , m_VideoMode{ std::move(other.m_VideoMode) }
    , m_IconFile{ std::move(other.m_IconFile) }
    , m_WindowTitle{ std::move(other.m_WindowTitle) }
    , m_SFMLWindow{ std::move(other.m_SFMLWindow) }
    , m_MouseDelta{ std::move(other.m_MouseDelta) }
    , m_Style{ std::move(other.m_Style) }
    , m_FramerateLimit{ std::move(other.m_FramerateLimit) }
    , m_Flags{ std::move(other.m_Flags) }
    , m_OpenGLThreadID{ std::move(other.m_OpenGLThreadID) }
    , m_UndergoingClosing{ other.m_UndergoingClosing.load() }
    , m_SFEventQueue{ std::move(other.m_SFEventQueue) }
    , m_MainThreadToEventThreadQueue{ std::move(other.m_MainThreadToEventThreadQueue) }
    , m_EventThread{ std::move(other.m_EventThread) }
{
}
Engine::priv::WindowData& Engine::priv::WindowData::operator=(WindowData&& other) noexcept {
    if (this != &other) {
        m_SFContextSettings            = std::move(other.m_SFContextSettings);
        m_MousePosition                = std::move(other.m_MousePosition);
        m_MousePosition_Previous       = std::move(other.m_MousePosition_Previous);
        m_MouseDifference              = std::move(other.m_MouseDifference);
        m_OldWindowSize                = std::move(other.m_OldWindowSize);
        m_VideoMode                    = std::move(other.m_VideoMode);
        m_IconFile                     = std::move(other.m_IconFile);
        m_WindowTitle                  = std::move(other.m_WindowTitle);
        m_SFMLWindow                   = std::move(other.m_SFMLWindow);
        m_MouseDelta                   = std::move(other.m_MouseDelta);
        m_Style                        = std::move(other.m_Style);
        m_FramerateLimit               = std::move(other.m_FramerateLimit);
        m_Flags                        = std::move(other.m_Flags);
        m_OpenGLThreadID               = std::move(other.m_OpenGLThreadID);
        m_UndergoingClosing            = other.m_UndergoingClosing.load();
        m_SFEventQueue                 = std::move(other.m_SFEventQueue);
        m_MainThreadToEventThreadQueue = std::move(other.m_MainThreadToEventThreadQueue);
        m_EventThread                  = std::move(other.m_EventThread);
    }
    return *this;
}
Engine::priv::WindowData::~WindowData() {
    internal_on_close(true);
}
void Engine::priv::WindowData::internal_on_close(bool useLatch) {
    m_UndergoingClosing = true;
    if (useLatch) {
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
        m_Latch.wait();
#endif
    }
    //sf::Event dummyEvent;
    //while (m_SFMLWindow->pollEvent(dummyEvent)); //clear all queued events
    m_SFMLWindow->close();
}
void Engine::priv::WindowData::internal_on_resize(uint32_t newWindowWidth, uint32_t newWindowHeight, bool saveSize) {
    if (saveSize) {
        m_VideoMode.width  = newWindowWidth;
        m_VideoMode.height = newWindowHeight;
    }
}
void Engine::priv::WindowData::internal_on_mouse_wheel_scrolled(float delta, int x, int y) {
    m_MouseDelta += double(delta) * 10.0;
}
void Engine::priv::WindowData::internal_restore_state(Window& super) {
    m_SFMLWindow->setFramerateLimit(m_FramerateLimit);
    m_SFContextSettings = m_SFMLWindow->getSettings();
    super.setMouseCursorVisible(m_Flags & Window_Flags::MouseVisible);
    super.setActive(m_Flags & Window_Flags::Active);
    super.setVerticalSyncEnabled(m_Flags & Window_Flags::Vsync);
    super.keepMouseInWindow(m_Flags & Window_Flags::MouseGrabbed);
    super.setKeyRepeatEnabled(m_Flags & Window_Flags::KeyRepeat);
}
const sf::ContextSettings Engine::priv::WindowData::internal_create(Window& super, const std::string& windowTitle) {
    internal_on_close(false);
    std::latch waitForContext{ 1 };
    internal_thread_startup(super, windowTitle, &waitForContext); //calls window.setActive(false) on the created event thread, so we call setActive(true) below
    waitForContext.wait();
    super.setActive(true);
    super.m_Data.m_OpenGLThreadID = std::this_thread::get_id();
    return m_SFMLWindow->getSettings();
}
void Engine::priv::WindowData::internal_update_mouse_position(Window& super, float x, float y, bool resetDifference, bool resetPrevious) {
    const auto sfml_size     = m_SFMLWindow->getSize();
    const auto winSize       = glm::vec2{ sfml_size.x, sfml_size.y };
    const glm::vec2 newPos   = glm::vec2{ x, winSize.y - y }; //opengl flipping y axis
    m_MousePosition_Previous = resetPrevious ? newPos : m_MousePosition;
    m_MousePosition          = newPos;
    m_MouseDifference       += (m_MousePosition - m_MousePosition_Previous);
    if (resetDifference) {
        m_MouseDifference    = glm::vec2{ 0.0f };
    }
}
void Engine::priv::WindowData::internal_on_fullscreen(Window& super, bool isToBeFullscreen, bool isMaximized, bool isMinimized) {
    if (isToBeFullscreen) {
        m_OldWindowSize    = glm::uvec2{ m_VideoMode.width, m_VideoMode.height };
        m_VideoMode        = internal_get_default_desktop_video_mode();
    } else {
        m_VideoMode.width  = m_OldWindowSize.x;
        m_VideoMode.height = m_OldWindowSize.y;
    }
    internal_create(super, m_WindowTitle);
    m_SFMLWindow->requestFocus();
    super.setVisible(false);
    Engine::getRenderer()._onFullscreen(m_VideoMode.width, m_VideoMode.height);

    const auto winSize = super.getSize();

    //this does not trigger the sfml event resize method automatically so we must call it here
    Engine::priv::Core::m_Engine->m_Modules->m_EngineEventHandler.internal_on_event_resize(super, winSize.x, winSize.y, false, *Engine::priv::Core::m_Engine->m_GameCore);

    internal_restore_state(super);
    if (isMaximized) {
        super.maximize();
    } 
    super.setVisible(true);

    Event ev{ ::EventType::WindowFullscreenChanged };
    ev.eventWindowFullscreenChanged = Engine::priv::EventWindowFullscreenChanged{ isToBeFullscreen };
    Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
sf::VideoMode Engine::priv::WindowData::internal_get_default_desktop_video_mode() {
    return sf::VideoMode::getFullscreenModes().size() > 0 ? sf::VideoMode::getFullscreenModes()[0] : sf::VideoMode::getDesktopMode();
}
void Engine::priv::WindowData::internal_update_on_reset_events(const float dt) {
    m_MouseDifference   = glm::vec2{ 0.0f };
    const double step   = 1.0 - double(dt);
    m_MouseDelta       *= (step * step * step);
}




//window thread
void Engine::priv::WindowData::internal_push(WindowEventThreadOnlyCommands command) {
    m_MainThreadToEventThreadQueue.push(command);
}
void Engine::priv::WindowData::internal_process_command_queue() {
    while (!m_MainThreadToEventThreadQueue.empty()) {
        auto command_ptr = m_MainThreadToEventThreadQueue.front();
        m_MainThreadToEventThreadQueue.pop();
        switch (command_ptr) {
            case WindowEventThreadOnlyCommands::ShowMouse: {
                m_SFMLWindow->setMouseCursorVisible(true);
                m_Flags.add(Window_Flags::MouseVisible);
                break;
            } case WindowEventThreadOnlyCommands::HideMouse: {
                m_SFMLWindow->setMouseCursorVisible(false);
                m_Flags.remove(Window_Flags::MouseVisible);
                break;
            } case WindowEventThreadOnlyCommands::RequestFocus: {
                m_SFMLWindow->requestFocus();
                break;
            } case WindowEventThreadOnlyCommands::KeepMouseInWindow: {
                m_SFMLWindow->setMouseCursorGrabbed(true);
                m_Flags.add(Window_Flags::MouseGrabbed);
                break;
            } case WindowEventThreadOnlyCommands::FreeMouseFromWindow: {
                m_SFMLWindow->setMouseCursorGrabbed(false);
                m_Flags.remove(Window_Flags::MouseGrabbed);
                break;
            }
        }
    }
}
void Engine::priv::WindowData::internal_populate_sf_event_queue() {
    if (!m_UndergoingClosing) {
        sf::Event e;
        while (m_SFMLWindow->pollEvent(e)) {
            m_SFEventQueue.push(e);
        }
    }
}

Engine::priv::WindowData::WindowEventType Engine::priv::WindowData::internal_try_pop() noexcept {
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
    return m_SFEventQueue.try_pop(); //populated off-thread
#else
    if (!m_UndergoingClosing) {
        internal_populate_sf_event_queue();
        internal_process_command_queue();
    }
    if (m_SFEventQueue.size() > 0) {
        auto x = m_SFEventQueue.front();
        m_SFEventQueue.pop();
        return x;
    }
    return {};
#endif
}

void Engine::priv::WindowData::internal_thread_startup(Window& super, const std::string& name, std::latch* waitForContext) {
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
    if (m_EventThread && !m_UndergoingClosing) {
        m_UndergoingClosing = true;
        if (m_EventThread->joinable()) {
            m_EventThread->join();
        }
    }
#endif
    auto thread_event_loop = [this, &super, &name, &waitForContext]() {
        m_SFMLWindow->create(m_VideoMode, name, m_Style, m_SFContextSettings);
        super.setIcon(m_IconFile);
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
        m_SFMLWindow->setActive(false);
#endif
        m_UndergoingClosing = false;
        if (waitForContext) {
            waitForContext->count_down();
        }
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
        while (!m_UndergoingClosing) {
            internal_populate_sf_event_queue();
            internal_process_command_queue();
        }
        m_Latch.count_down();
#endif
    };

#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
    m_EventThread = std::make_unique<std::jthread>(std::move(thread_event_loop));
    //m_EventThread.reset(NEW std::jthread{ std::move(thread_event_loop) });
#else
    thread_event_loop();
#endif
}
