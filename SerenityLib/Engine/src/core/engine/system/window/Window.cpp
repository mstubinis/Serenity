#include <core/engine/system/Engine.h>
#include <core/engine/system/EngineOptions.h>
#include <core/engine/system/window/Window.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/events/Engine_EventDispatcher.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Viewport.h>

#include <ecs/ECS.h>
#include <ecs/ComponentCamera.h>

#include <chrono>

#include <iostream>

using namespace Engine;
using namespace std;

#pragma region WindowThread
Window::WindowData::WindowThread::WindowThread(WindowData& data) : m_Data(data){
    m_EventThread = nullptr;
}
Window::WindowData::WindowThread::~WindowThread() {
    cleanup();
}
bool Window::WindowData::WindowThread::operator==(const bool& rhs) const {
    if (rhs == true) {
        return (m_EventThread) ? true : false;
    }
    return (m_EventThread) ? false : true;
}
Window::WindowData::WindowThread::operator bool() const {
    return (m_EventThread) ? true : false;
}
std::optional<sf::Event> Window::WindowData::WindowThread::try_pop() {
    auto x = m_Queue.try_pop();
    return std::move(x);
}
void Window::WindowData::WindowThread::push(const EventThreadOnlyCommands::Command& cmd) {
    m_MainThreadToEventThreadQueue.push(cmd);
}
void Window::WindowData::WindowThread::startup(Window& super, const string& name) {
    auto lamda = [&]() {
        m_Data.m_SFMLWindow.create(m_Data.m_VideoMode, name, m_Data.m_Style, m_Data.m_SFContextSettings);
        if (!m_Data.m_IconFile.empty())
            super.setIcon(m_Data.m_IconFile);
        m_Data.m_SFMLWindow.setActive(false);
        m_Data.m_UndergoingClosing = false;
        sf::Event e;
        while (!m_Data.m_UndergoingClosing) {
            while (!m_Data.m_UndergoingClosing && m_Data.m_SFMLWindow.pollEvent(e)) {
                m_Queue.push(std::move(e));
            }
            auto command_ptr = m_MainThreadToEventThreadQueue.try_pop();
            while (command_ptr) {
                auto& command = *command_ptr;

                switch (command) {
                    case EventThreadOnlyCommands::HideMouse: {
                        m_Data.m_SFMLWindow.setMouseCursorVisible(false);
                        m_Data.m_Flags.add(Window_Flags::MouseVisible);
                        break;
                    }case EventThreadOnlyCommands::ShowMouse: {
                        m_Data.m_SFMLWindow.setMouseCursorVisible(true);
                        m_Data.m_Flags.add(Window_Flags::MouseVisible);
                        break;
                    }case EventThreadOnlyCommands::RequestFocus: {
                        m_Data.m_SFMLWindow.requestFocus();
                        break;
                    }case EventThreadOnlyCommands::KeepMouseInWindow: {
                        m_Data.m_SFMLWindow.setMouseCursorGrabbed(true);
                        m_Data.m_Flags.add(Window_Flags::MouseGrabbed);
                        break;
                    }case EventThreadOnlyCommands::FreeMouseFromWindow: {
                        m_Data.m_SFMLWindow.setMouseCursorGrabbed(false);
                        m_Data.m_Flags.remove(Window_Flags::MouseGrabbed);
                        break;
                    }default: {
                        break;
                    }
                }
                command_ptr = m_MainThreadToEventThreadQueue.try_pop();
            }
        }
    };
    m_EventThread.reset(NEW std::thread(lamda));
}
void Window::WindowData::WindowThread::cleanup() {
    if (m_EventThread && m_EventThread->joinable()) {
        m_EventThread->join();
    }
}
#pragma endregion

#pragma region WindowData

