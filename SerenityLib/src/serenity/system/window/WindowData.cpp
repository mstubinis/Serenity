
#include <serenity/system/window/WindowData.h>
#include <serenity/system/window/Window.h>
#include <serenity/system/EngineOptions.h>

#include <serenity/system/Engine.h>
#include <serenity/events/Event.h>

Engine::priv::WindowData::~WindowData() {
    internal_on_close();
}
void Engine::priv::WindowData::internal_on_close(bool skipRenderThisFrame) {
    m_UndergoingClosing = true;
    if (skipRenderThisFrame) {
        Engine::priv::PublicScene::SkipRenderThisFrame(*Engine::Resources::getCurrentScene());
    }
    sf::Event dummyEvent;
    while (m_SFMLWindow.pollEvent(dummyEvent)); //clear all queued events
    m_SFMLWindow.close();
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
    m_SFMLWindow.setFramerateLimit(m_FramerateLimit);
    m_SFContextSettings = m_SFMLWindow.getSettings();
    super.setMouseCursorVisible(m_Flags & Window_Flags::MouseVisible);
    super.setActive(m_Flags & Window_Flags::Active);
    super.setVerticalSyncEnabled(m_Flags & Window_Flags::Vsync);
    super.keepMouseInWindow(m_Flags & Window_Flags::MouseGrabbed);
    super.setKeyRepeatEnabled(m_Flags & Window_Flags::KeyRepeat);
}
void Engine::priv::WindowData::internal_init_position(Window& super) {
    auto winSize               = glm::vec2{ super.getSize() };
    auto desktopSize           = sf::VideoMode::getDesktopMode();
    float final_desktop_width  = float(desktopSize.width);
    float final_desktop_height = float(desktopSize.height);
    //float x_other              = 0.0f;
    float y_other              = 0.0f;
    #ifdef _WIN32
        //get the dimensions of the desktop's bottom task bar. Only tested on Windows 10
        //auto os_handle         = super.getSFMLHandle().getSystemHandle();
        //            left   right   top   bottom
        RECT rect; //[0,     1920,   0,    1040]  //bottom task bar
        SystemParametersInfoA(SPI_GETWORKAREA, 0, &rect, 0);
        y_other                = final_desktop_height - float(rect.bottom);
        final_desktop_height  -= y_other;
    #endif

    super.setPosition(uint32_t((final_desktop_width - winSize.x) / 2.0f), uint32_t((final_desktop_height - winSize.y) / 2.0f));
}
const sf::ContextSettings Engine::priv::WindowData::internal_create(Window& super, const std::string& windowTitle) {
    internal_on_close(false);
    std::latch ltch{ 1 };
    internal_thread_startup(super, windowTitle, &ltch); //calls window.setActive(false) on the created event thread, so we call setActive(true) below
    ltch.wait();
    super.setActive(true);
    super.m_Data.m_OpenGLThreadID = std::this_thread::get_id();
    return m_SFMLWindow.getSettings();
}
void Engine::priv::WindowData::internal_update_mouse_position(Window& super, float x, float y, bool resetDifference, bool resetPrevious) {
    const auto sfml_size     = m_SFMLWindow.getSize();
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
    m_SFMLWindow.requestFocus();
    super.setVisible(false);
    Engine::priv::Core::m_Engine->m_RenderModule._onFullscreen(m_VideoMode.width, m_VideoMode.height);

    const auto winSize = super.getSize();

    //this does not trigger the sfml event resize method automatically so we must call it here
    Engine::priv::Core::m_Engine->m_EngineEventHandler.internal_on_event_resize(super, winSize.x, winSize.y, false, *Engine::priv::Core::m_Engine->m_GameCore);

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
    const auto validModes = sf::VideoMode::getFullscreenModes();
    return validModes.size() > 0 ? validModes[0] : sf::VideoMode::getDesktopMode();
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
                m_SFMLWindow.setMouseCursorVisible(true);
                m_Flags.add(Window_Flags::MouseVisible);
                break;
            } case WindowEventThreadOnlyCommands::HideMouse: {
                m_SFMLWindow.setMouseCursorVisible(false);
                m_Flags.remove(Window_Flags::MouseVisible);
                break;
            } case WindowEventThreadOnlyCommands::RequestFocus: {
                m_SFMLWindow.requestFocus();
                break;
            } case WindowEventThreadOnlyCommands::KeepMouseInWindow: {
                m_SFMLWindow.setMouseCursorGrabbed(true);
                m_Flags.add(Window_Flags::MouseGrabbed);
                break;
            } case WindowEventThreadOnlyCommands::FreeMouseFromWindow: {
                m_SFMLWindow.setMouseCursorGrabbed(false);
                m_Flags.remove(Window_Flags::MouseGrabbed);
                break;
            }
        }
    }
}
void Engine::priv::WindowData::internal_populate_sf_event_queue(bool isUndergoingClosing) {
    sf::Event e;
    if (!isUndergoingClosing) {
        while (m_SFMLWindow.pollEvent(e)) {
            m_SFEventQueue.push(e);
        }
    }
}

Engine::priv::WindowData::WindowEventType Engine::priv::WindowData::internal_try_pop() noexcept {
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
    return m_SFEventQueue.try_pop();
#else
    const bool isUndergoingClosing = m_UndergoingClosing;
    if (!isUndergoingClosing) {
        internal_populate_sf_event_queue(isUndergoingClosing);
        internal_process_command_queue();
    }
    if (m_SFEventQueue.size() > 0) {
        auto x = m_SFEventQueue.front();
        m_SFEventQueue.pop();
        return x;
    }
    return WindowEventType{};
#endif
}

void Engine::priv::WindowData::internal_thread_startup(Window& super, const std::string& name, std::latch* ltch) {
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
    const bool isUndergoingClosing = m_UndergoingClosing;
    if (m_EventThread && !isUndergoingClosing) {
        m_UndergoingClosing = true;
        if (m_EventThread->joinable()) {
            m_EventThread->join();
        }
    }
#endif
    auto update_lambda_loop = [this, &super, &name, &ltch]() {
        m_SFMLWindow.create(m_VideoMode, name, m_Style, m_SFContextSettings);
        super.setIcon(m_IconFile);
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
        m_SFMLWindow.setActive(false);
#endif
        m_UndergoingClosing = false;
        if (ltch) {
            ltch->count_down();
        }
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
        while (!m_UndergoingClosing) {
            const bool isUndergoingClosing = m_UndergoingClosing;
            internal_populate_sf_event_queue(isUndergoingClosing);
            internal_process_command_queue();
        }
#endif
    };

#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
    m_EventThread.reset(NEW std::jthread{ std::move(update_lambda_loop) });
#else
    update_lambda_loop();
#endif
}
