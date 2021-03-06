#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/system/window/WindowData.h>
#include <core/engine/system/window/Window.h>

#include <core/engine/system/Engine.h>
#include <core/engine/events/Event.h>

Engine::priv::WindowData::WindowData()
#ifdef ENGINE_THREAD_WINDOW_EVENTS
    : m_WindowThread{ *this }
#endif
{
    m_Flags = (Window_Flags::Windowed | Window_Flags::MouseVisible);
}
Engine::priv::WindowData::~WindowData() {
    internal_on_close();
}
void Engine::priv::WindowData::internal_on_close() {
    m_UndergoingClosing = true;

    m_SFMLWindow.setVisible(false);
    m_SFMLWindow.close();

    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        m_WindowThread.internal_cleanup();
    #endif
}
void Engine::priv::WindowData::internal_on_mouse_wheel_scrolled(float delta, int x, int y) {
    m_MouseDelta += ((double)delta * 10.0);
}
void Engine::priv::WindowData::internal_restore_state(Window& super) {
    if (m_FramerateLimit > 0) {
        m_SFMLWindow.setFramerateLimit(m_FramerateLimit);
    }
    m_SFContextSettings = m_SFMLWindow.getSettings();
    m_SFMLWindow.setMouseCursorVisible(m_Flags & Window_Flags::MouseVisible);
    super.setActive(m_Flags & Window_Flags::Active);
    m_SFMLWindow.setVerticalSyncEnabled(m_Flags & Window_Flags::Vsync);
    m_SFMLWindow.setMouseCursorGrabbed(m_Flags & Window_Flags::MouseGrabbed);
}
void Engine::priv::WindowData::internal_init_position(Window& super) {
    auto winSize               = glm::vec2(super.getSize());
    auto desktopSize           = sf::VideoMode::getDesktopMode();
    float final_desktop_width  = (float)desktopSize.width;
    float final_desktop_height = (float)desktopSize.height;
    float x_other              = 0.0f;
    float y_other              = 0.0f;
    #ifdef _WIN32
        //get the dimensions of the desktop's bottom task bar. Only tested on Windows 10
        auto os_handle             = super.getSFMLHandle().getSystemHandle();
        //            left   right   top   bottom
        RECT rect; //[0,     1920,   0,    1040]  //bottom task bar
        SystemParametersInfoA(SPI_GETWORKAREA, 0, &rect, 0);
        y_other               = final_desktop_height - (float)rect.bottom;
        final_desktop_height -= y_other;
    #endif

    super.setPosition((unsigned int)((final_desktop_width - winSize.x) / 2.0f), (unsigned int)((final_desktop_height - winSize.y) / 2.0f));
}
const sf::ContextSettings Engine::priv::WindowData::internal_create(Window& super, const std::string& name) {
    internal_on_close();
    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        boost::latch bLatch{ 1 }; //TODO: replace with std::latch once it is avaiable for c++20 msvc compiler
        m_WindowThread.internal_startup(super, name, &bLatch); //calls window.setActive(false) on the created event thread, so we call setActive(true) below
        bLatch.wait();
        super.setActive(true);
    #else
        m_SFMLWindow.create(m_VideoMode, name, m_Style, m_SFContextSettings);
        super.m_Data.m_OpenGLThreadID = std::this_thread::get_id();
        super.setIcon(m_IconFile);
        m_UndergoingClosing = false;
    #endif
    return m_SFMLWindow.getSettings();
}
void Engine::priv::WindowData::internal_update_mouse_position(Window& super, float x, float y, bool resetDifference, bool resetPrevious) {
    auto sfml_size           = m_SFMLWindow.getSize();
    auto winSize             = glm::vec2(sfml_size.x, sfml_size.y);
    glm::vec2 newPos         = glm::vec2(x, winSize.y - y); //opengl flipping y axis
    m_MousePosition_Previous = (resetPrevious) ? newPos : m_MousePosition;
    m_MousePosition          = newPos;
    m_MouseDifference       += (m_MousePosition - m_MousePosition_Previous);
    if (resetDifference) {
        m_MouseDifference    = glm::vec2(0.0f);
    }
}
void Engine::priv::WindowData::internal_on_fullscreen(Window& super, bool isToBeFullscreen, bool isMaximized, bool isMinimized) {
    if (isToBeFullscreen) {
        m_OldWindowSize    = glm::uvec2(m_VideoMode.width, m_VideoMode.height);
        m_VideoMode        = internal_get_default_desktop_video_mode();
    }else{
        m_VideoMode.width  = m_OldWindowSize.x;
        m_VideoMode.height = m_OldWindowSize.y;
    }
    internal_create(super, m_WindowName);
    m_SFMLWindow.requestFocus();
    Engine::priv::Core::m_Engine->m_RenderModule._onFullscreen(m_VideoMode.width, m_VideoMode.height);

    auto sfml_size = m_SFMLWindow.getSize();
    auto winSize   = glm::uvec2(sfml_size.x, sfml_size.y);

    //this does not trigger the sfml event resize method automatically so we must call it here
    Engine::priv::Core::m_Engine->m_EngineEventHandler.internal_on_event_resize(super, winSize.x, winSize.y, false);

    internal_restore_state(super);
    //TODO: very wierd, but there is an after-effect "reflection" of the last frame on the window if maximize() is called. Commenting out until it is fixed
    /*
    if (isMaximized) {
        maximize();
    }else if (isMinimized) {
        minimize();
    }
    */

    //event dispatch
    Event ev(EventType::WindowFullscreenChanged);
    ev.eventWindowFullscreenChanged = Engine::priv::EventWindowFullscreenChanged(isToBeFullscreen);
    Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
}
sf::VideoMode Engine::priv::WindowData::internal_get_default_desktop_video_mode() {
    auto validModes = sf::VideoMode::getFullscreenModes();
    return (validModes.size() > 0) ? validModes[0] : sf::VideoMode::getDesktopMode();
}
void Engine::priv::WindowData::internal_on_reset_events(const float dt) {
    m_MouseDifference.x = 0.0f;
    m_MouseDifference.y = 0.0f;
    double step         = (1.0 - dt);
    m_MouseDelta       *= (step * step * step);
}
