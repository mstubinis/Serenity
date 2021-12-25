
#include <serenity/system/Engine.h>
#include <serenity/system/EngineOptions.h>
#include <serenity/renderer/opengl/OpenGL.h>
#include <serenity/system/window/Window.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/system/cursor/Cursor.h>

namespace {
    constexpr std::array<std::pair<uint32_t, uint32_t>, size_t(WindowMode::_TOTAL)> WINDOW_MODE_DATA{
        std::make_pair(static_cast<uint32_t>(sf::Style::Default),     static_cast<uint32_t>(Window_Flags::Windowed)),
        std::make_pair(static_cast<uint32_t>(sf::Style::None),        static_cast<uint32_t>(Window_Flags::WindowedFullscreen)),
        std::make_pair(static_cast<uint32_t>(sf::Style::Fullscreen),  static_cast<uint32_t>(Window_Flags::Fullscreen)),
    };

}
Window::Window() {
    m_Data.m_SFContextSettings = sf::ContextSettings{ 24, 0, 0, 4, 6, 0, false };

    #ifdef _DEBUG
        m_Data.m_SFContextSettings.attributeFlags = m_Data.m_SFContextSettings.Debug;
    #else
        m_Data.m_SFContextSettings.attributeFlags = m_Data.m_SFContextSettings.Core;
    #endif

    m_Data.m_VideoMode.bitsPerPixel = 32;
}
void Window::init(const EngineOptions& options) noexcept {
    m_Data.m_WindowTitle       = options.window_title;
    m_Data.m_VideoMode         = sf::VideoMode{ options.width, options.height, m_Data.m_VideoMode.bitsPerPixel };
    m_Data.m_SFContextSettings = m_Data.internal_create(*this, options.window_title);
    //int opengl_version         = std::stoi(std::to_string(m_Data.m_SFContextSettings.majorVersion) + std::to_string(m_Data.m_SFContextSettings.minorVersion));
    Engine::priv::Core::m_Engine->m_RenderModule._onOpenGLContextCreation(m_Data.m_VideoMode.width, m_Data.m_VideoMode.height);

    m_Data.internal_init_position(*this);
    display();
    setVisible(false);
    setIcon(options.icon);
    setWindowMode(options.window_mode);
    if (options.maximized) {
        maximize();
    }
    requestFocus();
    setVisible(true);
    setVerticalSyncEnabled(options.vsync);
    setJoystickProcessingActive(false);
    if (options.show_console) {
        ENGINE_PRODUCTION_LOG("Using OpenGL: " << m_Data.m_SFContextSettings.majorVersion << "." << m_Data.m_SFContextSettings.minorVersion <<
            ", with depth bits: " << m_Data.m_SFContextSettings.depthBits << " and stencil bits: " << m_Data.m_SFContextSettings.stencilBits
        )
    }
}
bool Window::setWindowMode(WindowMode::Mode mode) {
    const auto& data = WINDOW_MODE_DATA[mode];
    if (!(m_Data.m_Flags & data.second)) {
        m_Data.m_Style = data.first;
        m_Data.m_Flags.remove(Window_Flags::Windowed | Window_Flags::Fullscreen | Window_Flags::WindowedFullscreen);
        m_Data.m_Flags.add(data.second);
        m_Data.internal_on_fullscreen(*this, mode == WindowMode::Fullscreen || mode == WindowMode::WindowedFullscreen, isMaximized(), isMinimized());
        return true;
    }
    return false;
}
WindowMode::Mode Window::getWindowMode() const noexcept {
    if (m_Data.m_Flags & Window_Flags::Fullscreen) {
        return WindowMode::Fullscreen;
    } else if (m_Data.m_Flags & Window_Flags::WindowedFullscreen) {
        return WindowMode::WindowedFullscreen;
    }
    return WindowMode::Windowed;
}
void Window::setMouseCursor(const Cursor& cursor) noexcept { 
    m_Data.m_SFMLWindow.setMouseCursor(cursor.getSFMLCursor());
}
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
void Window::internal_restore_state() { 
    m_Data.internal_restore_state(*this); 
}
void Window::internal_update_dynamic_resize() {
    #ifdef _WIN32
        WINDOWINFO wiInfo;
        GetWindowInfo(m_Data.m_SFMLWindow.getSystemHandle(), &wiInfo);
        const glm::uvec2 current_size = glm::uvec2{ wiInfo.rcClient.right - wiInfo.rcClient.left, wiInfo.rcClient.bottom - wiInfo.rcClient.top };
        const glm::uvec2 old_size = getSize();

        if (current_size.x != old_size.x || current_size.y != old_size.y) {
            Window::setSize(current_size.x, current_size.y);
            Engine::priv::Core::m_Engine->m_EngineEventHandler.internal_on_event_resize(*this, current_size.x, current_size.y, true, *Engine::priv::Core::m_Engine->m_GameCore);
        }
    #endif
}
bool Window::pollEvents(sf::Event& e) {
    auto x = m_Data.internal_try_pop();
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
void Window::setPosition(uint32_t x, uint32_t y) { 
    m_Data.m_SFMLWindow.setPosition(sf::Vector2i(x, y));
}
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
void Window::setIcon(std::string_view inFile) {
    if (!inFile.empty()) {
        auto texture = Engine::Resources::getResource<Texture>(inFile);
        if (!texture.m_Resource) {
            texture.m_Handle = Engine::Resources::loadTexture(inFile, ImageInternalFormat::SRGB8_ALPHA8, false);
            texture.m_Resource = texture.m_Handle.get<Texture>();
        }
        uint32_t width  = texture.m_Resource->width();
        uint32_t height = texture.m_Resource->height();

        std::vector<uint8_t> pixels(width * height * 4);
        std::copy(texture.m_Resource->pixels(), texture.m_Resource->pixels() + (width * height * 4), pixels.data());

        Engine::priv::TextureLoader::MirrorPixelsVertically(pixels.data(), width, height);

        m_Data.m_SFMLWindow.setIcon(width, height, pixels.data());
        m_Data.m_IconFile = inFile;
    }
}
void Window::setTitle(std::string_view inTitle) {
    if (m_Data.m_WindowTitle == inTitle) {
        return;
    }
    std::string title{ inTitle };
    m_Data.m_SFMLWindow.setTitle(title);
    m_Data.m_WindowTitle = std::move(title);
}
void Window::setVerticalSyncEnabled(bool isToBeEnabled) {
    m_Data.m_SFMLWindow.setVerticalSyncEnabled(isToBeEnabled);
    (isToBeEnabled) ? m_Data.m_Flags.add(Window_Flags::Vsync) : m_Data.m_Flags.remove(Window_Flags::Vsync);
}
bool Window::isVsyncEnabled() const { 
    return m_Data.m_Flags.has(Window_Flags::Vsync); 
}
void Window::setKeyRepeatEnabled(bool isToBeEnabled) {
    m_Data.m_SFMLWindow.setKeyRepeatEnabled(isToBeEnabled);
    (isToBeEnabled) ? m_Data.m_Flags.add(Window_Flags::KeyRepeat) : m_Data.m_Flags.remove(Window_Flags::KeyRepeat);
}
void Window::setMouseCursorVisible(bool isToBeVisible) {
    (isToBeVisible) ? m_Data.internal_push(WindowEventThreadOnlyCommands::ShowMouse) : m_Data.internal_push(WindowEventThreadOnlyCommands::HideMouse);
}
void Window::requestFocus() { 
    m_Data.internal_push(WindowEventThreadOnlyCommands::RequestFocus);
}
void Window::close() {
    Engine::priv::Core::m_Engine->m_EngineEventHandler.internal_on_event_window_closed(*this);
    m_Data.m_SFMLWindow.close();
}
bool Window::isWindowOnSeparateThread() const { 
    return m_Data.m_EventThread != nullptr;
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

bool Window::isMouseKeptInWindow() const { 
    return m_Data.m_Flags.has(Window_Flags::MouseGrabbed); 
}
void Window::display() { 
    m_Data.m_SFMLWindow.display(); 
}
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
void Window::keepMouseInWindow(bool isToBeKept) {
    (isToBeKept) ? m_Data.internal_push(WindowEventThreadOnlyCommands::KeepMouseInWindow) : m_Data.internal_push(WindowEventThreadOnlyCommands::FreeMouseFromWindow);
}
void Window::setFramerateLimit(uint32_t limit){
    m_Data.m_SFMLWindow.setFramerateLimit(limit);
    m_Data.m_FramerateLimit = limit;
}
void Window::setVisible(bool isVisible) { 
    m_Data.m_SFMLWindow.setVisible(isVisible);
}
void Window::hide() { 
    setVisible(false); 
}
void Window::show() {
    setVisible(true); 
}