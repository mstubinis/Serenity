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
		class EventManager;     class SoundManager;  class RenderManager;    class ComponentManager;  class ThreadManager;
		class ResourceManager;  class TimeManager;   class PhysicsManager;   class EventDispatcher;   class NoiseManager;
		class Core final{
		    public:
				static Core* m_Engine;

				EventManager* m_EventManager;
				ResourceManager* m_ResourceManager;
				TimeManager* m_TimeManager;
				SoundManager* m_SoundManager;
				PhysicsManager* m_PhysicsManager;
				RenderManager* m_RenderManager;
				EventDispatcher* m_EventDispatcher;
				ComponentManager* m_ComponentManager;
				ThreadManager* m_ThreadManager;
				NoiseManager* m_NoiseManager;

				Core(const char* name,uint width,uint height);
				~Core();
		};
	};
    void init(const char* name,uint width=0,uint height=0);
    void run();

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
