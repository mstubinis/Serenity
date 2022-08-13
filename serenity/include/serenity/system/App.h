#pragma once
#ifndef ENGINE_SYSTEM_APP_H
#define ENGINE_SYSTEM_APP_H

struct EngineOptions;

#include <serenity/system/EngineOptions.h>
#include <serenity/types/ViewPointer.h>
#include <serenity/system/Engine.h>

namespace Engine {
    template<class GAMECORE>
	class App {
        private:
            App() = delete;
		public:
            App(const EngineOptions& engineOptions) {
                auto renderingContexts = std::make_unique<Engine::priv::EngineRenderingContexts>( engineOptions );
                Engine::priv::Core::m_RenderingContexts = renderingContexts;

                auto windows = std::make_unique<Engine::priv::EngineWindows>(engineOptions);
                Engine::priv::Core::m_Windows = windows;
                assert(!windows->empty());
                auto& mainWindow = windows->at(0);
                mainWindow.setRenderContext(renderingContexts->at(0));
                mainWindow.setVerticalSyncEnabled(engineOptions.vsync);

                auto engine = std::make_unique<Engine::priv::EngineCore>( engineOptions, mainWindow);
                Engine::priv::Core::m_Engine = engine;

                auto gameCore = std::make_unique<GAMECORE>();
                engine->run(engineOptions, gameCore);
            }
            App(const App&)                = delete;
            App& operator=(const App&)     = delete;
            App(App&&) noexcept            = delete;
            App& operator=(App&&) noexcept = delete;
	};
}

#endif
