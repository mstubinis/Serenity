#pragma once
#ifndef ENGINE_WINDOW_DATA_H
#define ENGINE_WINDOW_DATA_H

class  Window;
struct EngineOptions;
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

namespace Engine::priv {
    class WindowData final {
        friend class Engine::priv::EngineCore;
        friend class Engine::priv::EventManager;
        friend class Engine::priv::WindowThread;
        friend class Engine::priv::EngineEventHandler;
        friend class Window;
        private:
            Engine::priv::WindowThread      m_WindowThread;

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
            uint32_t                        m_Style                  = 0U;
            uint32_t                        m_FramerateLimit         = 0U;
            Engine::Flag<uint16_t>          m_Flags;
            std::thread::id                 m_OpenGLThreadID;
            bool                            m_UndergoingClosing      = false;

            void internal_restore_state(Window&);
            const sf::ContextSettings internal_create(Window&, const std::string& windowTitle);
            void internal_update_mouse_position(Window&, float x, float y, bool resetDifference, bool resetPrevious);
            void internal_on_fullscreen(Window&, bool isToBeFullscreen, bool isMaximized, bool isMinimized);
            [[nodiscard]] sf::VideoMode internal_get_default_desktop_video_mode();
            void internal_init_position(Window&);
            void internal_on_mouse_wheel_scrolled(float delta, int x, int y);
            void internal_update_on_reset_events(const float dt);
            void internal_on_close();
        public:
            WindowData();
            ~WindowData();
    };
};
#endif