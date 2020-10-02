#pragma once
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

struct EngineOptions;
class  Window;
namespace Engine::priv {
    class WindowData;
};

#include <core/engine/utils/Engine_Debugging.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/physics/Engine_Physics.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/sounds/SoundModule.h>
#include <core/engine/events/EventModule.h>
#include <core/engine/math/SimplexNoise.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/networking/NetworkingModule.h>
#include <core/engine/discord/DiscordModule.h>
#include <core/engine/lua/LuaModule.h>
#include <core/engine/mesh/BuiltInMeshes.h>

namespace Engine{
    //void reset_malloc_count() noexcept;
    //void print_malloc_count() noexcept;

    void pause(bool pause = true);
    bool paused();
    void unpause();

    float getFPS();
    Window& getWindow();
    glm::uvec2 getWindowSize();
    void setWindowIcon(const Texture& texture);
    void showMouseCursor();
    void hideMouseCursor();
    void setTimeScale(float timeScale);
    void stop();
    bool setFullscreen(bool isFullscreen);
    bool setFullscreenWindowed(bool isFullscreen);
    namespace priv{
        class EngineCore final {
            friend class Window;
            friend class Engine::priv::WindowData;
            private:
                void internal_init_os_specific(const EngineOptions& options);
                void internal_cleanup_os_specific();

                /* The sf::Event::Resized event is triggered when the window is resized,
                either through user action or programmatically by calling window.setSize. */
                void internal_on_event_resize(Window& window, unsigned int w, unsigned int h, bool saveSize);

                /* Triggered when the game application fully closes, by calling Engine::stop(). This triggers Game::onGameEnded(), and this will be called before Game::cleanup() */
                void internal_on_event_game_ended();


                /* Triggered when a window has closed via Window::close() */
                void internal_on_event_window_closed(Window& window);

                /* The sf::Event::Closed event is triggered when the user wants to close the window,
                through any of the possible methods the window manager provides ("close" button, keyboard shortcut, etc.).
                This event only represents a close request, the window is not yet closed when the event is received.

                Typical code will just call window.close() in reaction to this event, to actually close the window.
                However, you may also want to do something else first, like saving the current application state or asking the user what to do.
                If you don't do anything, the window remains open. */
                void internal_on_event_window_requested_closed(Window& window);

                /* The sf::Event::LostFocus and sf::Event::GainedFocus events are triggered when the window loses/gains focus,
                which happens when the user switches the currently active window. When the window is out of focus, it doesn't receive keyboard events. */
                void internal_on_event_lost_focus(Window& window);

                /* The sf::Event::LostFocus and sf::Event::GainedFocus events are triggered when the window loses/gains focus,
                which happens when the user switches the currently active window. When the window is out of focus, it doesn't receive keyboard events. */
                void internal_on_event_gained_focus(Window& window);

                /* The sf::Event::TextEntered event is triggered when a character is typed.
                This must not be confused with the KeyPressed event: TextEntered interprets the user input and produces the appropriate printable character.
                For example, pressing '^' then 'e' on a French keyboard will produce two KeyPressed events, but a single TextEntered event containing the '�' character.
                It works with all the input methods provided by the operating system, even the most specific or complex ones.
                This event is typically used to catch user input in a text field. */
                void internal_on_event_text_entered(Window& window, unsigned int unicode);


                /* The sf::Event::KeyPressed and sf::Event::KeyReleased events are triggered when a keyboard key is pressed/released.
                If a key is held, multiple KeyPressed events will be generated, at the default operating system delay
                (ie. the same delay that applies when you hold a letter in a text editor). To disable repeated KeyPressed events, you can call window.setKeyRepeatEnabled(false).
                On the flip side, it is obvious that KeyReleased events can never be repeated.
                This event is the one to use if you want to trigger an action exactly once when a key is pressed or released,
                like making a character jump with space, or exiting something with escape. */
                void internal_on_event_key_pressed(Window& window, unsigned int key);

