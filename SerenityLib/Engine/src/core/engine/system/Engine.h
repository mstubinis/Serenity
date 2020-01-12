#pragma once
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

struct EngineOptions;
class  Engine_Window;

#include <core/engine/utils/Engine_Debugging.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/physics/Engine_Physics.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/math/SimplexNoise.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/discord/Discord.h>


namespace Engine{
    void pause(const bool& pause = true);
    bool paused();
    void unpause();

    const float getFPS();
    Engine_Window& getWindow();
    const glm::uvec2 getWindowSize();
    void setWindowIcon(const Texture& texture);
    void showMouseCursor();
    void hideMouseCursor();
    void stop();
    const bool setFullscreen(const bool& isFullscreen);
    const bool setFullscreenWindowed(const bool& isFullscreen);
    namespace epriv{
        class EngineCore final {
            friend class Engine_Window;
            private:

                void init_os_specific(const EngineOptions& options);
                void init_window(const EngineOptions& options);

                void on_event_resize(const unsigned int& w, const unsigned int& h, const bool& saveSize);
                void on_event_close();
                void on_event_lost_focus();
                void on_event_gained_focus();
                void on_event_text_entered(const unsigned int& unicode);
                void on_event_key_pressed(const unsigned int& key);
                void on_event_key_released(const unsigned int& key);
                void on_event_mouse_wheel_moved(const int& delta);
                void on_event_mouse_button_pressed(const unsigned int& mouseButton);
                void on_event_mouse_button_released(const unsigned int& mouseButton);
                void on_event_mouse_moved(const int& mouseX, const int& mouseY);
                void on_event_mouse_entered();
                void on_event_mouse_left();
                void on_event_joystick_button_pressed(const unsigned int& button, const unsigned int& id);
                void on_event_joystick_button_released(const unsigned int& button, const unsigned int& id);
                void on_event_joystick_moved(const unsigned int& id, const float& position, const unsigned int axis);
                void on_event_joystick_connected(const unsigned int& id);
                void on_event_joystick_disconnected(const unsigned int& id);

                void update_logic(const double& dt);
                void update_sounds(const double& dt);
                void update_physics(const double& dt);
                void update(const double& dt);
                void render(const double& dt);
                void cleanup(const double& dt);

            public:
                struct Misc final {
                    Engine::Discord::DiscordCore   m_DiscordCore;
                    SimplexNoise                   m_SimplexNoise;
                    bool                           m_Paused;
                    bool                           m_Destroyed;
                };
                Misc                  m_Misc;

                EventManager          m_EventManager;
                RenderManager         m_RenderManager;
                PhysicsManager        m_PhysicsManager;
                ResourceManager       m_ResourceManager;
                SoundManager          m_SoundManager;
                DebugManager          m_DebugManager;
                ThreadManager         m_ThreadManager;


                EngineCore(const EngineOptions& options);
                ~EngineCore();

                void init(const EngineOptions& options);
                void run();
                void handle_events();
        };
        struct Core final{
            static EngineCore*          m_Engine;
        };
    };
};

#endif