#pragma once
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

class Texture;
class Engine_Window;

#include <memory>
#include <glm/fwd.hpp>

typedef unsigned int uint;

namespace Engine{
	namespace epriv{
		class EventManager;  class ResourceManager;  class TimeManager;  class SoundManager;
		class Core final{
		    private:
				class impl; std::unique_ptr<impl> m_i;
		    public:
				static Core* m_Engine;

				EventManager* m_EventManager;
				ResourceManager* m_ResourceManager;
				TimeManager* m_TimeManager;
				SoundManager* m_SoundManager;

				Core(const char* name,uint width,uint height);
				~Core();
		};
	};


    void init(const char* name,uint width=0,uint height=0);
    void run();
    namespace Detail{
         class EngineClass final{
            public:
                static void initGame(const char* name,uint w,uint h);

                #pragma region Event Handlers
                static void EVENT_RESIZE(uint width, uint height,bool saveSize = true);
                static void EVENT_CLOSE();
                static void EVENT_LOST_FOCUS();
                static void EVENT_GAINED_FOCUS();
                static void EVENT_TEXT_ENTERED(uint);
                static void EVENT_KEY_PRESSED(uint);
                static void EVENT_KEY_RELEASED(uint);
                static void EVENT_MOUSE_WHEEL_MOVED(int delta);
                static void EVENT_MOUSE_BUTTON_PRESSED(uint);
                static void EVENT_MOUSE_BUTTON_RELEASED(uint);
                static void EVENT_MOUSE_MOVED(float mouseX,float mouseY);
                static void EVENT_MOUSE_ENTERED();
                static void EVENT_MOUSE_LEFT();
                static void EVENT_JOYSTICK_BUTTON_PRESSED();
                static void EVENT_JOYSTICK_BUTTON_RELEASED();
                static void EVENT_JOYSTICK_MOVED();
                static void EVENT_JOYSTICK_CONNECTED();
                static void EVENT_JOYSTICK_DISCONNECTED();
                #pragma endregion

                static void RESET_EVENTS();

				static void handleEvents();
				static void updatePhysics();
				static void updateSounds();
                static void update();
                static void render();
        };
    };
    const float getFPS();
    Engine_Window* getWindow();
    const glm::uvec2 getWindowSize();
    void setWindowIcon(Texture* texture);
    void showMouseCursor();
    void hideMouseCursor();
    void stop();
    void setFullScreen(bool b);
};

namespace Game{
    void initResources();
    void initLogic();
    void update(float);
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
