#include "Engine.h"
#include "Engine_Window.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "GBuffer.h"
#include <SFML/Window.hpp>
#include <string>

using namespace Engine;

#ifdef _WIN32
void Engine_Window::_createDirectXWindow(const char* name,uint width,uint height){

}
void Engine_Window::setRenderingAPI(uint api){
    //do alot of crap here...
    if(api == ENGINE_RENDERING_API_OPENGL){
    }
    else if(api == ENGINE_RENDERING_API_DIRECTX){
    }
	Resources::Detail::ResourceManagement::m_RenderingAPI = static_cast<ENGINE_RENDERING_API>(api);
}
#endif
void Engine_Window::_destroyOpenGLContext(){
    HGLRC    hglrc; 
    HDC      hdc ; 
    if(hglrc = wglGetCurrentContext()){ 
        hdc = wglGetCurrentDC(); 
        wglMakeCurrent(0, 0); 
		ReleaseDC (m_SFMLWindow->getSystemHandle(), hdc); 
        wglDeleteContext(hglrc); 
    }
}
Engine_Window::Engine_Window(const char* name,uint width,uint height,uint api){
    m_WindowName = name;
    m_Width = width; m_Height = height;
    m_SFMLWindow = new sf::Window();

	_destroyOpenGLContext();

	if(api == ENGINE_RENDERING_API_OPENGL){
		this->_createOpenGLWindow(name,width,height);
	}
	else if(api == ENGINE_RENDERING_API_DIRECTX){
		this->_createOpenGLWindow(name,width,height);
		_destroyOpenGLContext();
		m_SFMLWindow->requestFocus();
		this->_createDirectXWindow(name,width,height);
	}
    setMouseCursorVisible(false);
    setKeyRepeatEnabled(false);
}
void Engine_Window::_createOpenGLWindow(const char* name,uint width,uint height){
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
    settings.majorVersion = 3;
    settings.minorVersion = 0;

    m_VideoMode.width = m_Width;
    m_VideoMode.height = m_Height;
    m_VideoMode.bitsPerPixel = 32;

    m_Style = sf::Style::Default;
    if(m_Width == 0 || m_Height == 0){
        m_Style = sf::Style::Fullscreen;
        m_VideoMode = sf::VideoMode::getDesktopMode();
        m_Width = m_VideoMode.width;
        m_Height = m_VideoMode.height;
    }
    m_SFMLWindow->create(m_VideoMode,name,m_Style,settings);
}
Engine_Window::~Engine_Window(){
    delete(m_SFMLWindow);
}
sf::Vector2u Engine_Window::getSize(){
    return m_SFMLWindow->getSize();
}
void Engine_Window::setIcon(uint width, uint height, const sf::Uint8* pixels){
    m_SFMLWindow->setIcon(width,height,pixels);
}
const char* Engine_Window::name(){
    return m_WindowName;
}
void Engine_Window::setName(const char* name){
    m_WindowName = name;
    m_SFMLWindow->setTitle(m_WindowName);
}
void Engine_Window::setVerticalSyncEnabled(bool enabled){
    m_SFMLWindow->setVerticalSyncEnabled(enabled);
}
void Engine_Window::setKeyRepeatEnabled(bool enabled){
    m_SFMLWindow->setKeyRepeatEnabled(enabled);
}
void Engine_Window::setMouseCursorVisible(bool visible){
    m_SFMLWindow->setMouseCursorVisible(visible);
}
void Engine_Window::requestFocus(){
    m_SFMLWindow->requestFocus();
}
void Engine_Window::close(){
    m_SFMLWindow->close();
}
bool Engine_Window::hasFocus(){
    return m_SFMLWindow->hasFocus();
}
bool Engine_Window::isOpen(){
    return m_SFMLWindow->isOpen();
}
bool Engine_Window::pollEventSFML(sf::Event& e){
    return m_SFMLWindow->pollEvent(e);
}
void Engine_Window::display(){
    m_SFMLWindow->display();
}
void Engine_Window::setSize(uint w, uint h){
	m_Width = w;
	m_Height = h;
	const sf::Vector2u size = sf::Vector2u(w,h);
	m_SFMLWindow->setSize(size);
}
void Engine_Window::setStyle(uint style){
	if(m_Style == style) return;
	m_Style = style;
	m_SFMLWindow->create(m_VideoMode,m_WindowName,m_Style,m_SFMLWindow->getSettings());
}
void Engine_Window::setFullScreen(bool fullscreen){
	if(m_Style == sf::Style::Fullscreen && fullscreen == true) return;
	if(m_Style != sf::Style::Fullscreen && fullscreen == false) return;

    if(Resources::Detail::ResourceManagement::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
        m_VideoMode = sf::VideoMode::getDesktopMode();
        m_Style = sf::Style::Fullscreen;
        if(!fullscreen){
            m_Style = sf::Style::Default;
            m_VideoMode.width = m_Width;
            m_VideoMode.height = m_Height;
        }
        delete(Renderer::Detail::RenderManagement::m_gBuffer);
        m_SFMLWindow->create(m_VideoMode,Resources::Detail::ResourceManagement::m_Window->name(),m_Style);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        Renderer::Detail::RenderManagement::m_gBuffer = new GBuffer(Resources::getWindowSize().x,Resources::getWindowSize().y);
        Detail::EngineClass::EVENT_RESIZE(Resources::getWindowSize().x,Resources::getWindowSize().y,false);
    }
    else if(Resources::Detail::ResourceManagement::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){
    }
}
