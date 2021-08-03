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

    //controls the execution of the window's event polling. Can use a separate thread or be a part of the main thread depending on
    //preprocessor directive ENGINE_THREAD_WINDOW_EVENTS
    class WindowThread final {
        friend class WindowData;
        friend class Window;
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
        using EventType         = std::optional<sf::Event>;
        using EventQueue        = Engine::queue_ts<sf::Event>;
        using EventCommandQueue = Engine::queue_ts<WindowEventThreadOnlyCommands>;
#else
        using EventType         = std::optional<sf::Event>;
        using EventQueue        = std::queue<EventType>;
        using EventCommandQueue = std::queue<WindowEventThreadOnlyCommands>;
#endif
        private:
            EventQueue                      m_SFEventQueue;
            EventCommandQueue               m_MainThreadToEventThreadQueue;
            std::unique_ptr<std::jthread>   m_EventThread = nullptr;
            WindowData&                     m_Data;

            void internal_populate_sf_event_queue();
            void internal_process_command_queue();
            void internal_thread_startup(Window&, const std::string& name, boost::latch*);
            void internal_push(WindowEventThreadOnlyCommands);
            EventType internal_try_pop() noexcept;
        public:
            WindowThread(WindowData&);

            inline bool isUsingSeparateThread() const noexcept { return m_EventThread != nullptr; }
    };
}
#endif