#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <GL/glew.h>
#include <GL/GL.h>
#include <SFML/Graphics.hpp>
class Game;
namespace Engine{
	class EngineClass{
		private:
			void _INIT_Window(std::string name, unsigned int width, unsigned int height);
			void _INIT_Game();

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

			void _EVENT_HANDLERS(sf::Event);
			void _RESET_EVENTS();

			bool m_DrawDebug;

			Game* game;
			GLuint m_vao;
			sf::Clock clock;

			void _Update(float);
			void _Render();

		public:
			EngineClass(std::string name, unsigned int width, unsigned int height);
			~EngineClass();

			void Run();
	};
};
#endif