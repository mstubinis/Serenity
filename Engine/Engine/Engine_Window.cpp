#include "Engine.h"
#include "Engine_Window.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Texture.h"
#include "GBuffer.h"

#include <SFML/Window.hpp>

using namespace Engine;


class Engine_Window::impl{
    public:
        uint m_Style;
        sf::VideoMode m_VideoMode;
        const char* m_WindowName;
        sf::Window* m_SFMLWindow;
        uint m_Width; uint m_Height;
        void _init(const char* name,uint width,uint height){
            m_WindowName = name;
            m_Width = width;
            m_Height = height;
            m_SFMLWindow = new sf::Window();
            _createOpenGLWindow(name,width,height);
        }
        void _destruct(){
            SAFE_DELETE(m_SFMLWindow);
        }
        void _createOpenGLWindow(const char* name,uint width,uint height){
            sf::ContextSettings settings;
            settings.depthBits = 24;
            settings.stencilBits = 8;
            settings.antialiasingLevel = 0;
            settings.majorVersion = 3;
            settings.minorVersion = 3;

            #ifdef _DEBUG
                settings.attributeFlags = settings.Debug;
            #else
                settings.attributeFlags = settings.Default;
            #endif

            m_VideoMode.width = width;
            m_VideoMode.height = height;
            m_Width = width;
            m_Height = height;
            m_VideoMode.bitsPerPixel = 32;

            m_Style = sf::Style::Default;
            if(m_Width == 0 || m_Height == 0){
                m_Style = sf::Style::Fullscreen;
                m_VideoMode = sf::VideoMode::getDesktopMode();
                m_Width = m_VideoMode.width;
                m_Height = m_VideoMode.height;
            }
            m_SFMLWindow->create(m_VideoMode,name,m_Style,settings);

            //move these 2 functions elsewhere like renderer::init() or engine::init() ?
            glewExperimental = GL_TRUE; 
            glewInit();
            
            glEnable(GL_TEXTURE_2D);//is this really needed?

            Renderer::Settings::enableCullFace();
            Renderer::Settings::cullFace(GL_BACK);

            SAFE_DELETE(Renderer::Detail::RenderManagement::m_gBuffer);
            Renderer::Detail::RenderManagement::m_gBuffer = new GBuffer(m_Width,m_Height);
        }
        void _setFullScreen(bool fullscreen){
            if(m_Style == sf::Style::Fullscreen && fullscreen) return;
            if(m_Style != sf::Style::Fullscreen && !fullscreen) return;
            if(!m_SFMLWindow->hasFocus()) return;

            m_VideoMode = sf::VideoMode::getDesktopMode();
            m_Style = sf::Style::Fullscreen;
            if(!fullscreen){
                m_Style = sf::Style::Default;
                m_VideoMode.width = m_Width;
                m_VideoMode.height = m_Height;
            }
            SAFE_DELETE(Renderer::Detail::RenderManagement::m_gBuffer);
            m_SFMLWindow->create(m_VideoMode,m_WindowName,m_Style,m_SFMLWindow->getSettings());

            glEnable(GL_TEXTURE_2D);//is this really needed?
            Renderer::Settings::enableCullFace();
            Renderer::Settings::cullFace(GL_BACK);

            Renderer::Detail::RenderManagement::m_gBuffer = new GBuffer(Resources::getWindowSize().x,Resources::getWindowSize().y);
            Detail::EngineClass::EVENT_RESIZE(Resources::getWindowSize().x,Resources::getWindowSize().y,false);
        }
        void _setStyle(uint style){
            if(m_Style == style) return;
            m_Style = style;
            m_SFMLWindow->close();
            _createOpenGLWindow(m_WindowName,m_Width,m_Height);
        }
        void _setSize(uint w, uint h){
            m_Width = w; m_Height = h;
            m_SFMLWindow->setSize(sf::Vector2u(w,h));
        }
        void _setName(const char* name){
            if(m_WindowName == name) return;
            m_WindowName = name;
            m_SFMLWindow->setTitle(m_WindowName);
        }
        void _setIcon(const char* file){
            if(!Resources::Detail::ResourceManagement::m_Textures.count(file)){
                new Texture(file,"",GL_TEXTURE_2D,false,ImageInternalFormat::RGBA8);
            }
            Texture* texture = Resources::getTexture(file);
            m_SFMLWindow->setIcon(texture->width(),texture->height(),texture->pixels());
        }
        void _setIcon(Texture* texture){
            m_SFMLWindow->setIcon(texture->width(),texture->height(),texture->pixels());
        }
};

Engine_Window::Engine_Window(const char* name,uint width,uint height):m_i(new impl){
    m_i->_init(name,width,height);
}
Engine_Window::~Engine_Window(){
    m_i->_destruct();
}
sf::Vector2u Engine_Window::getSize(){return m_i->m_SFMLWindow->getSize();}
void Engine_Window::setIcon(Texture* texture){m_i->_setIcon(texture);}
void Engine_Window::setIcon(const char* file){m_i->_setIcon(file);}
const char* Engine_Window::name() const {return m_i->m_WindowName;}
void Engine_Window::setName(const char* name){m_i->_setName(name);}
void Engine_Window::setVerticalSyncEnabled(bool enabled){m_i->m_SFMLWindow->setVerticalSyncEnabled(enabled);}
void Engine_Window::setKeyRepeatEnabled(bool enabled){m_i->m_SFMLWindow->setKeyRepeatEnabled(enabled);}
void Engine_Window::setMouseCursorVisible(bool visible){m_i->m_SFMLWindow->setMouseCursorVisible(visible);}
void Engine_Window::requestFocus(){m_i->m_SFMLWindow->requestFocus();}
void Engine_Window::close(){m_i->m_SFMLWindow->close();}
bool Engine_Window::hasFocus(){return m_i->m_SFMLWindow->hasFocus();}
bool Engine_Window::isOpen(){return m_i->m_SFMLWindow->isOpen();}
void Engine_Window::display(){m_i->m_SFMLWindow->display();}
void Engine_Window::setActive(bool active){m_i->m_SFMLWindow->setActive(active);}
void Engine_Window::setSize(uint w, uint h){m_i->_setSize(w,h);}
void Engine_Window::setStyle(uint style){m_i->_setStyle(style);}
void Engine_Window::setFullScreen(bool fullscreen){ m_i->_setFullScreen(fullscreen); }
void Engine_Window::keepMouseInWindow(bool keep){ m_i->m_SFMLWindow->setMouseCursorGrabbed(keep); }
sf::Window* Engine_Window::getSFMLHandle() const { return m_i->m_SFMLWindow; }
uint Engine_Window::getStyle(){ return m_i->m_Style; }
