#include <serenity/renderer/direct3d/DirectXContext.h>
#include <serenity/system/window/Window.h>

#ifdef _WIN32
#include <DirectXTK/DirectXHelpers.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DirectXTK.lib")
#endif

struct Engine::priv::DirectXContext::impl {
	ID3D11Device*            m_Device           = nullptr;
	ID3D11DeviceContext*     m_DeviceContext    = nullptr;
	IDXGISwapChain*          m_SwapChain        = nullptr;
	ID3D11RenderTargetView*  m_RenderTargetView = nullptr;
	Engine::view_ptr<Window> m_Window           = nullptr;

	bool on_window_resize() {
		if (m_SwapChain) {
			m_DeviceContext->OMSetRenderTargets(0, 0, 0);

			// Release all outstanding references to the swap chain's buffers.
			m_RenderTargetView->Release();

			HRESULT hr;
			// Preserve the existing buffer count and format.
			// Automatically choose the width and height to match the client rect for HWNDs.
			hr = m_SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
			if (FAILED(hr)) {
				return false;
			}

			// Get buffer and create a render-target-view.
			ID3D11Texture2D* pBuffer;
			hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);
			if (FAILED(hr)) {
				return false;
			}

			hr = m_Device->CreateRenderTargetView(pBuffer, NULL, &m_RenderTargetView);
			if (FAILED(hr)) {
				return false;
			}
			pBuffer->Release();

			m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, NULL);

			// Set up the viewport.
			D3D11_VIEWPORT vp;
			vp.Width    = m_Window->getSize().x;
			vp.Height   = m_Window->getSize().y;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			m_DeviceContext->RSSetViewports(1, &vp);
		}
		return true;
	}
};

namespace Engine::priv {
	DirectXContext::DirectXContext()
		: m_i{ std::make_unique<impl>() }
	{

	}

	bool DirectXContext::init(Window& window) {
		m_i->m_Window = &window;
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount                        = 1;
		sd.BufferDesc.Width                   = window.getSize().x;
		sd.BufferDesc.Height                  = window.getSize().y;
		sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator   = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow                       = window.getSystemHandle();
		sd.SampleDesc.Count                   = 1;
		sd.SampleDesc.Quality                 = 0;
		sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.Windowed                           = TRUE;


		D3D_FEATURE_LEVEL  FeatureLevelsSupported;
		const D3D_FEATURE_LEVEL lvl[] = { 
			D3D_FEATURE_LEVEL_11_1, 
			D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1, 
			D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3, 
			D3D_FEATURE_LEVEL_9_2, 
			D3D_FEATURE_LEVEL_9_1 
		};
		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		ID3D11Device* device = nullptr;
		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr, 
			D3D_DRIVER_TYPE_HARDWARE, 
			nullptr, 
			createDeviceFlags, 
			lvl, 
			_countof(lvl), 
			D3D11_SDK_VERSION, 
			&sd,
			&m_i->m_SwapChain, 
			&m_i->m_Device,
			&FeatureLevelsSupported, 
			&m_i->m_DeviceContext
		);
		if (hr == E_INVALIDARG) {
			hr = D3D11CreateDeviceAndSwapChain(
				nullptr, 
				D3D_DRIVER_TYPE_HARDWARE, 
				nullptr, 
				createDeviceFlags, 
				&lvl[1], 
				_countof(lvl) - 1, 
				D3D11_SDK_VERSION, 
				&sd, 
				&m_i->m_SwapChain,
				&m_i->m_Device,
				&FeatureLevelsSupported,
				&m_i->m_DeviceContext
			);
		}
		if (FAILED(hr)) {
			return false;
		}

		ID3D11Texture2D* pBackBuffer;
		// Get a pointer to the back buffer
		hr = m_i->m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);


		// Create a render-target view
		m_i->m_Device->CreateRenderTargetView(pBackBuffer, NULL, &m_i->m_RenderTargetView);


		// Bind the view
		m_i->m_DeviceContext->OMSetRenderTargets(1, &m_i->m_RenderTargetView, NULL);


		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width    = window.getSize().x;
		vp.Height   = window.getSize().y;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_i->m_DeviceContext->RSSetViewports(1, &vp);

		return true;
	}
	void DirectXContext::destroy(Window& window) {
	}
	bool DirectXContext::detatch(Window& window) {
		return true;
	}

	void DirectXContext::setFramerateLimit(int limit) {

	}
	bool DirectXContext::setVerticalSyncEnabled(bool enabled) {
		return true;
	}
	bool DirectXContext::display() {
		//TODO: this is just to test - get rid of it eventually
		const float bgcolor[] = {0.0f, 0.0f, 1.0f, 1.0f};
		m_i->m_DeviceContext->ClearRenderTargetView(m_i->m_RenderTargetView, bgcolor);

		HRESULT hr = m_i->m_SwapChain->Present(m_i->m_Window->isVsyncEnabled() ? 1 : 0, 0);
		return (!(FAILED(hr)));
		//return true;
	}
}