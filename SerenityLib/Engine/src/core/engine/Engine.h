#pragma once
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include <core/engine/Engine_Debugging.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/events/Engine_Events.h>
#include <core/engine/math/SimplexNoise.h>
#include <core/engine/Engine_Window.h>


struct EngineOptions final {
    AntiAliasingAlgorithm::Algorithm aa_algorithm;
    bool                             ssao_enabled;
    bool                             hdr_enabled;
    bool                             fullscreen;
    bool                             god_rays_enabled;
    bool                             fog_enabled;
    unsigned int                     width;
    unsigned int                     height;
    const char*                      window_title;
    EngineOptions(){
        window_title     = "Engine";
        width            = 800;
        height           = 600;
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
        struct Core final{
            static Core*          m_Engine;

            EventManager          m_EventManager;
            PhysicsManager        m_PhysicsManager;
            ResourceManager       m_ResourceManager;
            DebugManager          m_DebugManager;
            SoundManager          m_SoundManager;
            RenderManager         m_RenderManager;
            ThreadManager         m_ThreadManager;
            NoiseManager          m_NoiseManager;

            bool                  m_Paused, m_Destroyed;

            Core(const EngineOptions&);
            ~Core();
        };
    };
    void init(const EngineOptions&);
    void pause(const bool& pause = true);
    bool paused();
    void unpause();
    void run();

    const float getFPS();
    Engine_Window& getWindow();
    const glm::uvec2 getWindowSize();
    void setWindowIcon(const Texture& texture);
    void showMouseCursor();
    void hideMouseCursor();
    void stop();
    void setFullScreen(const bool& b);
};

namespace Game{
    void initResources();
    void initLogic();
    void update(const double& dt);
    void render();
    void cleanup();

    #pragma region EventHandlers
    void onResize(const uint& width, const uint& height);
    void onClose();
    void onLostFocus();
    void onGainedFocus();
    void onTextEntered(const uint&);
    void onKeyPressed(const uint&);
    void onKeyReleased(const uint&);
    void onMouseWheelMoved(const int& delta);
    void onMouseButtonPressed(const uint&);
    void onMouseButtonReleased(const uint&);
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
