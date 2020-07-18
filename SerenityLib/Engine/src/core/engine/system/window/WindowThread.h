#pragma once
#ifndef ENGINE_WINDOW_THREAD_H
#define ENGINE_WINDOW_THREAD_H

class Window;

#include <core/engine/system/window/WindowIncludes.h>
#include <core/engine/containers/Queue_ts.h>
#include <SFML/Window/Event.hpp>

namespace Engine::priv {
    class WindowData;
    class WindowThread final {
        friend class WindowData;
        friend class Window;
        private:
            WindowData& m_Data;
            Engine::queue_ts<sf::Event>                                m_Queue;
            Engine::queue_ts<WindowEventThreadOnlyCommands::Command>   m_MainThreadToEventThreadQueue;
            std::unique_ptr<std::thread>                               m_EventThread = nullptr;

            void cleanup();
            void startup(Window& super, const std::string& name);
            void push(WindowEventThreadOnlyCommands::Command command);
            std::optional<sf::Event> try_pop();
            void updateLoop();
        public:
            WindowThread(WindowData&);
            ~WindowThread();

            bool operator==(const bool rhs) const;
            explicit operator bool() const;
    };
}
#endif