
#include <serenity/system/Engine.h>
#include <serenity/system/EngineOptions.h>
#include <serenity/system/window/Window.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/system/cursor/Cursor.h>

#include <serenity/renderer/APIContext.h>

namespace {
    constexpr const std::array<std::pair<uint32_t, uint32_t>, size_t(WindowMode::_TOTAL)> WINDOW_MODE_DATA{ {
        { sf::Style::Default,     Window_Flags::Windowed },
        { sf::Style::None,        Window_Flags::WindowedFullscreen },
        { sf::Style::Fullscreen,  Window_Flags::Fullscreen },
    } };

    void internal_center_window_to_desktop_screen(Window& window) {
        const auto winSize         = glm::vec2{ window.getSize() };
        const auto desktopSize     = sf::VideoMode::getDesktopMode();
        float final_desktop_width  = float(desktopSize.width);
        float final_desktop_height = float(desktopSize.height);
        // float x_other              = 0.0f;
        float y_other = 0.0f;
#ifdef _WIN32
        // get the dimensions of the desktop's bottom task bar. Only tested on Windows 10
        // auto os_handle = super.getSFMLHandle().getSystemHandle();
        //            left   right   top   bottom
        RECT rect; //[0,     1920,   0,    1040]  //bottom task bar
        SystemParametersInfoA(SPI_GETWORKAREA, 0, &rect, 0);
        y_other = final_desktop_height - float(rect.bottom);
        final_desktop_height -= y_other;
#endif
        window.setPosition(uint32_t((final_desktop_width - winSize.x) / 2.0f), uint32_t((final_desktop_height - winSize.y) / 2.0f));
    }


}

Window::Window(const EngineOptions& options)
    : m_Data{ 32 }
{
    m_Data.m_WindowTitle = options.window_title;
    m_Data.m_VideoMode = sf::VideoMode{ options.width, options.height, m_Data.m_VideoMode.bitsPerPixel };
    m_Data.internal_create(*this, options.window_title);

    internal_center_window_to_desktop_screen(*this);
    setWindowMode(options.window_mode);
    if (options.maximized) {
        maximize();
    }
    requestFocus();
    setIcon(options.icon);
    setVisible(true);
}
Window::Window(Window&& other) noexcept 
    : m_Data{ std::move(other.m_Data) }
    , m_RenderingContext{ std::move(other.m_RenderingContext)}
{
}
Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        m_Data = std::move(other.m_Data);
        m_RenderingContext = std::move(other.m_RenderingContext);
    }
    return *this;
}
Window::~Window() {
    if (m_RenderingContext) {
        m_RenderingContext->destroy(*this);
    }
}
void Window::setRenderContext(Engine::view_ptr<Engine::priv::IContext> context) {
    if (m_RenderingContext) {
        m_RenderingContext->destroy(*this);
        m_RenderingContext = nullptr;
    }
    if (context->init(*this)) {
        m_RenderingContext = context;
    }
}

