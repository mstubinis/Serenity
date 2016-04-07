#include "Engine_Window.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "GBuffer.h"
#include <SFML/Window.hpp>
#include <string>


#ifdef _WIN32
void Engine_Window::_createDirectXWindow(const char* name,uint width,uint height){

}
void Engine_Window::setRenderingAPI(uint api){
    //do alot of crap here...
    if(api == ENGINE_RENDERING_API_OPENGL){
    }
    else if(api == ENGINE_RENDERING_API_DIRECTX){
    }
    Engine::Detail::EngineClass::m_RenderingAPI = static_cast<ENGINE_RENDERING_API>(api);
}
#endif
void Engine_Window::_destroyOpenGLContext(){
    HGLRC    hglrc; 
    HDC      hdc ; 
    if(hglrc = wglGetCurrentContext()) { 
        hdc = wglGetCurrentDC(); 
        wglMakeCurrent(NULL, NULL); 
		ReleaseDC (m_SFMLWindow->getSystemHandle(), hdc) ; 
        wglDeleteContext(hglrc); 
    }
}
Engine_Window::Engine_Window(const char* name,uint width,uint height,ENGINE_RENDERING_API api){
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

    sf::VideoMode mode;
    mode.width = m_Width;
    mode.height = m_Height;
    mode.bitsPerPixel = 32;

    uint style = sf::Style::Default;
    if(m_Width == 0 || m_Height == 0){
        style = sf::Style::Fullscreen;
        mode = sf::VideoMode::getDesktopMode();
        m_Width = mode.width;
        m_Height = mode.height;
    }
    m_SFMLWindow->create(mode,name,style,settings);
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
void Engine_Window::setFullScreen(bool fullscreen){
    if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
        sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();
        unsigned int style = sf::Style::Fullscreen;
        if(!fullscreen){
            style = sf::Style::Default;
            videoMode.width = m_Width;
            videoMode.height = m_Height;
        }
        delete(Engine::Renderer::Detail::RenderManagement::m_gBuffer);
        m_SFMLWindow->create(videoMode,Engine::Resources::Detail::ResourceManagement::m_Window->name(),style);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        Engine::Renderer::Detail::RenderManagement::m_gBuffer = new GBuffer(Engine::Resources::getWindowSize().x,Engine::Resources::getWindowSize().y);
        Engine::Detail::EngineClass::EVENT_RESIZE(Engine::Resources::getWindowSize().x,Engine::Resources::getWindowSize().y);
    }
    else if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){
    }
}
