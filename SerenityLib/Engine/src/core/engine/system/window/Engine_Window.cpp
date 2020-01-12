#include <core/engine/system/Engine.h>
#include <core/engine/system/window/Engine_Window.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/events/Engine_EventDispatcher.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Viewport.h>

#include <ecs/ECS.h>
#include <ecs/ComponentCamera.h>

#include <iostream>

using namespace Engine;
using namespace std;

Engine_Window::Engine_WindowData::Engine_WindowData() {
    m_FramerateLimit     = 0;
    m_OldWindowSize = glm::uvec2(0, 0);
    m_Flags = (Engine_Window_Flags::Windowed | Engine_Window_Flags::MouseVisible | Engine_Window_Flags::Active);
}
Engine_Window::Engine_WindowData::~Engine_WindowData() {

}
void Engine_Window::Engine_WindowData::restore_state() {
    if (m_FramerateLimit > 0) {
        m_SFMLWindow.setFramerateLimit(m_FramerateLimit);
    }
    m_SFContextSettings = m_SFMLWindow.getSettings();

    m_SFMLWindow.setMouseCursorVisible(m_Flags & Engine_Window_Flags::MouseVisible);
    m_SFMLWindow.setActive(m_Flags & Engine_Window_Flags::Active);
    m_SFMLWindow.setVerticalSyncEnabled(m_Flags & Engine_Window_Flags::Vsync);
    m_SFMLWindow.setMouseCursorGrabbed(m_Flags & Engine_Window_Flags::MouseGrabbed);
}
const sf::ContextSettings Engine_Window::Engine_WindowData::create(Engine_Window& super, const string& _name, const unsigned int& _width, const unsigned int& _height) {
    sf::ContextSettings settings;
    settings.depthBits         = 24;
    settings.stencilBits       = 0;
    settings.antialiasingLevel = 0;
    settings.majorVersion      = m_OpenGLMajorVersion;
    settings.minorVersion      = m_OpenGLMinorVersion;

#ifdef _DEBUG
    settings.attributeFlags = settings.Debug;
#else
    settings.attributeFlags = settings.Core;
#endif

    m_OldWindowSize = glm::uvec2(m_VideoMode.width, m_VideoMode.height);

    m_VideoMode.width        = _width;
    m_VideoMode.height       = _height;
    m_VideoMode.bitsPerPixel = 32;
    m_Style                  = sf::Style::Default;
    if (_width == 0 || _height == 0) {
        m_Style = sf::Style::None;
        m_VideoMode = super.get_default_desktop_video_mode();
    }

    m_SFMLWindow.create(m_VideoMode, _name, m_Style, settings);
    unsigned int opengl_version = stoi(to_string(settings.majorVersion) + to_string(settings.minorVersion));
    epriv::Core::m_Engine->m_RenderManager._onOpenGLContextCreation(m_VideoMode.width, m_VideoMode.height, m_GLSLVersion, opengl_version);

    return m_SFMLWindow.getSettings();
}

