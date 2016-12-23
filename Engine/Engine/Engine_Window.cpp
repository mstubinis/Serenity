#include "Engine.h"
#include "Engine_Window.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Texture.h"
#include "GBuffer.h"

#include <SFML/Window.hpp>

using namespace Engine;

#ifdef _WIN32
#include <d3d11.h>
#include <d3d10.h>
#include <windows.h>
#include <windowsx.h>
#pragma comment (lib, "d3d11.lib") // include the Direct3D Library files
#pragma comment (lib, "d3d10.lib") // include the Direct3D Library files
#endif

class Engine_Window::impl{
	public:
        uint m_Style;
        sf::VideoMode m_VideoMode;
        const char* m_WindowName;
        sf::Window* m_SFMLWindow;
        uint m_Width; uint m_Height;

		#ifdef _WIN32
		void _setRenderingAPI(uint newAPI){
			if(newAPI == Resources::Detail::ResourceManagement::m_RenderingAPI) return;

			if(newAPI == ENGINE_RENDERING_API_OPENGL){
				_destroyDirectXContext();
				_createOpenGLWindow(m_WindowName,m_Width,m_Height);
			}
			else if(newAPI == ENGINE_RENDERING_API_DIRECTX){
				_destroyOpenGLContext();
				_createDirectXWindow(m_WindowName,m_Width,m_Height);
			}
			Resources::Detail::ResourceManagement::m_RenderingAPI = static_cast<ENGINE_RENDERING_API>(newAPI);
		}
		void _createDirectXWindow(const char* name,uint width,uint height,unsigned int windowed = 1){
			SAFE_DELETE_COM(Renderer::Detail::RenderManagement::m_DirectXSwapChain);
			SAFE_DELETE_COM(Renderer::Detail::RenderManagement::m_DirectXDevice);
			SAFE_DELETE_COM(Renderer::Detail::RenderManagement::m_DirectXDeviceContext);
			SAFE_DELETE_COM(Renderer::Detail::RenderManagement::m_DirectXBackBuffer);

			// create a struct to hold information about the swap chain
			DXGI_SWAP_CHAIN_DESC scd;

			// clear out the struct for use
			ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

			// fill the swap chain description struct
			scd.BufferCount = 1;                                    // one back buffer
			scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
			scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
			scd.BufferDesc.Width = width;                           // set the back buffer width
			scd.BufferDesc.Height = height;                         // set the back buffer height
			scd.OutputWindow = m_SFMLWindow->getSystemHandle();     // the window to be used
			scd.SampleDesc.Count = 4;                               // how many multisamples
			scd.Windowed = static_cast<BOOL>(windowed);             // windowed/full-screen mode

			// create a device, device context and swap chain using the information in the scd struct
			D3D11CreateDeviceAndSwapChain(0,D3D_DRIVER_TYPE_HARDWARE,0,0,0,0,D3D11_SDK_VERSION,&scd,
				&Renderer::Detail::RenderManagement::m_DirectXSwapChain,
				&Renderer::Detail::RenderManagement::m_DirectXDevice,0,
				&Renderer::Detail::RenderManagement::m_DirectXDeviceContext);

			// get the address of the back buffer
			ID3D11Texture2D *pBackBuffer;
			Renderer::Detail::RenderManagement::m_DirectXSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

			// use the back buffer address to create the render target
			Renderer::Detail::RenderManagement::m_DirectXDevice->CreateRenderTargetView(pBackBuffer, NULL, &Renderer::Detail::RenderManagement::m_DirectXBackBuffer);
			SAFE_DELETE_COM(pBackBuffer);

			// set the render target as the back buffer
			Renderer::Detail::RenderManagement::m_DirectXDeviceContext->OMSetRenderTargets(1, &Renderer::Detail::RenderManagement::m_DirectXBackBuffer, NULL);

			// Set the viewport
			D3D11_VIEWPORT viewport;
			ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = static_cast<FLOAT>(width);
			viewport.Height = static_cast<FLOAT>(height);

			Renderer::Detail::RenderManagement::m_DirectXDeviceContext->RSSetViewports(1, &viewport);
			//Resources::initRenderingContexts(ENGINE_RENDERING_API_DIRECTX);
		}
		void _destroyDirectXContext(){
			//Resources::cleanupRenderingContexts(ENGINE_RENDERING_API_DIRECTX);
			//Renderer::Detail::RenderManagement::m_DirectXSwapChain->SetFullscreenState(FALSE, NULL);//this might not even be needed
			Renderer::Detail::RenderManagement::m_DirectXDeviceContext->Flush();//this might not even be needed
			Renderer::Detail::RenderManagement::m_DirectXDeviceContext->ClearState();//this might not even be needed
			SAFE_DELETE_COM(Renderer::Detail::RenderManagement::m_DirectXSwapChain);
			SAFE_DELETE_COM(Renderer::Detail::RenderManagement::m_DirectXBackBuffer);
			SAFE_DELETE_COM(Renderer::Detail::RenderManagement::m_DirectXDevice);
			SAFE_DELETE_COM(Renderer::Detail::RenderManagement::m_DirectXDeviceContext);
		}
		#endif

