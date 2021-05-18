#pragma once
#ifndef ENGINE_WINDOW_DATA_H
#define ENGINE_WINDOW_DATA_H

class Window;

namespace Engine::priv {
    class  EngineCore;
    class  EventManager;
    class  WindowThread;
    class  EngineEventHandler;
};

#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <serenity/system/window/WindowThread.h>
#include <serenity/types/Flag.h>
#include <serenity/dependencies/glm.h>

//#define ENGINE_FORCE_DISABLE_THREAD_WINDOW_EVENTS

#if !defined(_APPLE_) && !defined(ENGINE_FORCE_DISABLE_THREAD_WINDOW_EVENTS) && !defined(ENGINE_THREAD_WINDOW_EVENTS)
    #define ENGINE_THREAD_WINDOW_EVENTS
#endif

namespace Engine::priv {
    class WindowData final {
        friend class Engine::priv::EngineCore;
        friend class Engine::priv::EventManager;
        friend class Engine::priv::WindowThread;
        friend class Engine::priv::EngineEventHandler;
        friend class Window;
        private:
            #ifdef ENGINE_THREAD_WINDOW_EVENTS
                Engine::priv::WindowThread  m_WindowThread;
            #endif

            glm::uvec2                      m_OldWindowSize = glm::uvec2(0, 0);
            uint32_t                        m_Style;
            sf::VideoMode                   m_VideoMode;
            std::string                     m_WindowName;
            sf::RenderWindow                m_SFMLWindow;
            uint32_t                        m_FramerateLimit = 0U;
            bool                            m_UndergoingClosing = false;
            Engine::Flag<uint16_t>          m_Flags;
            std::string                     m_IconFile;
            std::thread::id                 m_OpenGLThreadID;

            glm::vec2                       m_MousePosition = glm::vec2(0.0f);
            glm::vec2                       m_MousePosition_Previous = glm::vec2(0.0f);
            glm::vec2                       m_MouseDifference = glm::vec2(0.0f);
            double                          m_MouseDelta = 0.0;

            sf::ContextSettings             m_SFContextSettings;

            void internal_restore_state(Window&);
            const sf::ContextSettings internal_create(Window&, const std::string& name);
            void internal_update_mouse_position(Window&, float x, float y, bool resetDifference, bool resetPrevious);
            void internal_on_fullscreen(Window&, bool isToBeFullscreen, bool isMaximized, bool isMinimized);
            [[nodiscard]] sf::VideoMode internal_get_default_desktop_video_mode();
            void internal_init_position(Window&);

            void internal_on_mouse_wheel_scrolled(float delta, int x, int y);

            void internal_on_reset_events(const float dt);

            void internal_on_close();
        public:
            WindowData();
            ~WindowData();
    };
};
#endif