                /* The sf::Event::KeyPressed and sf::Event::KeyReleased events are triggered when a keyboard key is pressed/released.
                If a key is held, multiple KeyPressed events will be generated, at the default operating system delay
                (ie. the same delay that applies when you hold a letter in a text editor). To disable repeated KeyPressed events, you can call window.setKeyRepeatEnabled(false).
                On the flip side, it is obvious that KeyReleased events can never be repeated.
                This event is the one to use if you want to trigger an action exactly once when a key is pressed or released,
                like making a character jump with space, or exiting something with escape. */
                void internal_on_event_key_released(Window& window, unsigned int key);

                /* The sf::Event::MouseWheelScrolled event is triggered when a mouse wheel moves up or down, but also laterally if the mouse supports it.
                The member associated with this event is event.mouseWheelScroll, it contains the number of ticks the wheel has moved,
                what the orientation of the wheel is and the current position of the mouse cursor. */
                void internal_on_event_mouse_wheel_scrolled(Window& window, float delta, int mouseWheelX, int mouseWheelY);

                /* The sf::Event::MouseButtonPressed and sf::Event::MouseButtonReleased events are
                triggered when a mouse button is pressed/released.
                SFML supports 5 mouse buttons: left, right, middle (wheel), extra #1 and extra #2 (side buttons). */
                void internal_on_event_mouse_button_pressed(Window& window, unsigned int mouseButton);

                /* The sf::Event::MouseButtonPressed and sf::Event::MouseButtonReleased events are
                triggered when a mouse button is pressed/released.
                SFML supports 5 mouse buttons: left, right, middle (wheel), extra #1 and extra #2 (side buttons). */
                void internal_on_event_mouse_button_released(Window& window, unsigned int mouseButton);

                /* The sf::Event::MouseMoved event is triggered when the mouse moves within the window.
                This event is triggered even if the window isn't focused. However, it is triggered
                only when the mouse moves within the inner area of the window, not when it moves over the title bar or borders. */
                void internal_on_event_mouse_moved(Window& window, int mouseX, int mouseY);

                /* The sf::Event::MouseEntered and sf::Event::MouseLeft events are triggered when the mouse cursor enters/leaves the window. */
                void internal_on_event_mouse_entered(Window& window);

                /* The sf::Event::MouseEntered and sf::Event::MouseLeft events are triggered when the mouse cursor enters/leaves the window. */
                void internal_on_event_mouse_left(Window& window);


                void internal_on_event_joystick_button_pressed(Window& window, unsigned int button, unsigned int id);
                void internal_on_event_joystick_button_released(Window& window, unsigned int button, unsigned int id);
                void internal_on_event_joystick_moved(Window& window, unsigned int id, float position, unsigned int axis);
                void internal_on_event_joystick_connected(Window& window, unsigned int id);
                void internal_on_event_joystick_disconnected(Window& window, unsigned int id);

                void internal_update_logic(Scene& scene, Window& window, const float dt);
                void internal_update_sounds(Scene& scene, Window& window, const float dt);
                void internal_update_physics(Window& window, const float dt);
                void internal_update(Window& window, const float dt);
                void internal_render(Window& window, const float dt);
                void internal_cleanup(Window& window, const float dt);
            public:
                class Misc final {
                    public:
                        Engine::priv::BuiltInMeshses   m_BuiltInMeshes;
                        SimplexNoise                   m_SimplexNoise;
                        bool                           m_Paused         = false;
                        bool                           m_Destroyed      = false;
                };
                Misc                m_Misc;

                LUAModule           m_LUAModule;
                NetworkingModule    m_NetworkingModule;
                DiscordModule       m_DiscordModule;
                EventModule         m_EventModule;
                Renderer            m_RenderManager;
                PhysicsManager      m_PhysicsManager;
                ResourceManager     m_ResourceManager;
                SoundModule         m_SoundModule;
                DebugManager        m_DebugManager;
                ThreadingModule     m_ThreadingModule;

                EngineCore(const EngineOptions& options);
                ~EngineCore();

                void init(const EngineOptions& options);
                void run();
                void handle_events(Window& window);
        };
        class Core final{
            public:
                static EngineCore* m_Engine;
        };
    };
};

#endif