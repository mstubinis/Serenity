#include <serenity/renderer/APIManager.h>
#include <serenity/renderer/opengl/APIStateOpenGL.h>
#include <serenity/renderer/direct3d/APIStateDirectX.h>
#include <serenity/types/ViewPointer.h>

#include <serenity/system/Engine.h>
#include <serenity/system/EngineOptions.h>


namespace Engine::priv {


	APIManager::APIManager(const EngineOptions& engineOptions, Window& window) {
		switch (engineOptions.renderingAPI) {
			case RenderingAPI::OpenGL: {
				m_APIStateOpenGL = NEW APIState<Engine::priv::OpenGL>(window);
				break;
			}
			case RenderingAPI::DirectX: {
				m_APIStateDirectX = NEW APIState<Engine::priv::DirectX>(window);
				break;
			}
			case RenderingAPI::Vulkan: {
				break;
			}
			case RenderingAPI::Metal: {
				break;
			}
		}
		m_APIType        = engineOptions.renderingAPI;

		Engine::priv::Core::m_APIManager = this;
	}
	APIManager::~APIManager() {
		SAFE_DELETE(m_APIStateOpenGL);
		SAFE_DELETE(m_APIStateDirectX);
	}
}