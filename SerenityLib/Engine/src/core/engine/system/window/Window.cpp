#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/system/Engine.h>
#include <core/engine/system/EngineOptions.h>
#include <core/engine/renderer/opengl/OpenGL.h>
#include <core/engine/system/window/Window.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/system/cursor/Cursor.h>

#pragma region Window
Window::Window() {
    internal_init();
}
Window::~Window(){
}
void Window::init(const EngineOptions& options) noexcept {
    m_Data.m_WindowName        = options.window_title;
    m_Data.m_VideoMode.width   = options.width;
    m_Data.m_VideoMode.height  = options.height;
    m_Data.m_SFContextSettings = m_Data.internal_create(*this, options.window_title);
    int requested_glsl_version = std::stoi(Engine::priv::OpenGL::getHighestGLSLVersion(*this));
    int opengl_version         = std::stoi(std::to_string(m_Data.m_SFContextSettings.majorVersion) + std::to_string(m_Data.m_SFContextSettings.minorVersion));
    Engine::priv::Core::m_Engine->m_RenderManager._onOpenGLContextCreation(
        m_Data.m_VideoMode.width, 
        m_Data.m_VideoMode.height, 
        (unsigned int)requested_glsl_version, 
        (unsigned int)opengl_version
    );

    m_Data.internal_init_position(*this);

    if (!options.icon.empty()) {
        setIcon(options.icon);
    }
    if (options.window_mode == 1) {
        setFullscreen(true);
    }else if (options.window_mode == 2) {
        setFullscreenWindowed(true);
    }else {
        setFullscreen(false);
    }
    if (options.maximized) {
        maximize();
    }

    requestFocus();
    display();

    //setVerticalSyncEnabled(options.vsync); //unfortunately this will not work until a few frames after the window creation

    if (options.show_console) {
        ENGINE_PRODUCTION_LOG("Using OpenGL: " << m_Data.m_SFContextSettings.majorVersion << "." << m_Data.m_SFContextSettings.minorVersion <<
            ", with depth bits: " << m_Data.m_SFContextSettings.depthBits <<
            " and stencil bits: " << m_Data.m_SFContextSettings.stencilBits <<
            " and glsl version: " << requested_glsl_version)
    }
}
void Window::internal_init() noexcept {
    m_Data.m_SFContextSettings.depthBits          = 24;
    m_Data.m_SFContextSettings.stencilBits        = 0;
    m_Data.m_SFContextSettings.antialiasingLevel  = 0;
    m_Data.m_SFContextSettings.majorVersion       = 4;
    m_Data.m_SFContextSettings.minorVersion       = 6;

    #ifdef _DEBUG
        m_Data.m_SFContextSettings.attributeFlags = m_Data.m_SFContextSettings.Debug;
    #else
        m_Data.m_SFContextSettings.attributeFlags = m_Data.m_SFContextSettings.Core;
    #endif

    m_Data.m_VideoMode.bitsPerPixel = 32;
    m_Data.m_Style                  = sf::Style::Default;
}
void Window::setMouseCursor(const Cursor& cursor) noexcept {
    m_Data.m_SFMLWindow.setMouseCursor(cursor.getSFMLCursor());
}
void Window::setJoystickProcessingActive(bool active) {
    m_Data.m_SFMLWindow.setJoystickManagerActive(active);
}
bool Window::isJoystickProcessingActive() const {
    return m_Data.m_SFMLWindow.isJoystickManagerActive();
}
void Window::updateMousePosition(float x, float y, bool resetDifference, bool resetPrevious) {
    m_Data.internal_update_mouse_position(*this, x, y, resetDifference, resetPrevious);
}
void Window::updateMousePosition(const glm::vec2& position, bool resetDifference, bool resetPrevious) {
    m_Data.internal_update_mouse_position(*this, position.x, position.y, resetDifference, resetPrevious);
}
bool Window::internal_execute_show_window(unsigned int cmd) noexcept {
    #ifdef _WIN32
        ::ShowWindow(m_Data.m_SFMLWindow.getSystemHandle(), cmd);
        return true;
    #endif
    return false;
}
bool Window::maximize() noexcept {
    return internal_execute_show_window(SW_MAXIMIZE);
}
bool Window::minimize() noexcept {
    return internal_execute_show_window(SW_MINIMIZE);
}
void Window::setPosition(unsigned int x, unsigned int y) {
    m_Data.m_SFMLWindow.setPosition(sf::Vector2i(x, y));
}
glm::uvec2 Window::getPosition() {
    auto position = m_Data.m_SFMLWindow.getPosition();
    return glm::uvec2(position.x, position.y);
}
glm::uvec2 Window::getSize(){
    sf::Vector2u window_size = m_Data.m_SFMLWindow.getSize();
    return glm::uvec2(window_size.x, window_size.y);
}
void Window::setIcon(const Texture& texture) {
    m_Data.m_SFMLWindow.setIcon(texture.width(), texture.height(), const_cast<Texture&>(texture).pixels());
    m_Data.m_IconFile = texture.name();
}
void Window::setIcon(const char* file) {
    auto texture = Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.get<Texture>(file);
    if (!texture.first) {
        texture.second = Engine::Resources::loadTexture(file);
        texture.first  = texture.second.get<Texture>();
    }
    m_Data.m_SFMLWindow.setIcon(texture.first->width(), texture.first->height(), texture.first->pixels());
    m_Data.m_IconFile = file;
}
void Window::setIcon(const std::string& file) {
    if (file.empty()) {
        return;
    }
    Window::setIcon(file.c_str());
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
void Window::setKeyRepeatEnabled(bool isToBeEnabled) {
    m_Data.m_SFMLWindow.setKeyRepeatEnabled(isToBeEnabled);
    (isToBeEnabled) ? m_Data.m_Flags.add(Window_Flags::KeyRepeat) : m_Data.m_Flags.remove(Window_Flags::KeyRepeat);
}
void Window::setMouseCursorVisible(bool isToBeVisible) {
    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        (isToBeVisible) ? m_Data.m_WindowThread.internal_push(WindowEventThreadOnlyCommands::ShowMouse) : m_Data.m_WindowThread.internal_push(WindowEventThreadOnlyCommands::HideMouse);
    #else
        m_Data.m_SFMLWindow.setMouseCursorVisible(isToBeVisible);
        (isToBeVisible) ? m_Data.m_Flags.add(Window_Flags::MouseVisible) : m_Data.m_Flags.remove(Window_Flags::MouseVisible);
    #endif
}
void Window::requestFocus() {
    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        m_Data.m_WindowThread.internal_push(WindowEventThreadOnlyCommands::RequestFocus);
    #else
        m_Data.m_SFMLWindow.requestFocus();
    #endif
}
void Window::close() {
    Engine::priv::Core::m_Engine->internal_on_event_window_closed(*this);
    m_Data.m_SFMLWindow.close();
}
bool Window::isWindowOnSeparateThread() const {
    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        if (m_Data.m_WindowThread == true) {
            return true;
        }
    #endif
    return false;
}
bool Window::hasFocus() const {
    return m_Data.m_SFMLWindow.hasFocus();
}
bool Window::isOpen() const {
    return m_Data.m_SFMLWindow.isOpen();
}
bool Window::isActive() const {
    return m_Data.m_Flags.has(Window_Flags::Active);
}
bool Window::isFullscreen() const {
    return isFullscreenNonWindowed() || isFullscreenWindowed();
}
bool Window::isFullscreenWindowed() const {
    return m_Data.m_Flags.has(Window_Flags::WindowedFullscreen);
}
bool Window::isFullscreenNonWindowed() const {
    return m_Data.m_Flags.has(Window_Flags::Fullscreen);
}
bool Window::isMouseKeptInWindow() const {
    return m_Data.m_Flags.has(Window_Flags::MouseGrabbed);
}
void Window::display() {
    m_Data.m_SFMLWindow.display();
}
bool Window::internal_return_window_placement_cmd(unsigned int cmd) const noexcept {
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
    return internal_return_window_placement_cmd(SW_MAXIMIZE);
}
bool Window::isMinimized() const noexcept {
    return internal_return_window_placement_cmd(SW_MINIMIZE);
}
bool Window::setActive(bool isToBeActive) {
    bool result = m_Data.m_SFMLWindow.setActive(isToBeActive);
    if (result) {
        if (isToBeActive) {
            m_Data.m_OpenGLThreadID = std::this_thread::get_id();
            m_Data.m_Flags.add(Window_Flags::Active);
        }else{
            m_Data.m_Flags.remove(Window_Flags::Active);
        }
    }else{
        ENGINE_PRODUCTION_LOG("error: Window::setActive() failed when called from thread: " << std::this_thread::get_id())     
    }
    return result;
}
void Window::setSize(unsigned int width, unsigned int height) {
    if (m_Data.m_VideoMode.width == width && m_Data.m_VideoMode.height == height) {
        return;
    }
    m_Data.m_OldWindowSize = glm::uvec2(m_Data.m_VideoMode.width, m_Data.m_VideoMode.height);

    m_Data.m_VideoMode.width  = width;
    m_Data.m_VideoMode.height = height;

    auto dimensions = sf::Vector2u();
    dimensions.x = width;
    dimensions.y = height;
    m_Data.m_SFMLWindow.setSize(dimensions);
}
void Window::internal_restore_state() {
    m_Data.internal_restore_state(*this);
}
bool Window::setFullscreenWindowed(bool isToBeFullscreen) {
    if (isToBeFullscreen) {
        if (isFullscreenWindowed()) {
            return false;
        }
        m_Data.m_Style = sf::Style::None;    //windowed_fullscreen
        m_Data.m_Flags.add(Window_Flags::WindowedFullscreen);
        m_Data.m_Flags.remove(Window_Flags::Fullscreen);
        m_Data.m_Flags.remove(Window_Flags::Windowed);
    }else{
        if (!isFullscreen()) {
            return false;
        }
        m_Data.m_Style = sf::Style::Default; //windowed
        m_Data.m_Flags.add(Window_Flags::Windowed);
        m_Data.m_Flags.remove(Window_Flags::Fullscreen);
        m_Data.m_Flags.remove(Window_Flags::WindowedFullscreen);
    }
    bool old_max = isMaximized();
    bool old_min = isMinimized();
    m_Data.internal_on_fullscreen(*this, isToBeFullscreen, old_max, old_min);
    return true;
}
bool Window::setFullscreen(bool isToBeFullscreen) {
    if (isToBeFullscreen) {
        if (isFullscreenNonWindowed()) {
            return false;
        }
        m_Data.m_Style = sf::Style::Fullscreen; //fullscreen   
        m_Data.m_Flags.add(Window_Flags::Fullscreen);
        m_Data.m_Flags.remove(Window_Flags::WindowedFullscreen);
        m_Data.m_Flags.remove(Window_Flags::Windowed);
    }else{
        if (!isFullscreen()) {
            return false;
        }
        m_Data.m_Style = sf::Style::Default;    //windowed
        m_Data.m_Flags.add(Window_Flags::Windowed);
        m_Data.m_Flags.remove(Window_Flags::Fullscreen);
        m_Data.m_Flags.remove(Window_Flags::WindowedFullscreen);
    }
    bool old_max = isMaximized();
    bool old_min = isMinimized();
    m_Data.internal_on_fullscreen(*this, isToBeFullscreen, old_max, old_min);
    return true;
}
void Window::keepMouseInWindow(bool isToBeKept) {
    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        (isToBeKept) ? m_Data.m_WindowThread.internal_push(WindowEventThreadOnlyCommands::KeepMouseInWindow) : m_Data.m_WindowThread.internal_push(WindowEventThreadOnlyCommands::FreeMouseFromWindow);
    #else
        m_Data.m_SFMLWindow.setMouseCursorGrabbed(isToBeKept);
        (isToBeKept) ? m_Data.m_Flags.add(Window_Flags::MouseGrabbed) : m_Data.m_Flags.remove(Window_Flags::MouseGrabbed);
    #endif
}
void Window::setFramerateLimit(unsigned int limit){
    m_Data.m_SFMLWindow.setFramerateLimit(limit);
    m_Data.m_FramerateLimit = limit;
}
void Window::internal_on_dynamic_resize() {
    #ifdef _WIN32
        WINDOWINFO wiInfo;
        GetWindowInfo(m_Data.m_SFMLWindow.getSystemHandle(), &wiInfo);
        const glm::uvec2 current_size = glm::uvec2(wiInfo.rcClient.right - wiInfo.rcClient.left, wiInfo.rcClient.bottom - wiInfo.rcClient.top);
        const glm::uvec2 old_size     = getSize();

        if (current_size.x != old_size.x || current_size.y != old_size.y) {
            Window::setSize(current_size.x, current_size.y);
            Engine::priv::Core::m_Engine->internal_on_event_resize(*this, current_size.x, current_size.y, true);
        }
    #endif
}
bool Window::pollEvents(sf::Event& e) {
    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        std::optional<sf::Event> x = m_Data.m_WindowThread.internal_try_pop(); //expensive as it uses lock & mutex
        if (x) {
            e = std::move(*x);
        }
        return (bool)x;
    #else
        return m_Data.m_SFMLWindow.pollEvent(e);
    #endif
}
#pragma endregion