bool Window::setWindowMode(WindowMode::Mode mode) {
    const auto& modeData = WINDOW_MODE_DATA[mode];
    if (!(m_Data.m_Flags & modeData.second)) {
        m_Data.m_Style = modeData.first;
        m_Data.m_Flags.remove(Window_Flags::Windowed | Window_Flags::Fullscreen | Window_Flags::WindowedFullscreen);
        m_Data.m_Flags.add(modeData.second);
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
    m_Data.m_SFMLWindow->setMouseCursor(cursor.getSFMLCursor());
}
void Window::updateMousePosition(float x, float y, bool resetDifference, bool resetPrevious) {
    m_Data.internal_update_mouse_position(*this, x, y, resetDifference, resetPrevious);
}
void Window::updateMousePosition(const glm::vec2& position, bool resetDifference, bool resetPrevious) {
    m_Data.internal_update_mouse_position(*this, position.x, position.y, resetDifference, resetPrevious);
}
bool Window::internal_execute_show_window(uint32_t cmd) noexcept {
    #ifdef _WIN32
        ::ShowWindow(m_Data.m_SFMLWindow->getSystemHandle(), cmd);
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
        GetWindowInfo(m_Data.m_SFMLWindow->getSystemHandle(), &wiInfo);
        const glm::ivec2 current_size = glm::ivec2{ int(wiInfo.rcClient.right) - int(wiInfo.rcClient.left), int(wiInfo.rcClient.bottom) - int(wiInfo.rcClient.top) };
        const glm::ivec2 old_size = getSize();

        if (current_size.x >= 0 && current_size.y >= 0 && (current_size.x != old_size.x || current_size.y != old_size.y)) {
            setSize(current_size.x, current_size.y);
            Engine::priv::Core::m_Engine->m_Modules->m_EngineEventHandler.internal_on_event_resize(*this, current_size.x, current_size.y, true, *Engine::priv::Core::m_Engine->m_GameCore);
        }
    #endif
}
bool Window::pollEvents(sf::Event& outEvent) {
    auto sfEvent        = m_Data.internal_try_pop();
    const bool hasEvent = sfEvent.has_value();
    if (hasEvent) {
        outEvent = std::move(*sfEvent);
    }
    return hasEvent;
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
void Window::setPosition(int32_t x, int32_t y) { 
    m_Data.m_SFMLWindow->setPosition(sf::Vector2i(x, y));
}
glm::ivec2 Window::getPosition() {
    const auto position = m_Data.m_SFMLWindow->getPosition();
    return glm::ivec2{ position.x, position.y };
}
glm::ivec2 Window::getSize(){
    const auto windowSize = m_Data.m_SFMLWindow->getSize();
    return glm::ivec2{ windowSize.x, windowSize.y };
}
bool Window::setIcon(const Texture& texture) {
    m_Data.m_SFMLWindow->setIcon(texture.width(), texture.height(), const_cast<Texture&>(texture).pixels());
    m_Data.m_IconFile = texture.name();
    return true;
}
bool Window::setIcon(std::string_view inFile) {
    if (!inFile.empty()) {
        sf::Image iconImage;
        const auto successfulLoad = iconImage.loadFromFile(std::string{ inFile });
        if (successfulLoad) {
            m_Data.m_SFMLWindow->setIcon(iconImage.getSize().x, iconImage.getSize().y, iconImage.getPixelsPtr());
            m_Data.m_IconFile = inFile;
        }
        return successfulLoad;
    }
    return false;
}
void Window::setTitle(std::string_view inTitle) {
    if (m_Data.m_WindowTitle == inTitle) {
        return;
    }
    std::string title{ inTitle };
    m_Data.m_SFMLWindow->setTitle(title);
    m_Data.m_WindowTitle = std::move(title);
}
void Window::setVerticalSyncEnabled(bool isToBeEnabled) {
    if (m_RenderingContext) {
        m_RenderingContext->setVerticalSyncEnabled(isToBeEnabled);
    }
    (isToBeEnabled) ? m_Data.m_Flags.add(Window_Flags::Vsync) : m_Data.m_Flags.remove(Window_Flags::Vsync);
}
bool Window::isVsyncEnabled() const { 
    return m_Data.m_Flags.has(Window_Flags::Vsync); 
}
void Window::setKeyRepeatEnabled(bool isToBeEnabled) {
    m_Data.m_SFMLWindow->setKeyRepeatEnabled(isToBeEnabled);
    (isToBeEnabled) ? m_Data.m_Flags.add(Window_Flags::KeyRepeat) : m_Data.m_Flags.remove(Window_Flags::KeyRepeat);
}
void Window::setMouseCursorVisible(bool isToBeVisible) {
    (isToBeVisible) ? m_Data.internal_push(WindowEventThreadOnlyCommands::ShowMouse) : m_Data.internal_push(WindowEventThreadOnlyCommands::HideMouse);
}
void Window::requestFocus() { 
    m_Data.internal_push(WindowEventThreadOnlyCommands::RequestFocus);
}
void Window::close() {
    m_Data.internal_on_close(true);
}
bool Window::isWindowOnSeparateThread() const { 
    return m_Data.m_EventThread != nullptr;
}
bool Window::hasFocus() const {
    return m_Data.m_SFMLWindow->hasFocus(); 
}
bool Window::isOpen() const { 
    return m_Data.m_SFMLWindow->isOpen();
}
bool Window::isActive() const {
    return m_Data.m_Flags.has(Window_Flags::Active); 
}

bool Window::isMouseKeptInWindow() const { 
    return m_Data.m_Flags.has(Window_Flags::MouseGrabbed); 
}
void Window::display() { 
    if (m_RenderingContext) {
        m_RenderingContext->display();
    }
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
void Window::setSize(int32_t width, int32_t height) {
    if (width < 0 || height < 0 || (m_Data.m_VideoMode.width == width && m_Data.m_VideoMode.height == height)) {
        return;
    }
    m_Data.m_OldWindowSize = glm::ivec2( m_Data.m_VideoMode.width, m_Data.m_VideoMode.height );
    m_Data.m_VideoMode     = sf::VideoMode( width, height, m_Data.m_VideoMode.bitsPerPixel );
    m_Data.m_SFMLWindow->setSize(sf::Vector2u( width, height ));
}
void Window::keepMouseInWindow(bool isToBeKept) {
    (isToBeKept) ? m_Data.internal_push(WindowEventThreadOnlyCommands::KeepMouseInWindow) : m_Data.internal_push(WindowEventThreadOnlyCommands::FreeMouseFromWindow);
}
void Window::setFramerateLimit(uint32_t limit){
    if (m_RenderingContext) {
        m_RenderingContext->setFramerateLimit(static_cast<int>(limit));
    }
    m_Data.m_FramerateLimit = limit;
}
void Window::setVisible(bool isVisible) { 
    m_Data.m_SFMLWindow->setVisible(isVisible);
}
void Window::hide() {
    setVisible(false); 
}
void Window::show() {
    setVisible(true); 
}