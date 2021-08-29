#pragma once
#ifndef ENGINE_WINDOW_DATA_H
#define ENGINE_WINDOW_DATA_H

class  Window;
struct EngineOptions;
namespace Engine::priv {
    class  EngineCore;
    class  EventManager;
    class  EngineEventHandler;
};

#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <serenity/types/Flag.h>
#include <serenity/system/window/WindowIncludes.h>
#include <serenity/containers/Queue_ts.h>
#include <serenity/dependencies/glm.h>
#include <atomic>
#include <latch>

namespace Engine::priv {
    class WindowData final {
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
        using WindowEventType         = std::optional<sf::Event>;
        using WindowEventQueue        = Engine::queue_ts<sf::Event>;
        using WindowEventCommandQueue = Engine::queue_ts<WindowEventThreadOnlyCommands>;
#else
        using WindowEventType         = std::optional<sf::Event>;
        using WindowEventQueue        = std::queue<WindowEventType>;
        using WindowEventCommandQueue = std::queue<WindowEventThreadOnlyCommands>;
#endif
        friend class Engine::priv::EngineCore;
        friend class Engine::priv::EventManager;
        friend class Engine::priv::EngineEventHandler;
        friend class Window;
        private:
            sf::ContextSettings             m_SFContextSettings;
            glm::vec2                       m_MousePosition          = glm::vec2{ 0.0f };
            glm::vec2                       m_MousePosition_Previous = glm::vec2{ 0.0f };
            glm::vec2                       m_MouseDifference        = glm::vec2{ 0.0f };
            glm::uvec2                      m_OldWindowSize          = glm::uvec2{ 0, 0 };
            sf::VideoMode                   m_VideoMode;
            std::string                     m_IconFile;
            std::string                     m_WindowTitle;
            sf::RenderWindow                m_SFMLWindow;
            double                          m_MouseDelta             = 0.0;
            uint32_t                        m_Style                  = static_cast<uint32_t>(sf::Style::Default);
            uint32_t                        m_FramerateLimit         = 0U;
            Engine::Flag<uint16_t>          m_Flags                  = (Window_Flags::Windowed | Window_Flags::MouseVisible | Window_Flags::KeyRepeat);
            std::thread::id                 m_OpenGLThreadID;
            std::atomic<bool>               m_UndergoingClosing      = false;


            //window thread
            WindowEventQueue                m_SFEventQueue;
            WindowEventCommandQueue         m_MainThreadToEventThreadQueue;
            std::unique_ptr<std::jthread>   m_EventThread = nullptr;

            void internal_populate_sf_event_queue(bool isUndergoingClosing);
            void internal_process_command_queue();
            void internal_thread_startup(Window&, const std::string& name, std::latch*);
            void internal_push(WindowEventThreadOnlyCommands);
            WindowEventType internal_try_pop() noexcept;
            /////////////////////////////////



            void internal_restore_state(Window&);
            const sf::ContextSettings internal_create(Window&, const std::string& windowTitle);
            void internal_update_mouse_position(Window&, float x, float y, bool resetDifference, bool resetPrevious);
            void internal_on_fullscreen(Window&, bool isToBeFullscreen, bool isMaximized, bool isMinimized);
            [[nodiscard]] sf::VideoMode internal_get_default_desktop_video_mode();
            void internal_init_position(Window&);
            void internal_on_mouse_wheel_scrolled(float delta, int x, int y);
            void internal_update_on_reset_events(const float dt);
            void internal_on_close(bool skipRenderThisFrame = true);
        public:
            WindowData() = default;
            ~WindowData();
    };
};
#endif