#include "Engine_Window.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "GBuffer.h"
#include <SFML/Window.hpp>
#include <string>


#ifdef _WIN32
void Engine_Window::_createDirectXWindow(const char* wCName,const char* name,HINSTANCE hInst,int nCmdShow,uint width,uint height,uint xPos, uint yPos){
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));// clear out the window class for use

    // fill in the struct with the needed information
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = Engine::Detail::EngineClass::WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = wCName;
    RegisterClassEx(&wc); // register the window class

    m_DirectXWindow = CreateWindowEx(0,wCName,name,WS_OVERLAPPEDWINDOW,xPos,yPos,width,height,0,0,hInst,0);
}
void Engine_Window::setRenderingAPI(uint api){
    //do alot of crap here...
    if(api == ENGINE_RENDERING_API_OPENGL){
        m_SFMLWindow->setActive(true);
        m_SFMLWindow->setVisible(true);
        // and hide directx window
        ShowWindow(m_DirectXWindow,SW_HIDE);
    }
    else if(api == ENGINE_RENDERING_API_DIRECTX){
        m_SFMLWindow->setActive(false);
        m_SFMLWindow->setVisible(false);
        // and show directx window
        ShowWindow(m_DirectXWindow,SW_SHOW);
    }
    Engine::Detail::EngineClass::m_RenderingAPI = static_cast<ENGINE_RENDERING_API>(api);
}
#endif

Engine_Window::Engine_Window(const char* name,uint width,uint height,ENGINE_RENDERING_API api){
    m_WindowName = name;
    m_Width = width; m_Height = height;
    m_SFMLWindow = new sf::Window();
    this->_createOpenGLWindow(name,width,height);
    #ifdef _WIN32
        this->_createDirectXWindow("WindowClass1",name,GetModuleHandle(0),1,width,height,0,0);
        if(api == ENGINE_RENDERING_API_DIRECTX){
            m_SFMLWindow->setActive(false);
            m_SFMLWindow->setVisible(false);
            ShowWindow(m_DirectXWindow, SW_SHOW);
        }
    #endif

    if(api == ENGINE_RENDERING_API_OPENGL){
        #ifdef _WIN32
            ShowWindow(m_DirectXWindow,SW_HIDE);
        #endif
        m_SFMLWindow->setActive(true);
        m_SFMLWindow->setVisible(true);
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
    if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
        return m_SFMLWindow->getSize();
    }
    else if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){
        #ifdef _WIN32
            RECT rect;
            if(GetWindowRect(m_DirectXWindow, &rect)){
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;
                return sf::Vector2u(width,height);
            }
        #endif
    }
    return sf::Vector2u(0,0);
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
    #ifdef _WIN32
        SetWindowText(m_DirectXWindow,m_WindowName);
    #endif
}
void Engine_Window::setVerticalSyncEnabled(bool enabled){
    m_SFMLWindow->setVerticalSyncEnabled(enabled);
}
void Engine_Window::setKeyRepeatEnabled(bool enabled){
    m_SFMLWindow->setKeyRepeatEnabled(enabled);
}
void Engine_Window::setMouseCursorVisible(bool visible){
    m_SFMLWindow->setMouseCursorVisible(visible);
    #ifdef _WIN32
        ShowCursor(visible);
    #endif
}
void Engine_Window::requestFocus(){
    if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
        m_SFMLWindow->requestFocus();
        EnableWindow(m_DirectXWindow,false);
    }
    else if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){
        #ifdef _WIN32
            EnableWindow(m_DirectXWindow,true);
            EnableWindow(m_SFMLWindow->getSystemHandle(),false);
        #endif
    }
}
void Engine_Window::close(){
    m_SFMLWindow->close();
    #ifdef _WIN32
        CloseWindow(m_DirectXWindow);
    #endif
}
bool Engine_Window::hasFocus(){
    if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
        return m_SFMLWindow->hasFocus();
    }
    else if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){
        //double check this function
        #ifdef _WIN32
            if(GetForegroundWindow() == m_DirectXWindow) return true; return false;
        #endif
    }
    return false;
}
bool Engine_Window::isOpen(){
    if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
        return m_SFMLWindow->isOpen();
    }
    else if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){
        //double check this function
        #ifdef _WIN32
            if(IsWindowVisible(m_DirectXWindow) == 1)return true; return false;
        #endif
    }
    return false;
}
bool Engine_Window::pollEventSFML(sf::Event& e){
    return m_SFMLWindow->pollEvent(e);
}
void Engine_Window::display(){
    if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
        m_SFMLWindow->display();
    }
    else if(Engine::Detail::EngineClass::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){
    }
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
