#pragma once
#ifndef ENGINE_SYSTEM_APP_H
#define ENGINE_SYSTEM_APP_H

struct EngineOptions;

#include <serenity/types/ViewPointer.h>
#include <serenity/system/Engine.h>

namespace Engine {
    template<class GAMECORE>
	class App {
        private:
            App() = delete;
		public:
            App(const EngineOptions& engineOptions) {
                Engine::priv::EngineWindows windows{ engineOptions };
                Engine::priv::Core::m_Windows = std::addressof(windows);
                Engine::priv::EngineCore engine{ engineOptions };
                Engine::priv::Core::m_Engine  = std::addressof(engine);
                GAMECORE gameCore;
                engine.run(engineOptions, std::addressof(gameCore));
            }
            App(const App&)                = delete;
            App& operator=(const App&)     = delete;
            App(App&&) noexcept            = delete;
            App& operator=(App&&) noexcept = delete;
	};
}

#endif
