#pragma once
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include "Engine_Time.h"
#include "Engine_EventDispatcher.h"
#include "Engine_ThreadManager.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Engine_Sounds.h"
#include "Engine_Events.h"
#include "Engine_Noise.h"
#include "Engine_Window.h"
#include "Components.h"

typedef unsigned int uint;
namespace Engine{
    namespace epriv{
        struct Core final{
            static Core*          m_Engine;

            EventManager          m_EventManager;
            EventDispatcher       m_EventDispatcher;
            ResourceManager       m_ResourceManager;
            TimeManager           m_TimeManager;
            SoundManager          m_SoundManager;
            PhysicsManager        m_PhysicsManager;
            RenderManager         m_RenderManager;
            OLD_ComponentManager  m_ComponentManager;
            ThreadManager         m_ThreadManager;
            NoiseManager          m_NoiseManager;
            bool                  m_Paused, m_Destroyed;

            Core(const char* name,uint width,uint height);
            ~Core();
        };
    };
    void init(const char* name,uint width=0,uint height=0);
    void pause(bool=true);
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
    void setFullScreen(bool b);
};

namespace Game{
    void initResources();
    void initLogic();
    void update(const float& dt);
    void render();
    void cleanup();

    #pragma region EventHandlers
    void onResize(uint width,uint height);
    void onClose();
    void onLostFocus();
    void onGainedFocus();
    void onTextEntered(uint);
    void onKeyPressed(uint);
    void onKeyReleased(uint);
    void onMouseWheelMoved(int delta);
    void onMouseButtonPressed(uint);
    void onMouseButtonReleased(uint);
    void onMouseMoved(float mouseX,float mouseY);
    void onMouseEntered();
    void onMouseLeft();
    void onPreUpdate(float dt);
    void onPostUpdate(float dt);
    void onJoystickButtonPressed();
    void onJoystickButtonReleased();
    void onJoystickMoved();
    void onJoystickConnected();
    void onJoystickDisconnected();
    #pragma endregion
};
#endif
