#pragma once
#ifndef ENGINE_WINDOW_THREAD_H
#define ENGINE_WINDOW_THREAD_H

class  Window;
struct EngineOptions;

#include <serenity/system/window/WindowIncludes.h>
#include <serenity/containers/Queue_ts.h>
#include <SFML/Window/Event.hpp>
#include <boost/thread/latch.hpp>
#include <string>
#include <thread>

namespace Engine::priv {
    class WindowData;
    class WindowThread final {
        friend class WindowData;
        friend class Window;
        private:
            WindowData&                                       m_Data;
            Engine::queue_ts<sf::Event>                       m_SFEventQueue;
            Engine::queue_ts<WindowEventThreadOnlyCommands>   m_MainThreadToEventThreadQueue;
            std::unique_ptr<std::jthread>                     m_EventThread = nullptr;

            void internal_startup(Window&, const std::string& name, boost::latch*);
            void internal_push(WindowEventThreadOnlyCommands);
            std::optional<sf::Event> internal_try_pop();
            void internal_update_loop();
        public:
            WindowThread(WindowData&);

            bool operator==(bool rhs) const noexcept {
                bool res = bool(m_EventThread.get());
                return (rhs) ? res : !res;
            }
            inline explicit operator bool() const noexcept { return bool(m_EventThread.get()); }
    };
}
#endif