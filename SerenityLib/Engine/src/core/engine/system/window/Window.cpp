#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/system/Engine.h>
#include <core/engine/system/EngineOptions.h>
#include <core/engine/renderer/opengl/OpenGL.h>
#include <core/engine/system/window/Window.h>
#include <core/engine/textures/Texture.h>

using namespace Engine;
using namespace std;

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
    


    unsigned int requested_glsl_version = stoi(Engine::priv::OpenGL::getHighestGLSLVersion(*this));
 
    unsigned int opengl_version = stoi(to_string(m_Data.m_SFContextSettings.majorVersion) + to_string(m_Data.m_SFContextSettings.minorVersion));
    priv::Core::m_Engine->m_RenderManager._onOpenGLContextCreation(m_Data.m_VideoMode.width, m_Data.m_VideoMode.height, requested_glsl_version, opengl_version);

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
        std::cout << "Using OpenGL: " << m_Data.m_SFContextSettings.majorVersion << "." << m_Data.m_SFContextSettings.minorVersion << 
            ", with depth bits: " << m_Data.m_SFContextSettings.depthBits << 
            " and stencil bits: " << m_Data.m_SFContextSettings.stencilBits << 
            " and glsl version: " << requested_glsl_version << std::endl;
    }
}
Window::~Window(){

}
void Window::setJoystickProcessingActive(bool active) {
    m_Data.m_SFMLWindow.setJoystickManagerActive(active);
}
bool Window::isJoystickProcessingActive() const {
    return m_Data.m_SFMLWindow.isJoystickManagerActive();
}
void Window::updateMousePosition(float x, float y, bool resetDifference, bool resetPrevious) {
    m_Data.update_mouse_position_internal(*this, x, y, resetDifference, resetPrevious);
}
void Window::updateMousePosition(const glm::vec2& position, bool resetDifference, bool resetPrevious) {
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
double Window::getMouseWheelDelta() const {
    return m_Data.m_MouseDelta;
}
std::thread::id Window::getOpenglThreadID() const {
    return m_Data.m_OpenGLThreadID;
}

bool Window::maximize() {
    #ifdef _WIN32
        ::ShowWindow(m_Data.m_SFMLWindow.getSystemHandle(), SW_MAXIMIZE);
        return true;
    #endif
    return false;
}
bool Window::minimize() {
    #ifdef _WIN32
        ::ShowWindow(m_Data.m_SFMLWindow.getSystemHandle(), SW_MINIMIZE);
        return true;
    #endif
    return false;
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
void Window::setVerticalSyncEnabled(bool isToBeEnabled){
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
void Window::setKeyRepeatEnabled(bool isToBeEnabled){
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
void Window::setMouseCursorVisible(bool isToBeVisible){
    if (isToBeVisible) {
        if (!m_Data.m_Flags.has(Window_Flags::MouseVisible)) {
            #ifdef ENGINE_THREAD_WINDOW_EVENTS
                m_Data.m_WindowThread.push(WindowEventThreadOnlyCommands::ShowMouse);
            #else
                m_Data.m_SFMLWindow.setMouseCursorVisible(true);
                m_Data.m_Flags.add(Window_Flags::MouseVisible);
            #endif
        }
    }else{
        if (m_Data.m_Flags.has(Window_Flags::MouseVisible)) {
            #ifdef ENGINE_THREAD_WINDOW_EVENTS
                m_Data.m_WindowThread.push(WindowEventThreadOnlyCommands::HideMouse);
            #else
                m_Data.m_SFMLWindow.setMouseCursorVisible(false);
                m_Data.m_Flags.remove(Window_Flags::MouseVisible);
            #endif
        }
    }
}
void Window::requestFocus(){

    #ifdef ENGINE_THREAD_WINDOW_EVENTS
        m_Data.m_WindowThread.push(WindowEventThreadOnlyCommands::RequestFocus);
    #else
        m_Data.m_SFMLWindow.requestFocus();
    #endif
}
void Window::close(){
    priv::Core::m_Engine->on_event_window_closed(*this);
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
void Window::display(){
    m_Data.m_SFMLWindow.display();
}
bool Window::isMaximized() const {
    #ifdef _WIN32
        WINDOWPLACEMENT info;
        info.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(getSystemHandle(), &info);
        if (info.showCmd == SW_MAXIMIZE)
            return true;
    #endif
    return false;
}
bool Window::isMinimized() const {
    #ifdef _WIN32
        WINDOWPLACEMENT info;
        info.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(getSystemHandle(), &info);
        if (info.showCmd == SW_MINIMIZE)
            return true;
    #endif
    return false;
}
void Window::setActive(bool isToBeActive){
    if (isToBeActive) {
        if (!m_Data.m_Flags.has(Window_Flags::Active)) {
            m_Data.m_SFMLWindow.setActive(true);
            m_Data.m_OpenGLThreadID = std::this_thread::get_id();
            m_Data.m_Flags.add(Window_Flags::Active);
        }
    }else{
        if (m_Data.m_Flags.has(Window_Flags::Active)) {
            m_Data.m_SFMLWindow.setActive(false);
            m_Data.m_Flags.remove(Window_Flags::Active);
        }
    }
}
void Window::setSize(unsigned int width, unsigned int height){
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
    m_Data.restore_state(*this);
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
    m_Data.on_fullscreen_internal(*this, isToBeFullscreen, old_max, old_min);
    return true;
}
bool Window::setFullscreen(bool isToBeFullscreen){
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
    m_Data.on_fullscreen_internal(*this, isToBeFullscreen, old_max, old_min);
    return true;
}
void Window::keepMouseInWindow(bool isToBeKept){
    if (isToBeKept) {
        #ifdef ENGINE_THREAD_WINDOW_EVENTS
            m_Data.m_WindowThread.push(WindowEventThreadOnlyCommands::KeepMouseInWindow);
        #else
            if (!m_Data.m_Flags.has(Window_Flags::MouseGrabbed)) {
                m_Data.m_SFMLWindow.setMouseCursorGrabbed(true);
                m_Data.m_Flags.add(Window_Flags::MouseGrabbed);
            }
        #endif
    }else{
        #ifdef ENGINE_THREAD_WINDOW_EVENTS
            m_Data.m_WindowThread.push(WindowEventThreadOnlyCommands::FreeMouseFromWindow);
        #else
            if (m_Data.m_Flags.has(Window_Flags::MouseGrabbed)) {
                m_Data.m_SFMLWindow.setMouseCursorGrabbed(false);
                m_Data.m_Flags.remove(Window_Flags::MouseGrabbed);
            }
        #endif
    }
}
void Window::setFramerateLimit(unsigned int limit){
    m_Data.m_SFMLWindow.setFramerateLimit(limit);
    m_Data.m_FramerateLimit = limit;
}
sf::WindowHandle Window::getSystemHandle() const {
    return m_Data.m_SFMLWindow.getSystemHandle();
}
sf::RenderWindow& Window::getSFMLHandle() {
    return m_Data.m_SFMLWindow;
}
unsigned int Window::getFramerateLimit() const {
    return m_Data.m_FramerateLimit;
}
void Window::on_dynamic_resize() {
    #ifdef _WIN32
        WINDOWINFO wiInfo;
        GetWindowInfo(m_Data.m_SFMLWindow.getSystemHandle(), &wiInfo);
        const glm::uvec2 current_size = glm::uvec2(wiInfo.rcClient.right - wiInfo.rcClient.left, wiInfo.rcClient.bottom - wiInfo.rcClient.top);
        const glm::uvec2 old_size     = getSize();

        if (current_size.x != old_size.x || current_size.y != old_size.y) {
            Window::setSize(current_size.x, current_size.y);
            priv::Core::m_Engine->on_event_resize(*this, current_size.x, current_size.y, true);
        }
    #endif
}
bool Window::pollEvents(sf::Event& e) {
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