		void _init(const char* name,uint width,uint height,uint api){
			m_WindowName = name;
			m_Width = width;
			m_Height = height;
			m_SFMLWindow = new sf::Window();
			_createOpenGLWindow(name,width,height);
			if(api == ENGINE_RENDERING_API_DIRECTX){
				_destroyOpenGLContext();
				_createDirectXWindow(name,width,height);
			}
		}
		void _destruct(){
			SAFE_DELETE(m_SFMLWindow);
		}
		void _createOpenGLWindow(const char* name,uint width,uint height){
			sf::ContextSettings settings;
			settings.depthBits = 24;
			settings.stencilBits = 8;
			settings.antialiasingLevel = 4;
			settings.majorVersion = 3;
			settings.minorVersion = 0;

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

			glewExperimental = GL_TRUE; 
			glewInit();
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			SAFE_DELETE(Engine::Renderer::Detail::RenderManagement::m_gBuffer);
			Renderer::Detail::RenderManagement::m_gBuffer = new GBuffer(m_Width,m_Height);

			//Resources::initRenderingContexts(ENGINE_RENDERING_API_OPENGL);
		}
		void _destroyOpenGLContext(){
			//Resources::cleanupRenderingContexts(ENGINE_RENDERING_API_OPENGL);
			HGLRC hglrc; HDC hdc ; 
			if(hglrc = wglGetCurrentContext()){ 
				hdc = wglGetCurrentDC(); 
				wglMakeCurrent(0,0); 
				ReleaseDC(m_SFMLWindow->getSystemHandle(), hdc); 
				wglDeleteContext(hglrc); 
			}
			SAFE_DELETE(Engine::Renderer::Detail::RenderManagement::m_gBuffer);
		}
		void _setFullScreen(bool fullscreen){
			if(m_Style == sf::Style::Fullscreen && fullscreen == true) return;
			if(m_Style != sf::Style::Fullscreen && fullscreen == false) return;
			if(!m_SFMLWindow->hasFocus()) return;

			if(Resources::Detail::ResourceManagement::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
				m_VideoMode = sf::VideoMode::getDesktopMode();
				m_Style = sf::Style::Fullscreen;
				if(!fullscreen){
					m_Style = sf::Style::Default;
					m_VideoMode.width = m_Width;
					m_VideoMode.height = m_Height;
				}
				SAFE_DELETE(Renderer::Detail::RenderManagement::m_gBuffer);
				m_SFMLWindow->create(m_VideoMode,m_WindowName,m_Style,m_SFMLWindow->getSettings());

				glEnable(GL_TEXTURE_2D);
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);

				Renderer::Detail::RenderManagement::m_gBuffer = new GBuffer(Resources::getWindowSize().x,Resources::getWindowSize().y);
				Detail::EngineClass::EVENT_RESIZE(
					Resources::Detail::ResourceManagement::m_RenderingAPI,
					Resources::getWindowSize().x,Resources::getWindowSize().y,false
				);
			}
			else if(Resources::Detail::ResourceManagement::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){

			}
		}
		void _setStyle(uint style){
			if(m_Style == style) return;
			m_Style = style;
			m_SFMLWindow->close();
			if(Resources::Detail::ResourceManagement::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
				_destroyDirectXContext();
				_createOpenGLWindow(m_WindowName,m_Width,m_Height);
			}
			else if(Resources::Detail::ResourceManagement::m_RenderingAPI == ENGINE_RENDERING_API_DIRECTX){
				_destroyOpenGLContext();
				_createDirectXWindow(m_WindowName,m_Width,m_Height);
			}
		}
		void _setSize(uint w, uint h){
			m_Width = w;
			m_Height = h;
			const sf::Vector2u size = sf::Vector2u(w,h);
			m_SFMLWindow->setSize(size);
		}
		void _setName(const char* name){
			if(m_WindowName == name) return;
			m_WindowName = name;
			m_SFMLWindow->setTitle(m_WindowName);
		}
		void _setIcon(const char* file){
			if(!Resources::Detail::ResourceManagement::m_Textures.count(skey(file))){
				new Texture(file);
			}
			Texture* texture = Resources::getTexture(file);
			m_SFMLWindow->setIcon(texture->width(),texture->height(),texture->pixels());
		}
		void _setIcon(Texture* texture){
			m_SFMLWindow->setIcon(texture->width(),texture->height(),texture->pixels());
		}
};
#ifdef _WIN32
void Engine_Window::setRenderingAPI(uint api){
	m_i->_setRenderingAPI(api);
}
#endif
Engine_Window::Engine_Window(const char* name,uint width,uint height,uint api):m_i(new impl()){
	m_i->_init(name,width,height,api);
}
Engine_Window::~Engine_Window(){
	m_i->_destruct();
}
sf::Vector2u Engine_Window::getSize(){
    return m_i->m_SFMLWindow->getSize();
}
void Engine_Window::setIcon(Texture* texture){
    m_i->_setIcon(texture);
}
void Engine_Window::setIcon(const char* file){
	m_i->_setIcon(file);
}
const char* Engine_Window::name() const {
    return m_i->m_WindowName;
}
void Engine_Window::setName(const char* name){
	m_i->_setName(name);
}
void Engine_Window::setVerticalSyncEnabled(bool enabled){
    m_i->m_SFMLWindow->setVerticalSyncEnabled(enabled);
}
void Engine_Window::setKeyRepeatEnabled(bool enabled){
    m_i->m_SFMLWindow->setKeyRepeatEnabled(enabled);
}
void Engine_Window::setMouseCursorVisible(bool visible){
    m_i->m_SFMLWindow->setMouseCursorVisible(visible);
}
void Engine_Window::requestFocus(){
    m_i->m_SFMLWindow->requestFocus();
}
void Engine_Window::close(){
    m_i->m_SFMLWindow->close();
}
bool Engine_Window::hasFocus(){
    return m_i->m_SFMLWindow->hasFocus();
}
bool Engine_Window::isOpen(){
    return m_i->m_SFMLWindow->isOpen();
}
void Engine_Window::display(){
    m_i->m_SFMLWindow->display();
}
void Engine_Window::setActive(bool active){ 
	m_i->m_SFMLWindow->setActive(active); 
}
void Engine_Window::setSize(uint w, uint h){
	m_i->_setSize(w,h);
}
void Engine_Window::setStyle(uint style){
	m_i->_setStyle(style);
}
void Engine_Window::setFullScreen(bool fullscreen){
	m_i->_setFullScreen(fullscreen);
}
void Engine_Window::keepMouseInWindow(bool keep){
	m_i->m_SFMLWindow->setMouseCursorGrabbed(keep);
}
sf::Window* Engine_Window::getSFMLHandle() const { return m_i->m_SFMLWindow; }
uint Engine_Window::getStyle(){ return m_i->m_Style; }