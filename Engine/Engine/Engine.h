#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <GL/glew.h>
#include <GL/GL.h>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Engine_Events.h"
#include "Engine_Physics.h"

namespace Engine{

	static sf::Window* getWindow(){ return Resources::Detail::ResourceManagement::m_Window; }
	static sf::Vector2u getWindowSize(){ return Resources::Detail::ResourceManagement::m_Window->getSize(); }
	static sf::Mouse* getMouse(){ return Resources::Detail::ResourceManagement::m_Mouse; }
	static void setWindowIcon(Texture* texture){
		texture->generatePixelPointer();
		Resources::getWindow()->setIcon(texture->getWidth(),texture->getHeight(),texture->getPixelsPtr()); 
	}
	static void showMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(true); }
	static void hideMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(false); }
	static void stop(){ Resources::getWindow()->close(); }

	class EngineClass final{
		private:
			void _initWindow(std::string name, unsigned int width, unsigned int height);
			void _initGame();

			#pragma region Event Handlers
			void _EVENT_RESIZE(unsigned int width, unsigned int height);
			void _EVENT_CLOSE();
			void _EVENT_LOST_FOCUS();
			void _EVENT_GAINED_FOCUS();
			void _EVENT_TEXT_ENTERED(sf::Event::TextEvent);
			void _EVENT_KEY_PRESSED(sf::Event::KeyEvent);
			void _EVENT_KEY_RELEASED(sf::Event::KeyEvent);
			void _EVENT_MOUSE_WHEEL_MOVED(sf::Event::MouseWheelEvent);
			void _EVENT_MOUSE_BUTTON_PRESSED(sf::Event::MouseButtonEvent);
			void _EVENT_MOUSE_BUTTON_RELEASED(sf::Event::MouseButtonEvent);
			void _EVENT_MOUSE_MOVED(sf::Event::MouseMoveEvent);
			void _EVENT_MOUSE_ENTERED();
			void _EVENT_MOUSE_LEFT();
			/*
			void _EVENT_JOYSTICK_BUTTON_PRESSED();
			void _EVENT_JOYSTICK_BUTTON_RELEASED();
			void _EVENT_JOYSTICK_MOVED();
			void _EVENT_JOYSTICK_CONNECTED();
			void _EVENT_JOYSTICK_DISCONNECTED();
			*/
			#pragma endregion

			void _RESET_EVENTS();

			GLuint m_vao;
			sf::Clock clock;

			void _update(float);
			void _render();

		public:
			EngineClass(std::string app_name, unsigned int width=0, unsigned int height=0);
			~EngineClass();

			void run();
	};
};

namespace Game{
	void initResources();
	void initLogic();
	void update(float);
	void render();
	void cleanup();
};
#endif