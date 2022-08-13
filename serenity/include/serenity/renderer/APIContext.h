#pragma once
#ifndef ENGINE_RENDERER_API_CONTEXT_H
#define ENGINE_RENDERER_API_CONTEXT_H

class Window;

namespace Engine::priv {
	class IContext {
		public:
			virtual ~IContext() = default;

			virtual bool init(Window&) = 0;
			virtual void destroy(Window&) = 0;
			virtual bool detatch(Window&) = 0;

			virtual void setFramerateLimit(int limit) = 0;
			virtual bool setVerticalSyncEnabled(bool enabled) = 0;
			virtual bool display() = 0;
	};
}

#endif