Window::WindowData::WindowData() 
#ifdef ENGINE_THREAD_WINDOW_EVENTS
: m_WindowThread(*this) 
#endif
{
    m_FramerateLimit      = 0;
    m_IconFile            = "";
    m_UndergoingClosing   = false;
    m_MouseDelta          = 0.0;
    m_OldWindowSize       = glm::uvec2(0, 0);
    m_Flags               = (Window_Flags::Windowed | Window_Flags::MouseVisible | Window_Flags::Active);
    m_MousePosition       = m_MousePosition_Previous = m_MouseDifference = glm::vec2(0.0f);
}
Window::WindowData::~WindowData() {
    on_close();
}
void Window::WindowData::on_close() {
    m_UndergoingClosing = true;
    m_SFMLWindow.setVisible(false);
    m_SFMLWindow.close();


    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        m_WindowThread.cleanup();
    #endif
}
void Window::WindowData::on_mouse_wheel_scrolled(const float& delta, const int& x, const int& y) {
    m_MouseDelta += (static_cast<double>(delta) * 10.0);
}
void Window::WindowData::restore_state() {
    if (m_FramerateLimit > 0) {
        m_SFMLWindow.setFramerateLimit(m_FramerateLimit);
    }
    m_SFContextSettings = m_SFMLWindow.getSettings();

    m_SFMLWindow.setMouseCursorVisible(m_Flags & Window_Flags::MouseVisible);
    m_SFMLWindow.setActive(m_Flags & Window_Flags::Active);
    m_SFMLWindow.setVerticalSyncEnabled(m_Flags & Window_Flags::Vsync);
    m_SFMLWindow.setMouseCursorGrabbed(m_Flags & Window_Flags::MouseGrabbed);
}
void Window::WindowData::init_position(Window& super) {
    const auto winSize = glm::vec2(super.getSize());
    const auto desktopSize = sf::VideoMode::getDesktopMode();

    float final_desktop_width = static_cast<float>(desktopSize.width);
    float final_desktop_height = static_cast<float>(desktopSize.height);
    float x_other = 0.0f;
    float y_other = 0.0f;
    #ifdef _WIN32
        //get the dimensions of the desktop's bottom task bar. Only tested on Windows 10
        const auto os_handle = super.getSFMLHandle().getSystemHandle();
        //            left   right   top   bottom
        RECT rect; //[0,     1920,   0,    1040]  //bottom task bar
        SystemParametersInfoA(SPI_GETWORKAREA, 0, &rect, 0);
        y_other = final_desktop_height - static_cast<float>(rect.bottom);
        final_desktop_height -= y_other;
    #endif

    super.setPosition(  static_cast<unsigned int>((final_desktop_width - winSize.x) / 2.0f),   static_cast<unsigned int>((final_desktop_height - winSize.y) / 2.0f)   );
}
const sf::ContextSettings Window::WindowData::create(Window& super, const string& name) {
    on_close();

    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        m_WindowThread.startup(super, name);
        std::this_thread::sleep_for(std::chrono::milliseconds(450));
        m_SFMLWindow.setActive(true);
    #else
        m_SFMLWindow.create(m_VideoMode, name, m_Style, m_SFContextSettings);
        if (!m_IconFile.empty())
            super.setIcon(m_IconFile);
        m_UndergoingClosing = false;
    #endif
    return m_SFMLWindow.getSettings();
}
void Window::WindowData::update_mouse_position_internal(Window& super, const float x, const float y, const bool resetDifference, const bool resetPrevious) {
    const auto& sfml_size     = m_SFMLWindow.getSize();
    const auto winSize        = glm::vec2(sfml_size.x, sfml_size.y);
    const glm::vec2 newPos    = glm::vec2(x, winSize.y - y); //opengl flipping y axis
    m_MousePosition_Previous  = (resetPrevious) ? newPos : m_MousePosition;
    m_MousePosition           = newPos;
    m_MouseDifference        += (m_MousePosition - m_MousePosition_Previous);
    if (resetDifference) {
        m_MouseDifference     = glm::vec2(0.0f);
    }
}
void Window::WindowData::on_fullscreen_internal(Window& super, const bool isToBeFullscreen, const bool isMaximized, const bool isMinimized) {
    if (isToBeFullscreen) {
        m_OldWindowSize    = glm::uvec2(m_VideoMode.width, m_VideoMode.height);
        m_VideoMode        = get_default_desktop_video_mode();
    }else{
        m_VideoMode.width  = m_OldWindowSize.x;
        m_VideoMode.height = m_OldWindowSize.y;
    }
    create(super, m_WindowName);  
    m_SFMLWindow.requestFocus();
    priv::Core::m_Engine->m_RenderManager._onFullscreen(m_VideoMode.width, m_VideoMode.height);

    const auto& sfml_size = m_SFMLWindow.getSize();
    const auto winSize    = glm::uvec2(sfml_size.x, sfml_size.y);

    //this does not trigger the sfml event resize method automatically so we must call it here
    priv::Core::m_Engine->on_event_resize(super, winSize.x, winSize.y, false);

    restore_state();
    //TODO: very wierd, but there is an after-effect "reflection" of the last frame on the window if maximize() is called. Commenting out until it is fixed
    /*
    if (isMaximized) {
        maximize();
    }else if (isMinimized) {
        minimize();
    }
    */

    //event dispatch
    priv::EventWindowFullscreenChanged e;
    e.isFullscreen = isToBeFullscreen;
    Event ev(EventType::WindowFullscreenChanged);
    ev.eventWindowFullscreenChanged = e;
    priv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
sf::VideoMode Window::WindowData::get_default_desktop_video_mode() {
    const auto validModes = sf::VideoMode::getFullscreenModes();
    return (validModes.size() > 0) ? validModes[0] : sf::VideoMode::getDesktopMode();
}
void Window::WindowData::on_reset_events(const float& dt) {
    m_MouseDifference.x = 0.0f;
    m_MouseDifference.y = 0.0f;

    const double step   = (1.0 - dt);
    m_MouseDelta       *= (step * step * step);
}

#pragma endregion





#pragma region Window

Window::Window(const EngineOptions& options){
    m_Data.m_WindowName         = options.window_title;

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

    m_Data.m_VideoMode.width         = options.width;
    m_Data.m_VideoMode.height        = options.height;
    m_Data.m_VideoMode.bitsPerPixel  = 32;
    m_Data.m_Style                   = sf::Style::Default;

    m_Data.m_SFContextSettings       = m_Data.create(*this, options.window_title);
    
    unsigned int opengl_version = stoi(to_string(m_Data.m_SFContextSettings.majorVersion) + to_string(m_Data.m_SFContextSettings.minorVersion));
    priv::Core::m_Engine->m_RenderManager._onOpenGLContextCreation(m_Data.m_VideoMode.width, m_Data.m_VideoMode.height, 330, opengl_version);

    m_Data.init_position(*this);

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
        std::cout << "Using OpenGL: " << m_Data.m_SFContextSettings.majorVersion << "." << m_Data.m_SFContextSettings.minorVersion << ", with depth bits: " << m_Data.m_SFContextSettings.depthBits << " and stencil bits: " << m_Data.m_SFContextSettings.stencilBits << std::endl;
    }
}
Window::~Window(){

}
void Window::updateMousePosition(const float x, const float y, const bool resetDifference, const bool resetPrevious) {
    m_Data.update_mouse_position_internal(*this, x, y, resetDifference, resetPrevious);
}
void Window::updateMousePosition(const glm::vec2& position, const bool resetDifference, const bool resetPrevious) {
    m_Data.update_mouse_position_internal(*this, position.x, position.y, resetDifference, resetPrevious);
}
const glm::vec2& Window::getMousePositionDifference() const {
    return m_Data.m_MouseDifference;
}
const glm::vec2& Window::getMousePositionPrevious() const {
    return m_Data.m_MousePosition_Previous;
}
const glm::vec2& Window::getMousePosition() const {
    return m_Data.m_MousePosition;
}
const double& Window::getMouseWheelDelta() const {
    return m_Data.m_MouseDelta;
}
const bool Window::maximize() {
    #ifdef _WIN32
        ::ShowWindow(m_Data.m_SFMLWindow.getSystemHandle(), SW_MAXIMIZE);
        return true;
    #endif
    return false;
}
const bool Window::minimize() {
    #ifdef _WIN32
        ::ShowWindow(m_Data.m_SFMLWindow.getSystemHandle(), SW_MINIMIZE);
        return true;
    #endif
    return false;
}
void Window::setPosition(const unsigned int& x, const unsigned int& y) {
    m_Data.m_SFMLWindow.setPosition(sf::Vector2i(x, y));
}
const glm::uvec2 Window::getPosition() {
    const auto position = m_Data.m_SFMLWindow.getPosition();
    return glm::uvec2(position.x, position.y);
}
const glm::uvec2 Window::getSize(){
    sf::Vector2u window_size = m_Data.m_SFMLWindow.getSize(); 
    return glm::uvec2(window_size.x, window_size.y);
}
void Window::setIcon(const Texture& texture){
    m_Data.m_SFMLWindow.setIcon(texture.width(), texture.height(), const_cast<Texture&>(texture).pixels());
}
void Window::setIcon(const char* file){
    Texture* texture = priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
    if (!texture) {
        texture = NEW Texture(file, false, ImageInternalFormat::RGBA8);
        Handle handle = priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_Data.m_SFMLWindow.setIcon(texture->width(), texture->height(), texture->pixels());
    m_Data.m_IconFile = file;
}
void Window::setIcon(const string& file) {
    Texture* texture = priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
    if (!texture) {
        texture = NEW Texture(file, false, ImageInternalFormat::RGBA8);
        Handle handle = priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_Data.m_SFMLWindow.setIcon(texture->width(), texture->height(), texture->pixels());
    m_Data.m_IconFile = file;
}
const std::string& Window::name() const {
    return m_Data.m_WindowName;
}
void Window::setName(const char* name){
    if (m_Data.m_WindowName == name)
        return;
    m_Data.m_WindowName = name;
    m_Data.m_SFMLWindow.setTitle(name);
}
void Window::setVerticalSyncEnabled(const bool isToBeEnabled){
    if (isToBeEnabled) {
        if (!m_Data.m_Flags.has(Window_Flags::Vsync)) {
            m_Data.m_SFMLWindow.setVerticalSyncEnabled(true);
            m_Data.m_Flags.add(Window_Flags::Vsync);
        }
    }else{
        if (m_Data.m_Flags.has(Window_Flags::Vsync)) {
            m_Data.m_SFMLWindow.setVerticalSyncEnabled(false);
            m_Data.m_Flags.remove(Window_Flags::Vsync);
        }
    }
}
void Window::setKeyRepeatEnabled(const bool isToBeEnabled){
    if (isToBeEnabled) {
        if (!m_Data.m_Flags.has(Window_Flags::KeyRepeat)) {
            m_Data.m_SFMLWindow.setKeyRepeatEnabled(true);
            m_Data.m_Flags.add(Window_Flags::KeyRepeat);
        }
    }else{
        if (m_Data.m_Flags.has(Window_Flags::KeyRepeat)) {
            m_Data.m_SFMLWindow.setKeyRepeatEnabled(false);
            m_Data.m_Flags.remove(Window_Flags::KeyRepeat);
        }
    }
}
void Window::setMouseCursorVisible(const bool isToBeVisible){
    if (isToBeVisible) {
        if (!m_Data.m_Flags.has(Window_Flags::MouseVisible)) {
            #ifdef ENGINE_THREAD_WINDOW_EVENTS
                m_Data.m_WindowThread.push(WindowData::EventThreadOnlyCommands::ShowMouse);
            #else
                m_Data.m_SFMLWindow.setMouseCursorVisible(true);
                m_Data.m_Flags.add(Window_Flags::MouseVisible);
            #endif
        }
    }else{
        if (m_Data.m_Flags.has(Window_Flags::MouseVisible)) {
            #ifdef ENGINE_THREAD_WINDOW_EVENTS
                m_Data.m_WindowThread.push(WindowData::EventThreadOnlyCommands::HideMouse);
            #else
                m_Data.m_SFMLWindow.setMouseCursorVisible(false);
                m_Data.m_Flags.remove(Window_Flags::MouseVisible);
            #endif
        }
    }
}
void Window::requestFocus(){

    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        m_Data.m_WindowThread.push(WindowData::EventThreadOnlyCommands::RequestFocus);
    #else
        m_Data.m_SFMLWindow.requestFocus();
    #endif
}
void Window::close(){
    priv::Core::m_Engine->on_event_window_closed(*this);
    m_Data.m_SFMLWindow.close();
}
const bool Window::isWindowOnSeparateThread() const {
    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        if (m_Data.m_WindowThread == true) {
            return true;
        }
    #endif
    return false;
}
const bool Window::hasFocus() const {
    return m_Data.m_SFMLWindow.hasFocus();
}
const bool Window::isOpen() const {
    return m_Data.m_SFMLWindow.isOpen();
}
const bool Window::isActive() const {
    return m_Data.m_Flags.has(Window_Flags::Active);
}
const bool Window::isFullscreen() const {
    return isFullscreenNonWindowed() || isFullscreenWindowed();
}
const bool Window::isFullscreenWindowed() const {
    return m_Data.m_Flags.has(Window_Flags::WindowedFullscreen);
}
const bool Window::isFullscreenNonWindowed() const {
    return m_Data.m_Flags.has(Window_Flags::Fullscreen);
}
const bool Window::isMouseKeptInWindow() const {
    return m_Data.m_Flags.has(Window_Flags::MouseGrabbed);
}
void Window::display(){
    m_Data.m_SFMLWindow.display();
}
const bool Window::isMaximized() const {
    #ifdef _WIN32
        WINDOWPLACEMENT info;
        info.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(getSFMLHandle().getSystemHandle(), &info);
        if (info.showCmd == SW_MAXIMIZE)
            return true;
    #endif
    return false;
}
const bool Window::isMinimized() const {
    #ifdef _WIN32
        WINDOWPLACEMENT info;
        info.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(getSFMLHandle().getSystemHandle(), &info);
        if (info.showCmd == SW_MINIMIZE)
            return true;
    #endif
    return false;
}
void Window::setActive(const bool isToBeActive){
    if (isToBeActive) {
        if (!m_Data.m_Flags.has(Window_Flags::Active)) {
            m_Data.m_SFMLWindow.setActive(true);
            m_Data.m_Flags.add(Window_Flags::Active);
        }
    }else{
        if (m_Data.m_Flags.has(Window_Flags::Active)) {
            m_Data.m_SFMLWindow.setActive(false);
            m_Data.m_Flags.remove(Window_Flags::Active);
        }
    }
}
void Window::setSize(const unsigned int& width, const unsigned int& height){
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
void Window::restore_state() {
    m_Data.restore_state();
}
const bool Window::setFullscreenWindowed(const bool isToBeFullscreen) {
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
    const bool old_max = isMaximized();
    const bool old_min = isMinimized();
    m_Data.on_fullscreen_internal(*this, isToBeFullscreen, old_max, old_min);
    return true;
}
const bool Window::setFullscreen(const bool isToBeFullscreen){
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
    const bool old_max = isMaximized();
    const bool old_min = isMinimized();
    m_Data.on_fullscreen_internal(*this, isToBeFullscreen, old_max, old_min);
    return true;
}
void Window::keepMouseInWindow(const bool isToBeKept){
    if (isToBeKept) {
        #ifdef ENGINE_THREAD_WINDOW_EVENTS
            m_Data.m_WindowThread.push(WindowData::EventThreadOnlyCommands::KeepMouseInWindow);
        #else
            if (!m_Data.m_Flags.has(Window_Flags::MouseGrabbed)) {
                m_Data.m_SFMLWindow.setMouseCursorGrabbed(true);
                m_Data.m_Flags.add(Window_Flags::MouseGrabbed);
            }
        #endif
    }else{
        #ifdef ENGINE_THREAD_WINDOW_EVENTS
            m_Data.m_WindowThread.push(WindowData::EventThreadOnlyCommands::FreeMouseFromWindow);
        #else
            if (m_Data.m_Flags.has(Window_Flags::MouseGrabbed)) {
                m_Data.m_SFMLWindow.setMouseCursorGrabbed(false);
                m_Data.m_Flags.remove(Window_Flags::MouseGrabbed);
            }
        #endif
    }
}
void Window::setFramerateLimit(const unsigned int& limit){
    m_Data.m_SFMLWindow.setFramerateLimit(limit);
    m_Data.m_FramerateLimit = limit;
}
sf::Window& Window::getSFMLHandle() const {
    return const_cast<sf::Window&>(m_Data.m_SFMLWindow);
}
const unsigned int& Window::getFramerateLimit() const{
    return m_Data.m_FramerateLimit;
}
void Window::on_dynamic_resize() {
    #ifdef _WIN32
        WINDOWINFO wiInfo;
        GetWindowInfo(m_Data.m_SFMLWindow.getSystemHandle(), &wiInfo);
        const glm::uvec2 current_size = glm::uvec2(wiInfo.rcClient.right - wiInfo.rcClient.left, wiInfo.rcClient.bottom - wiInfo.rcClient.top);
        const glm::uvec2 old_size = getSize();

        if (current_size.x != old_size.x || current_size.y != old_size.y) {
            setSize(current_size.x, current_size.y);
            priv::Core::m_Engine->on_event_resize(*this, current_size.x, current_size.y, true);
        }
    #endif
}




const bool Window::pollEvents(sf::Event& e) {
    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        auto x = m_Data.m_WindowThread.try_pop(); //expensive as it uses lock & mutex
        if (x) {
            e = std::move(*x);
            return true;
        }
        return false;
    #else
        return m_Data.m_SFMLWindow.pollEvent(e);
    #endif
}



#pragma endregion