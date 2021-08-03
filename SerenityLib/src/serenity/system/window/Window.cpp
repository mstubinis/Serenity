
#include <serenity/system/Engine.h>
#include <serenity/system/EngineOptions.h>
#include <serenity/renderer/opengl/OpenGL.h>
#include <serenity/system/window/Window.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/system/cursor/Cursor.h>

Window::Window() {
    m_Data.m_SFContextSettings = sf::ContextSettings{ 24, 0, 0, 4, 6, 0, false };

    #ifdef _DEBUG
        m_Data.m_SFContextSettings.attributeFlags = m_Data.m_SFContextSettings.Debug;
    #else
        m_Data.m_SFContextSettings.attributeFlags = m_Data.m_SFContextSettings.Core;
    #endif

    m_Data.m_VideoMode.bitsPerPixel = 32;
    m_Data.m_Style = sf::Style::Default;
}
void Window::init(const EngineOptions& options) noexcept {
    m_Data.m_WindowName        = options.window_title;
    m_Data.m_VideoMode         = sf::VideoMode(options.width, options.height, m_Data.m_VideoMode.bitsPerPixel);
    m_Data.m_SFContextSettings = m_Data.internal_create(*this, options.window_title);
    int opengl_version         = std::stoi(std::to_string(m_Data.m_SFContextSettings.majorVersion) + std::to_string(m_Data.m_SFContextSettings.minorVersion));
    Engine::priv::Core::m_Engine->m_RenderModule._onOpenGLContextCreation(m_Data.m_VideoMode.width, m_Data.m_VideoMode.height);

    m_Data.internal_init_position(*this);
    setVisible(false);
    setIcon(options.icon);
    internal_set_fullscreen_from_options(options);
    internal_set_minimized_or_maximized_from_options(options);
    requestFocus();
    setVisible(true);
    display();
    setVerticalSyncEnabled(options.vsync);
    if (options.show_console) {
        ENGINE_PRODUCTION_LOG("Using OpenGL: " << m_Data.m_SFContextSettings.majorVersion << "." << m_Data.m_SFContextSettings.minorVersion <<
            ", with depth bits: " << m_Data.m_SFContextSettings.depthBits << " and stencil bits: " << m_Data.m_SFContextSettings.stencilBits
        )
    }
}
void Window::internal_set_minimized_or_maximized_from_options(const EngineOptions& options) {
    if (options.maximized) {
        maximize();
    }
}
void Window::internal_set_fullscreen_from_options(const EngineOptions& options) {
    switch (options.window_mode) {
        case 1: {
            setFullscreen(true);
            break;
        } case 2: {
            setFullscreenWindowed(true);
            break;
        } default: {
            setFullscreen(false);
            break;
        }
    }
}
void Window::setMouseCursor(const Cursor& cursor) noexcept { m_Data.m_SFMLWindow.setMouseCursor(cursor.getSFMLCursor()); }
void Window::updateMousePosition(float x, float y, bool resetDifference, bool resetPrevious) {
    m_Data.internal_update_mouse_position(*this, x, y, resetDifference, resetPrevious);
}
void Window::updateMousePosition(const glm::vec2& position, bool resetDifference, bool resetPrevious) {
    m_Data.internal_update_mouse_position(*this, position.x, position.y, resetDifference, resetPrevious);
}
bool Window::internal_execute_show_window(uint32_t cmd) noexcept {
    #ifdef _WIN32
        ::ShowWindow(m_Data.m_SFMLWindow.getSystemHandle(), cmd);
        return true;
    #endif
    return false;
}
void Window::internal_restore_state() { m_Data.internal_restore_state(*this); }
void Window::internal_update_dynamic_resize() {
    #ifdef _WIN32
        WINDOWINFO wiInfo;
        GetWindowInfo(m_Data.m_SFMLWindow.getSystemHandle(), &wiInfo);
        const glm::uvec2 current_size = glm::uvec2{ wiInfo.rcClient.right - wiInfo.rcClient.left, wiInfo.rcClient.bottom - wiInfo.rcClient.top };
        const glm::uvec2 old_size = getSize();

        if (current_size.x != old_size.x || current_size.y != old_size.y) {
            Window::setSize(current_size.x, current_size.y);
            Engine::priv::Core::m_Engine->m_EngineEventHandler.internal_on_event_resize(*this, current_size.x, current_size.y, true);
        }
    #endif
}
bool Window::pollEvents(sf::Event& e) {
    auto x = m_Data.m_WindowThread.internal_try_pop();
    if (x) {
        e = std::move(*x);
    }
    return static_cast<bool>(x);
}
bool Window::maximize() noexcept { 
    bool result = false;
#ifdef _WIN32
    result = internal_execute_show_window(SW_MAXIMIZE);
#endif
    return result;
}
bool Window::minimize() noexcept { 
    bool result = false;
#ifdef _WIN32
    result = internal_execute_show_window(SW_MINIMIZE);
#endif
    return result;
}
void Window::setPosition(uint32_t x, uint32_t y) { m_Data.m_SFMLWindow.setPosition(sf::Vector2i(x, y)); }
glm::uvec2 Window::getPosition() {
    const auto position = m_Data.m_SFMLWindow.getPosition();
    return glm::uvec2{ position.x, position.y };
}
glm::uvec2 Window::getSize(){
    const auto windowSize = m_Data.m_SFMLWindow.getSize();
    return glm::uvec2{ windowSize.x, windowSize.y };
}
void Window::setIcon(const Texture& texture) {
    m_Data.m_SFMLWindow.setIcon(texture.width(), texture.height(), const_cast<Texture&>(texture).pixels());
    m_Data.m_IconFile = texture.name();
}
void Window::setIcon(const char* file) {
    auto texture = Engine::Resources::getResource<Texture>(file);
    if (!texture.m_Resource) {
        texture.m_Handle   = Engine::Resources::loadTexture(file, ImageInternalFormat::SRGB8_ALPHA8, false);
        texture.m_Resource = texture.m_Handle.get<Texture>();
    }
    m_Data.m_SFMLWindow.setIcon(texture.m_Resource->width(), texture.m_Resource->height(), texture.m_Resource->pixels());
    m_Data.m_IconFile = file;
}
void Window::setIcon(const std::string& file) {
    if (!file.empty()) {
        Window::setIcon(file.c_str());
    }
}
void Window::setName(const char* name) {
    if (m_Data.m_WindowName == name) {
        return;
    }
    m_Data.m_WindowName = name;
    m_Data.m_SFMLWindow.setTitle(name);
}
void Window::setVerticalSyncEnabled(bool isToBeEnabled) {
    m_Data.m_SFMLWindow.setVerticalSyncEnabled(isToBeEnabled);
    (isToBeEnabled) ? m_Data.m_Flags.add(Window_Flags::Vsync) : m_Data.m_Flags.remove(Window_Flags::Vsync);
}
bool Window::isVsyncEnabled() const { return m_Data.m_Flags.has(Window_Flags::Vsync); }
void Window::setKeyRepeatEnabled(bool isToBeEnabled) {
    m_Data.m_SFMLWindow.setKeyRepeatEnabled(isToBeEnabled);
    (isToBeEnabled) ? m_Data.m_Flags.add(Window_Flags::KeyRepeat) : m_Data.m_Flags.remove(Window_Flags::KeyRepeat);
}
void Window::setMouseCursorVisible(bool isToBeVisible) {
    (isToBeVisible) ? m_Data.m_WindowThread.internal_push(WindowEventThreadOnlyCommands::ShowMouse) : m_Data.m_WindowThread.internal_push(WindowEventThreadOnlyCommands::HideMouse);
}
void Window::requestFocus() { m_Data.m_WindowThread.internal_push(WindowEventThreadOnlyCommands::RequestFocus); }
void Window::close() {
    Engine::priv::Core::m_Engine->m_EngineEventHandler.internal_on_event_window_closed(*this);
    m_Data.m_SFMLWindow.close();
}
bool Window::isWindowOnSeparateThread() const { return m_Data.m_WindowThread.isUsingSeparateThread(); }
bool Window::hasFocus() const { return m_Data.m_SFMLWindow.hasFocus(); }
bool Window::isOpen() const { return m_Data.m_SFMLWindow.isOpen(); }
bool Window::isActive() const { return m_Data.m_Flags.has(Window_Flags::Active); }
bool Window::isFullscreen() const { return isFullscreenNonWindowed() || isFullscreenWindowed(); }
bool Window::isFullscreenWindowed() const { return m_Data.m_Flags.has(Window_Flags::WindowedFullscreen); }
bool Window::isFullscreenNonWindowed() const { return m_Data.m_Flags.has(Window_Flags::Fullscreen); }
bool Window::isMouseKeptInWindow() const { return m_Data.m_Flags.has(Window_Flags::MouseGrabbed); }
void Window::display() { m_Data.m_SFMLWindow.display(); }
bool Window::internal_return_window_placement_cmd(uint32_t cmd) const noexcept {
    #ifdef _WIN32
        WINDOWPLACEMENT info;
        info.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(getSystemHandle(), &info);
        if (info.showCmd == cmd) {
            return true;
        }
    #endif
    return false;
}
bool Window::isMaximized() const noexcept { 
    bool result = false;
#ifdef _WIN32
    result = internal_return_window_placement_cmd(SW_MAXIMIZE);
#endif
    return result;
}
bool Window::isMinimized() const noexcept { 
    bool result = false;
#ifdef _WIN32
    result = internal_return_window_placement_cmd(SW_MINIMIZE);
#endif
    return result;
}
bool Window::setActive(bool isToBeActive) {
    bool result = m_Data.m_SFMLWindow.setActive(isToBeActive);
    if (result) {
        if (isToBeActive) {
            m_Data.m_OpenGLThreadID = std::this_thread::get_id();
            m_Data.m_Flags.add(Window_Flags::Active);
        } else {
            m_Data.m_Flags.remove(Window_Flags::Active);
        }
    } else {
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): failed when called from thread: " << std::this_thread::get_id())     
    }
    return result;
}
void Window::setSize(uint32_t width, uint32_t height) {
    if (m_Data.m_VideoMode.width == width && m_Data.m_VideoMode.height == height) {
        return;
    }
    m_Data.m_OldWindowSize = glm::uvec2{ m_Data.m_VideoMode.width, m_Data.m_VideoMode.height };
    m_Data.m_VideoMode     = sf::VideoMode{ width, height, m_Data.m_VideoMode.bitsPerPixel };
    m_Data.m_SFMLWindow.setSize(sf::Vector2u{ width, height });
}
bool Window::setFullscreenWindowed(bool isToBeFullscreen) {
    if (isToBeFullscreen) {
        if (isFullscreenWindowed()) {
            return false;
        }
        m_Data.m_Style = sf::Style::None;    //windowed_fullscreen
        m_Data.m_Flags.remove(Window_Flags::Fullscreen | Window_Flags::Windowed);
        m_Data.m_Flags.add(Window_Flags::WindowedFullscreen);
    } else {
        if (!isFullscreen()) {
            return false;
        }
        m_Data.m_Style = sf::Style::Default; //windowed
        m_Data.m_Flags.remove(Window_Flags::Fullscreen | Window_Flags::WindowedFullscreen);
        m_Data.m_Flags.add(Window_Flags::Windowed);
    }
    m_Data.internal_on_fullscreen(*this, isToBeFullscreen, isMaximized(), isMinimized());
    return true;
}
bool Window::setFullscreen(bool isToBeFullscreen) {
    if (isToBeFullscreen) {
        if (isFullscreenNonWindowed()) {
            return false;
        }
        m_Data.m_Style = sf::Style::Fullscreen; //fullscreen   
        m_Data.m_Flags.remove(Window_Flags::WindowedFullscreen | Window_Flags::Windowed);
        m_Data.m_Flags.add(Window_Flags::Fullscreen);
    } else {
        if (!isFullscreen()) {
            return false;
        }
        m_Data.m_Style = sf::Style::Default;    //windowed
        m_Data.m_Flags.remove(Window_Flags::Fullscreen | Window_Flags::WindowedFullscreen);
        m_Data.m_Flags.add(Window_Flags::Windowed);
    }
    m_Data.internal_on_fullscreen(*this, isToBeFullscreen, isMaximized(), isMinimized());
    return true;
}
void Window::keepMouseInWindow(bool isToBeKept) {
    (isToBeKept) ? m_Data.m_WindowThread.internal_push(WindowEventThreadOnlyCommands::KeepMouseInWindow) : m_Data.m_WindowThread.internal_push(WindowEventThreadOnlyCommands::FreeMouseFromWindow);
}
void Window::setFramerateLimit(uint32_t limit){
    m_Data.m_SFMLWindow.setFramerateLimit(limit);
    m_Data.m_FramerateLimit = limit;
}
void Window::setVisible(bool isVisible) { m_Data.m_SFMLWindow.setVisible(isVisible); }
void Window::hide() { setVisible(false); }
void Window::show() { setVisible(true); }