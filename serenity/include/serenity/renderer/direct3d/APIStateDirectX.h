#pragma once
#ifndef ENGINE_RENDERER_API_STATE_DIRECTX_H
#define ENGINE_RENDERER_API_STATE_DIRECTX_H

class Window;
class Texture;
class TextureCubemap;

#include <serenity/system/window/Window.h>
#include <serenity/renderer/APIManager.h>

namespace Engine::priv {
	template<>
	class APIState<Engine::priv::DirectX> {
		private:

		public:
            APIState<DirectX>(Window& window) {

            }
	};
}

#endif