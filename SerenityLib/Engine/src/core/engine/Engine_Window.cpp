#include <core/engine/Engine.h>
#include <core/engine/Engine_Window.h>
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

class Engine_Window::impl final{
    public:
        uint m_Style;
        sf::VideoMode m_VideoMode;
        const char* m_WindowName;
        sf::Window m_SFMLWindow;
        uint m_Width, m_Height, m_FramerateLimit;
        bool m_MouseCursorVisible;
        bool m_Fullscreen;
        bool m_Active;
        bool m_Vsync;
        bool m_MouseCursorGrabbed;
		int majorVersion;
		int minorVersion;
		int glslVersion;
        sf::ContextSettings m_SFContextSettings;
        void _init(const char* name, const uint& width, const uint& height){
            m_FramerateLimit = 0;
            m_MouseCursorVisible = true;
            m_Fullscreen = m_Vsync = m_MouseCursorGrabbed = false;
            m_WindowName = name;
            m_Width = width;
            m_Height = height;
			majorVersion = 4;
			minorVersion = 6;
			glslVersion = 330;
            m_SFContextSettings = _createOpenGLWindow(name,width,height, majorVersion, minorVersion, glslVersion);

            std::cout << "Using OpenGL: " << m_SFContextSettings.majorVersion << "." << m_SFContextSettings.minorVersion << ", with depth bits: " << m_SFContextSettings.depthBits << " and stencil bits: " << m_SFContextSettings.stencilBits << std::endl;
        }
        void _destruct(){
            //m_SFMLWindow.close();
        }
        void _restoreStateMachine(){
            if(m_FramerateLimit > 0)
                m_SFMLWindow.setFramerateLimit(m_FramerateLimit);
            m_SFMLWindow.setMouseCursorVisible(m_MouseCursorVisible);
            m_SFMLWindow.setActive(m_Active);
            m_SFMLWindow.setVerticalSyncEnabled(m_Vsync);
            m_SFMLWindow.setMouseCursorGrabbed(m_MouseCursorGrabbed);
        }
        const sf::ContextSettings _createOpenGLWindow(const char* name, const uint& width, const uint& height, const uint& _majorVersion, const uint& _minorVersion, const uint& _glslVersion){
            sf::ContextSettings settings;
            settings.depthBits         = 24;
            settings.stencilBits       = 0;
            settings.antialiasingLevel = 0;
            settings.majorVersion      = _majorVersion;
            settings.minorVersion      = _minorVersion;

            #ifdef _DEBUG
                settings.attributeFlags = settings.Debug;
            #else
                settings.attributeFlags = settings.Core;
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
            m_SFMLWindow.create(m_VideoMode,name,m_Style,settings);
            uint opengl_version = stoi(to_string(settings.majorVersion) + to_string(settings.minorVersion));
            epriv::Core::m_Engine->m_RenderManager._onOpenGLContextCreation(m_Width,m_Height,_glslVersion,opengl_version);

            return m_SFMLWindow.getSettings();
        }
        void _setFullScreen(const bool& fullscreen){
            if(m_Fullscreen == fullscreen) return;
            if(m_Style == sf::Style::Fullscreen && fullscreen) return;
            if(m_Style != sf::Style::Fullscreen && !fullscreen) return;
            if(!m_SFMLWindow.hasFocus()) return;

            auto validModes = sf::VideoMode::getFullscreenModes();
            if (validModes.size() > 0) m_VideoMode = validModes[0];
            else                       m_VideoMode = sf::VideoMode::getDesktopMode();

            m_Style = sf::Style::Fullscreen;
            if(!fullscreen){
                m_Style = sf::Style::Default;
                m_VideoMode.width = m_Width;
                m_VideoMode.height = m_Height;
            }
            epriv::Core::m_Engine->m_RenderManager._onFullscreen(&m_SFMLWindow,m_VideoMode,m_WindowName,m_Style,m_SFContextSettings);

            //event dispatch
            epriv::EventWindowFullscreenChanged e;
            e.isFullscreen = fullscreen;
            Event ev;
            ev.eventWindowFullscreenChanged = e;
            ev.type = EventType::WindowFullscreenChanged;
            epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);

            glm::uvec2 winSize = Engine::getWindowSize();

            //basically this block of code is a copy of EVENT_RESIZE, i wish this would trigger the event resize method...
            epriv::Core::m_Engine->m_RenderManager._resize(winSize.x,winSize.y);
            //resize cameras here
            Game::onResize(winSize.x,winSize.y);
            for (auto scene : epriv::Core::m_Engine->m_ResourceManager.scenes()) {
                epriv::InternalScenePublicInterface::GetECS(*scene).onResize<ComponentCamera>(winSize.x, winSize.y);
                epriv::InternalScenePublicInterface::GetViewports(*scene)[0]->setViewportDimensions(0, 0, winSize.x, winSize.y);
            }
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////

