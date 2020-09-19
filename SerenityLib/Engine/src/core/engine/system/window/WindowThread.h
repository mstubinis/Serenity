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
            WindowData&                                       m_Data;
            Engine::queue_ts<sf::Event>                       m_SFEventQueue;
            Engine::queue_ts<WindowEventThreadOnlyCommands>   m_MainThreadToEventThreadQueue;
            std::unique_ptr<std::thread>                      m_EventThread = nullptr;

            void internal_cleanup();
            void internal_startup(Window& super, const std::string& name);
            void internal_push(WindowEventThreadOnlyCommands command);
            std::optional<sf::Event> internal_try_pop();
            void internal_update_loop();
        public:
            WindowThread(WindowData&);
            ~WindowThread();

            bool operator==(bool rhs) const {
                bool res = (bool)m_EventThread.get();
                return (rhs) ? res : !res;
            }
            explicit operator bool() const {
                return (bool)m_EventThread.get();
            }
    };
}
#endif