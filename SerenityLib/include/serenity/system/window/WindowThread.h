#pragma once
#ifndef ENGINE_WINDOW_THREAD_H
#define ENGINE_WINDOW_THREAD_H

class Window;

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

            void internal_startup(Window& super, const std::string& name, boost::latch* bLatch);
            void internal_push(WindowEventThreadOnlyCommands command);
            std::optional<sf::Event> internal_try_pop();
            void internal_update_loop();
        public:
            WindowThread(WindowData&);

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