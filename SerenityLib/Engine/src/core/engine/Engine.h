#pragma once
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include <core/engine/utils/Engine_Debugging.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/math/SimplexNoise.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/threading/Engine_ThreadManager.h>

#define PHYSICS_MIN_STEP 0.016666666666666666f

struct EngineOptions final {
    AntiAliasingAlgorithm::Algorithm   aa_algorithm;
    bool                               ssao_enabled;
    bool                               hdr_enabled;
    bool                               fullscreen;
    bool                               god_rays_enabled;
    bool                               fog_enabled;
    unsigned int                       width;
    unsigned int                       height;
    const char*                        window_title;
    EngineOptions(){
        window_title     = "Engine";
        width            = 1024;
        height           = 768;
        ssao_enabled     = true;
        hdr_enabled      = true;
        god_rays_enabled = true;
        fullscreen       = false;
        fog_enabled      = false;
        aa_algorithm     = AntiAliasingAlgorithm::FXAA;
    }
};


namespace Engine{
    namespace epriv{
        class EngineCore final {
            private:
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
                    SimplexNoise      m_SimplexNoise;
                    bool              m_Paused;
                    bool              m_Destroyed;
                };
                Misc                  m_Misc;

                EventManager          m_EventManager;
                PhysicsManager        m_PhysicsManager;
                ResourceManager       m_ResourceManager;
                DebugManager          m_DebugManager;
                SoundManager          m_SoundManager;
                RenderManager         m_RenderManager;
                ThreadManager         m_ThreadManager;


                EngineCore(const EngineOptions& options);
                ~EngineCore();

                static void init(const EngineOptions& options);
                void run();
                void on_event_resize(const unsigned int& w, const unsigned int& h, const bool& saveSize);
                void handle_events();
        };
        struct Core final{
            static EngineCore*          m_Engine;
        };
    };
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
    void setFullScreen(const bool& isFullscreen);
};

namespace Game{
    void initResources();
    void initLogic();
    void update(const double& dt);
    void render();
    void cleanup();

    #pragma region EventHandlers
    void onResize(const unsigned int& width, const unsigned int& height);
    void onClose();
    void onLostFocus();
    void onGainedFocus();
    void onTextEntered(const unsigned int& unicode);
    void onKeyPressed(const unsigned int& key);
    void onKeyReleased(const unsigned int& key);
    void onMouseWheelMoved(const int& delta);
    void onMouseButtonPressed(const unsigned int& button);
    void onMouseButtonReleased(const unsigned int& button);
    void onMouseMoved(const float& mouseX, const float& mouseY);
    void onMouseEntered();
    void onMouseLeft();
    void onPreUpdate(const double& dt);
    void onPostUpdate(const double& dt);
    void onJoystickButtonPressed();
    void onJoystickButtonReleased();
    void onJoystickMoved();
    void onJoystickConnected();
    void onJoystickDisconnected();
    #pragma endregion
};
#endif
