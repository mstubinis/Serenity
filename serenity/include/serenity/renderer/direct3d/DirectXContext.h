#pragma once
#ifndef ENGINE_RENDERER_DIRECTX_CONTEXT_H
#define ENGINE_RENDERER_DIRECTX_CONTEXT_H

class Window;

#include <memory>
#include <serenity/renderer/APIContext.h>

#ifdef _WIN32

#endif

namespace Engine::priv {
	class DirectXContext : public IContext {
		private:
			struct impl;
			std::unique_ptr<impl> m_i;
		public:
			DirectXContext();

			bool init(Window&) override;
			void destroy(Window&) override;
			bool detatch(Window&) override;

			void setFramerateLimit(int limit) override;
			bool setVerticalSyncEnabled(bool enabled) override;
			bool display() override;
	};
}

#endif