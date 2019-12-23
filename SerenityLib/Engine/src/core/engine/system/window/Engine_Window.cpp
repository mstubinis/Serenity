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
    m_MouseCursorVisible = true;
    m_Fullscreen         = false;
    m_Vsync              = false;
    m_MouseCursorGrabbed = false;
}
Engine_Window::Engine_WindowData::~Engine_WindowData() {

}
void Engine_Window::Engine_WindowData::restore_state() {
    if (m_FramerateLimit > 0) {
        m_SFMLWindow.setFramerateLimit(m_FramerateLimit);
    }
    m_SFContextSettings = m_SFMLWindow.getSettings();
    m_SFMLWindow.setMouseCursorVisible(m_MouseCursorVisible);
    m_SFMLWindow.setActive(m_Active);
    m_SFMLWindow.setVerticalSyncEnabled(m_Vsync);
    m_SFMLWindow.setMouseCursorGrabbed(m_MouseCursorGrabbed);
}
const sf::ContextSettings Engine_Window::Engine_WindowData::create(const string& _name, const unsigned int& _width, const unsigned int& _height) {
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

    m_VideoMode.width        = _width;
    m_VideoMode.height       = _height;
    m_VideoMode.bitsPerPixel = 32;
    m_Style                  = sf::Style::Default;
    if (_width == 0 || _height == 0) {
        m_Style = sf::Style::None;
        m_VideoMode = sf::VideoMode::getDesktopMode();
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
    m_Data.m_SFContextSettings  = m_Data.create(name, width, height);
    
    std::cout << "Using OpenGL: " << 
        m_Data.m_SFContextSettings.majorVersion << "." << 
        m_Data.m_SFContextSettings.minorVersion << ", with depth bits: " << 
        m_Data.m_SFContextSettings.depthBits << " and stencil bits: " << 
        m_Data.m_SFContextSettings.stencilBits << std::endl;
}
Engine_Window::~Engine_Window(){

}
void Engine_Window::maximize() {
    #ifdef _WIN32
        ::ShowWindow(m_Data.m_SFMLWindow.getSystemHandle(), SW_MAXIMIZE);
    #endif
}
void Engine_Window::minimize() {
    #ifdef _WIN32
        ::ShowWindow(m_Data.m_SFMLWindow.getSystemHandle(), SW_MINIMIZE);
    #endif
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
void Engine_Window::setVerticalSyncEnabled(const bool enabled){
    if (m_Data.m_Vsync != enabled) {
        m_Data.m_SFMLWindow.setVerticalSyncEnabled(enabled);
        m_Data.m_Vsync = enabled;
    }
}
void Engine_Window::setKeyRepeatEnabled(const bool enabled){
    m_Data.m_SFMLWindow.setKeyRepeatEnabled(enabled);
}
void Engine_Window::setMouseCursorVisible(const bool visible){
    if (m_Data.m_MouseCursorVisible != visible) {
        m_Data.m_SFMLWindow.setMouseCursorVisible(visible);
        m_Data.m_MouseCursorVisible = visible;
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
    return m_Data.m_Active;
}
const bool Engine_Window::isFullscreen(){
    return m_Data.m_Fullscreen;
}
void Engine_Window::display(){
    m_Data.m_SFMLWindow.display();
}
void Engine_Window::setActive(const bool active){
    m_Data.m_Active = active;
    m_Data.m_SFMLWindow.setActive(active);
}
void Engine_Window::setSize(const unsigned int& width, const unsigned int& height){
    m_Data.m_VideoMode.width  = width;
    m_Data.m_VideoMode.height = height;
    m_Data.m_SFMLWindow.setSize(sf::Vector2u(width, height));
}
void Engine_Window::setStyle(const unsigned int& style){
    if (m_Data.m_Style == style)
        return;
    m_Data.m_Style = style;
    m_Data.m_SFMLWindow.close();
    m_Data.m_SFContextSettings = m_Data.create(m_Data.m_WindowName, m_Data.m_VideoMode.width, m_Data.m_VideoMode.height);
}
void Engine_Window::setFullScreen(const bool fullscreen, const bool isWindowedMode){
    if (m_Data.m_Fullscreen == fullscreen)
        return;
    if ((isWindowedMode && m_Data.m_Style == sf::Style::None) && fullscreen)
        return;
    if ((!isWindowedMode && m_Data.m_Style == sf::Style::Fullscreen) && fullscreen)
        return;
    if ((m_Data.m_Style != sf::Style::Fullscreen && m_Data.m_Style != sf::Style::None) && !fullscreen)
        return;
    if (!m_Data.m_SFMLWindow.hasFocus())
        return;

    auto validModes = sf::VideoMode::getFullscreenModes();
    validModes.size() > 0 ? m_Data.m_VideoMode = validModes[0] : m_Data.m_VideoMode = sf::VideoMode::getDesktopMode();

    isWindowedMode ? m_Data.m_Style = sf::Style::None : m_Data.m_Style = sf::Style::Fullscreen;
    if (!fullscreen) {
        m_Data.m_Style = sf::Style::Default;
    }
    epriv::Core::m_Engine->m_RenderManager._onFullscreen(&m_Data.m_SFMLWindow, m_Data.m_VideoMode, m_Data.m_WindowName, m_Data.m_Style, m_Data.m_SFContextSettings);

    glm::uvec2 winSize = Engine::getWindowSize();

    //i wish this would trigger the event resize method automatically...
    epriv::Core::m_Engine->on_event_resize(winSize.x, winSize.y, false);

    //restore state machine
    m_Data.restore_state();
    m_Data.m_Fullscreen = fullscreen;

    //event dispatch
    epriv::EventWindowFullscreenChanged e;
    e.isFullscreen = fullscreen;
    Event ev;
    ev.eventWindowFullscreenChanged = e;
    ev.type = EventType::WindowFullscreenChanged;
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
}
void Engine_Window::keepMouseInWindow(const bool keep){
    if (m_Data.m_MouseCursorGrabbed != keep) {
        m_Data.m_SFMLWindow.setMouseCursorGrabbed(keep);
        m_Data.m_MouseCursorGrabbed = keep;
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