Engine_Window::Engine_Window(const char* name, const unsigned int& width, const unsigned int& height){
    m_Data.m_WindowName         = name;
    m_Data.m_OpenGLMajorVersion = 4;
    m_Data.m_OpenGLMinorVersion = 6;
    m_Data.m_GLSLVersion        = 330;
    m_Data.m_SFContextSettings  = m_Data.create(*this, name, width, height);
    
    std::cout << "Using OpenGL: " << 
        m_Data.m_SFContextSettings.majorVersion << "." << 
        m_Data.m_SFContextSettings.minorVersion << ", with depth bits: " << 
        m_Data.m_SFContextSettings.depthBits << " and stencil bits: " << 
        m_Data.m_SFContextSettings.stencilBits << std::endl;
}
Engine_Window::~Engine_Window(){

}
const bool Engine_Window::maximize() {
    #ifdef _WIN32
        //if (!has_flag(Engine_Window_Flags::Maximized)) {
            ::ShowWindow(m_Data.m_SFMLWindow.getSystemHandle(), SW_MAXIMIZE);
        //    add_flag(Engine_Window_Flags::Maximized);
        //    remove_flag(Engine_Window_Flags::Minimized);
            return true;
        //}
    #endif
    //return false;
}
const bool Engine_Window::minimize() {
    #ifdef _WIN32
        //if (!has_flag(Engine_Window_Flags::Minimized)) {
            ::ShowWindow(m_Data.m_SFMLWindow.getSystemHandle(), SW_MINIMIZE);
        //    add_flag(Engine_Window_Flags::Minimized);
        //    remove_flag(Engine_Window_Flags::Maximized);
            return true;
        //}
    #endif
    //return false;
}
void Engine_Window::setPosition(const unsigned int& x, const unsigned int& y) {
    m_Data.m_SFMLWindow.setPosition(sf::Vector2i(x, y));
}
const glm::uvec2 Engine_Window::getPosition() {
    const auto position = m_Data.m_SFMLWindow.getPosition();
    return glm::uvec2(position.x, position.y);
}
const glm::uvec2 Engine_Window::getSize(){ 
    sf::Vector2u window_size = m_Data.m_SFMLWindow.getSize(); 
    return glm::uvec2(window_size.x, window_size.y);
}
void Engine_Window::setIcon(const Texture& texture){
    m_Data.m_SFMLWindow.setIcon(texture.width(), texture.height(), const_cast<Texture&>(texture).pixels());
}
void Engine_Window::setIcon(const char* file){
    Texture* texture = epriv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
    if (!texture) {
        texture = NEW Texture(file, false, ImageInternalFormat::RGBA8);
        Handle handle = epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_Data.m_SFMLWindow.setIcon(texture->width(), texture->height(), texture->pixels());
}
void Engine_Window::setIcon(const string& file) { 
    Texture* texture = epriv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
    if (!texture) {
        texture = NEW Texture(file, false, ImageInternalFormat::RGBA8);
        Handle handle = epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_Data.m_SFMLWindow.setIcon(texture->width(), texture->height(), texture->pixels());
}
const char* Engine_Window::name() const {
    return m_Data.m_WindowName;
}
void Engine_Window::setName(const char* name){
    if (m_Data.m_WindowName == name)
        return;
    m_Data.m_WindowName = name;
    m_Data.m_SFMLWindow.setTitle(name);
}
void Engine_Window::setVerticalSyncEnabled(const bool isToBeEnabled){
    if (isToBeEnabled) {
        if (!has_flag(Engine_Window_Flags::Vsync)) {
            m_Data.m_SFMLWindow.setVerticalSyncEnabled(true);
            add_flag(Engine_Window_Flags::Vsync);
        }
    }else{
        if (has_flag(Engine_Window_Flags::Vsync)) {
            m_Data.m_SFMLWindow.setVerticalSyncEnabled(false);
            remove_flag(Engine_Window_Flags::Vsync);
        }
    }
}
void Engine_Window::setKeyRepeatEnabled(const bool isToBeEnabled){
    if (isToBeEnabled) {
        if (!has_flag(Engine_Window_Flags::KeyRepeat)) {
            m_Data.m_SFMLWindow.setKeyRepeatEnabled(true);
            add_flag(Engine_Window_Flags::KeyRepeat);
        }
    }else{
        if (has_flag(Engine_Window_Flags::KeyRepeat)) {
            m_Data.m_SFMLWindow.setKeyRepeatEnabled(false);
            remove_flag(Engine_Window_Flags::KeyRepeat);
        }
    }
}
void Engine_Window::setMouseCursorVisible(const bool isToBeVisible){
    if (isToBeVisible) {
        if (!has_flag(Engine_Window_Flags::MouseVisible)) {
            m_Data.m_SFMLWindow.setMouseCursorVisible(true);
            add_flag(Engine_Window_Flags::MouseVisible);
        }
    }else{
        if (has_flag(Engine_Window_Flags::MouseVisible)) {
            m_Data.m_SFMLWindow.setMouseCursorVisible(false);
            remove_flag(Engine_Window_Flags::MouseVisible);
        }
    }
}
void Engine_Window::requestFocus(){
    m_Data.m_SFMLWindow.requestFocus();
}
void Engine_Window::close(){
    m_Data.m_SFMLWindow.close();
}
const bool Engine_Window::hasFocus(){
    return m_Data.m_SFMLWindow.hasFocus();
}
const bool Engine_Window::isOpen(){
    return m_Data.m_SFMLWindow.isOpen();
}
const bool Engine_Window::isActive(){ 
    return has_flag(Engine_Window_Flags::Active);
}
const bool Engine_Window::isFullscreen(){
    return isFullscreenNonWindowed() || isFullscreenWindowed();
}
const bool Engine_Window::isFullscreenWindowed() {
    return has_flag(Engine_Window_Flags::WindowedFullscreen);
}
const bool Engine_Window::isFullscreenNonWindowed() {
    return has_flag(Engine_Window_Flags::Fullscreen);
}
void Engine_Window::display(){
    m_Data.m_SFMLWindow.display();
}
const bool Engine_Window::isMaximized() {
    //return has_flag(Engine_Window_Flags::Maximized);
    #ifdef _WIN32
        WINDOWPLACEMENT info;
        info.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(getSFMLHandle().getSystemHandle(), &info);
        if (info.showCmd == SW_MAXIMIZE)
            return true;
    #endif
    return false;
}
const bool Engine_Window::isMinimized() {
    //return has_flag(Engine_Window_Flags::Minimized);
    #ifdef _WIN32
        WINDOWPLACEMENT info;
        info.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(getSFMLHandle().getSystemHandle(), &info);
        if (info.showCmd == SW_MINIMIZE)
            return true;
    #endif
    return false;
}
void Engine_Window::setActive(const bool isToBeActive){
    if (isToBeActive) {
        if (!has_flag(Engine_Window_Flags::Active)) {
            m_Data.m_SFMLWindow.setActive(true);
            add_flag(Engine_Window_Flags::Active);
        }
    }else{
        if (has_flag(Engine_Window_Flags::Active)) {
            m_Data.m_SFMLWindow.setActive(false);
            remove_flag(Engine_Window_Flags::Active);
        }
    }
}
void Engine_Window::setSize(const unsigned int& width, const unsigned int& height){
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
void Engine_Window::restore_state() {
    m_Data.restore_state();
}
void Engine_Window::setStyle(const unsigned int& style){
    if (m_Data.m_Style == style)
        return;
    m_Data.m_Style = style;
    m_Data.m_SFMLWindow.close();
    m_Data.m_SFContextSettings = m_Data.create(*this, m_Data.m_WindowName, m_Data.m_VideoMode.width, m_Data.m_VideoMode.height);
}

const bool Engine_Window::remove_flag(const Engine_Window_Flags::Flag& flag) {
    if (m_Data.m_Flags != (m_Data.m_Flags & ~flag)) {
        m_Data.m_Flags = m_Data.m_Flags & ~flag;
        return true;
    }
    return false;
}
const bool Engine_Window::add_flag(const Engine_Window_Flags::Flag& flag) {
    if (m_Data.m_Flags != (m_Data.m_Flags | flag)) {
        m_Data.m_Flags = m_Data.m_Flags | flag;
        return true;
    }
    return false;
}
const bool Engine_Window::has_flag(const Engine_Window_Flags::Flag& flag) {
    return (m_Data.m_Flags & flag) ? true : false;
}
const bool Engine_Window::setFullscreenWindowed(const bool isToBeFullscreen) {
    if (isToBeFullscreen) {
        if (isFullscreenWindowed())
            return false;
    }else{
        if (!isFullscreenWindowed())
            return false;
    }
    //if (!m_Data.m_SFMLWindow.hasFocus())
        //return false;

    const bool old_max = isMaximized();
    const bool old_min = isMinimized();

    if (!isToBeFullscreen) {
        m_Data.m_Style = sf::Style::Default; //windowed
        add_flag(Engine_Window_Flags::Windowed);
        remove_flag(Engine_Window_Flags::Fullscreen);
        remove_flag(Engine_Window_Flags::WindowedFullscreen);
    }else{
        m_Data.m_Style = sf::Style::None;    //windowed_fullscreen
        add_flag(Engine_Window_Flags::WindowedFullscreen);
        remove_flag(Engine_Window_Flags::Fullscreen);
        remove_flag(Engine_Window_Flags::Windowed);
    }
    internal_on_fullscreen(isToBeFullscreen, old_max, old_min);
    return true;
}
const bool Engine_Window::setFullscreen(const bool isToBeFullscreen){
    if (isToBeFullscreen) {
        if (isFullscreenNonWindowed())
            return false;
    }else{
        if (!isFullscreenNonWindowed())
            return false;
    }
    //if (!m_Data.m_SFMLWindow.hasFocus())
        //return false;

    const bool old_max = isMaximized();
    const bool old_min = isMinimized();

    if (!isToBeFullscreen) {
        m_Data.m_Style = sf::Style::Default;    //windowed
        add_flag(Engine_Window_Flags::Windowed);
        remove_flag(Engine_Window_Flags::Fullscreen);
        remove_flag(Engine_Window_Flags::WindowedFullscreen);
    }else{
        m_Data.m_Style = sf::Style::Fullscreen; //fullscreen      
        add_flag(Engine_Window_Flags::Fullscreen);
        remove_flag(Engine_Window_Flags::WindowedFullscreen);
        remove_flag(Engine_Window_Flags::Windowed);
    }
    internal_on_fullscreen(isToBeFullscreen, old_max, old_min);
    return true;
}
sf::VideoMode Engine_Window::get_default_desktop_video_mode() {
    const auto validModes = sf::VideoMode::getFullscreenModes();
    return (validModes.size() > 0) ? validModes[0] : sf::VideoMode::getDesktopMode();
}
void Engine_Window::internal_on_fullscreen(const bool isToBeFullscreen, const bool isMaximized, const bool isMinimized) {
    if (isToBeFullscreen) {
        m_Data.m_OldWindowSize = glm::uvec2(m_Data.m_VideoMode.width, m_Data.m_VideoMode.height);
        m_Data.m_VideoMode = get_default_desktop_video_mode();
    }else{
        m_Data.m_VideoMode.width = m_Data.m_OldWindowSize.x;
        m_Data.m_VideoMode.height = m_Data.m_OldWindowSize.y;
    }
    m_Data.m_SFMLWindow.create(m_Data.m_VideoMode, m_Data.m_WindowName, m_Data.m_Style, m_Data.m_SFContextSettings);

    epriv::Core::m_Engine->m_RenderManager._onFullscreen();

    glm::uvec2 winSize = Engine::getWindowSize();

    //this does not trigger the event resize method automatically...
    epriv::Core::m_Engine->on_event_resize(winSize.x, winSize.y, false);

    restore_state();
    //TODO: very wierd, but there is an after-effect "reflection" of the last frame on the window if maximize() is called. Commenting out until it is fixed
    /*
    if (isMaximized) {
        Engine_Window::maximize();
    }else if (isMinimized) {
        Engine_Window::minimize();
    }
    */


    //event dispatch
    epriv::EventWindowFullscreenChanged e;
    e.isFullscreen = isToBeFullscreen;
    Event ev;
    ev.eventWindowFullscreenChanged = e;
    ev.type = EventType::WindowFullscreenChanged;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void Engine_Window::keepMouseInWindow(const bool isToBeKept){
    if (isToBeKept) {
        if (!has_flag(Engine_Window_Flags::MouseGrabbed)) {
            m_Data.m_SFMLWindow.setMouseCursorGrabbed(true);
            add_flag(Engine_Window_Flags::MouseGrabbed);
        }
    }else{
        if (has_flag(Engine_Window_Flags::MouseGrabbed)) {
            m_Data.m_SFMLWindow.setMouseCursorGrabbed(false);
            remove_flag(Engine_Window_Flags::MouseGrabbed);
        }
    }
}
void Engine_Window::setFramerateLimit(const unsigned int& limit){
    m_Data.m_SFMLWindow.setFramerateLimit(limit);
    m_Data.m_FramerateLimit = limit;
}
sf::Window& Engine_Window::getSFMLHandle() const { 
    return const_cast<sf::Window&>(m_Data.m_SFMLWindow);
}
const unsigned int Engine_Window::getStyle(){
    return m_Data.m_Style;
}
const unsigned int Engine_Window::getFramerateLimit() const{
    return m_Data.m_FramerateLimit;
}