#pragma once
#ifndef ENGINE_RENDERER_API_MANAGER_H
#define ENGINE_RENDERER_API_MANAGER_H

class  Window;
struct EngineOptions;

class RenderingAPI {
	public: 
		enum Type {
			OpenGL = 0,
			DirectX,
			Vulkan,
			Metal,
			Unknown = -1,
		};
};

namespace Engine::priv {

	struct OpenGL {};
	struct DirectX {};
	struct Vulkan {};
	struct Metal {};

	template<class T> class APIState;
	template<> class APIState<OpenGL>;
	template<> class APIState<DirectX>;
	template<> class APIState<Vulkan>;
	template<> class APIState<Metal>;
}

namespace Engine::priv {
	class APIManager {
		private:
			APIState<OpenGL>*    m_APIStateOpenGL  = nullptr;
			APIState<DirectX>*   m_APIStateDirectX = nullptr;
			//APIState<Vulkan>*  m_APIStateVulkan  = nullptr;
			//APIState<Metal>*   m_APIStateMetal   = nullptr;
			RenderingAPI::Type   m_APIType         = RenderingAPI::Unknown;

		public:
			APIManager(const EngineOptions&, Window&);
			~APIManager();

			[[nodiscard]] APIState<OpenGL>& getOpenGL() noexcept { return *m_APIStateOpenGL; }
			[[nodiscard]] APIState<DirectX>& getDirectX() noexcept { return *m_APIStateDirectX; }

			[[nodiscard]] RenderingAPI::Type getAPI() noexcept { return m_APIType; }
	};
}

#endif