            //for some reason the mouse is shown even if it was hidden at first
            m_SFMLWindow.setMouseCursorVisible(m_MouseCursorVisible);
            m_SFContextSettings = m_SFMLWindow.getSettings();
            m_Fullscreen = fullscreen;
            _restoreStateMachine();
        }
        void _setStyle(const uint& style){
            if(m_Style == style) return;
            m_Style = style;
            m_SFMLWindow.close();
            _createOpenGLWindow(m_WindowName,m_Width,m_Height, majorVersion, minorVersion, glslVersion);
        }
        void _setSize(const uint& w, const uint& h){
            m_Width = w; m_Height = h;
            m_SFMLWindow.setSize(sf::Vector2u(w,h));
        }
        void _setName(const char* name){
            if(m_WindowName == name) return;
            m_WindowName = name;
            m_SFMLWindow.setTitle(m_WindowName);
        }
        void _setIcon(const char* file){
            Texture* texture = epriv::Core::m_Engine->m_ResourceManager._hasTexture(file);
            if(!texture){
                texture = new Texture(file,false,ImageInternalFormat::RGBA8);
                Handle handle = epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
            }
            m_SFMLWindow.setIcon(texture->width(),texture->height(),texture->pixels());
        }
        void _setIcon(Texture& texture){
            m_SFMLWindow.setIcon(texture.width(),texture.height(),texture.pixels());
        }
        void _setMouseCursorVisible(const bool& visible){
            if(m_MouseCursorVisible != visible){
                m_SFMLWindow.setMouseCursorVisible(visible);
                m_MouseCursorVisible = visible;
            }
        }
        void _setFramerateLimit(const uint& limit){
            m_SFMLWindow.setFramerateLimit(limit);
            m_FramerateLimit = limit;
        }
        void _keepMouseInWindow(const bool& keep){
            if(m_MouseCursorGrabbed != keep){
                m_SFMLWindow.setMouseCursorGrabbed(keep);
                m_MouseCursorGrabbed = keep;
            }
        }
};

Engine_Window::Engine_Window(const char* name, const uint& width, const uint& height):m_i(new impl){
    m_i->_init(name,width,height);
}
Engine_Window::~Engine_Window(){
    m_i->_destruct();
}
glm::uvec2 Engine_Window::getSize(){ 
    sf::Vector2u size = m_i->m_SFMLWindow.getSize(); 
    return glm::uvec2(size.x,size.y); 
}
void Engine_Window::setIcon(const Texture& texture){
    m_i->_setIcon(const_cast<Texture&>(texture));
}
void Engine_Window::setIcon(const char* file){
    m_i->_setIcon(file);
}
void Engine_Window::setIcon(const string& file) { 
    m_i->_setIcon(file.c_str()); 
}
const char* Engine_Window::name() const {
    return m_i->m_WindowName;
}
void Engine_Window::setName(const char* name){
    m_i->_setName(name);
}
void Engine_Window::setVerticalSyncEnabled(const bool enabled){
    if (m_i->m_Vsync != enabled) {
        m_i->m_SFMLWindow.setVerticalSyncEnabled(enabled);
        m_i->m_Vsync = enabled;
    }
}
void Engine_Window::setKeyRepeatEnabled(const bool enabled){
    m_i->m_SFMLWindow.setKeyRepeatEnabled(enabled);
}
void Engine_Window::setMouseCursorVisible(const bool visible){
    m_i->_setMouseCursorVisible(visible); 
}
void Engine_Window::requestFocus(){
    m_i->m_SFMLWindow.requestFocus();
}
void Engine_Window::close(){
    m_i->m_SFMLWindow.close();
}
bool Engine_Window::hasFocus(){
    return m_i->m_SFMLWindow.hasFocus();
}
bool Engine_Window::isOpen(){
    return m_i->m_SFMLWindow.isOpen();
}
bool Engine_Window::isActive(){ 
    return m_i->m_Active; 
}
bool Engine_Window::isFullscreen(){
    return m_i->m_Fullscreen;
}
void Engine_Window::display(){
    m_i->m_SFMLWindow.display();
}
void Engine_Window::setActive(const bool active){
    m_i->m_Active = active;
    m_i->m_SFMLWindow.setActive(active);
}
void Engine_Window::setSize(const uint& w, const uint& h){
    m_i->_setSize(w,h);
}
void Engine_Window::setStyle(const uint& style){
    m_i->_setStyle(style);
}
void Engine_Window::setFullScreen(const bool fullscreen){
    m_i->_setFullScreen(fullscreen); 
}
void Engine_Window::keepMouseInWindow(const bool keep){
    m_i->_keepMouseInWindow(keep); 
}
void Engine_Window::setFramerateLimit(const uint& limit){
    m_i->_setFramerateLimit(limit); 
}
sf::Window& Engine_Window::getSFMLHandle() const { 
    return m_i->m_SFMLWindow; 
}
uint Engine_Window::getStyle(){ 
    return m_i->m_Style; 
}
uint Engine_Window::getFramerateLimit() const{ 
    return m_i->m_FramerateLimit; 
}