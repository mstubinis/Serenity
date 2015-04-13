#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <GL/glew.h>
#include <GL/GL.h>
#include <SFML/Window.hpp>
class Game;
namespace Engine{
	class EngineClass{
		private:
			void INIT_Window(std::string name, unsigned int width, unsigned int height);
			void INIT_Game();

			#pragma region Event Handlers
			void EVENT_RESIZE(unsigned int width, unsigned int height);
			void EVENT_CLOSE();
			void EVENT_LOST_FOCUS();
			void EVENT_GAINED_FOCUS();
			void EVENT_TEXT_ENTERED(sf::Event::TextEvent);
			void EVENT_KEY_PRESSED(sf::Event::KeyEvent);
			void EVENT_KEY_RELEASED(sf::Event::KeyEvent);
			void EVENT_MOUSE_WHEEL_MOVED(sf::Event::MouseWheelEvent);
			void EVENT_MOUSE_BUTTON_PRESSED(sf::Event::MouseButtonEvent);
			void EVENT_MOUSE_BUTTON_RELEASED(sf::Event::MouseButtonEvent);
			void EVENT_MOUSE_MOVED(sf::Event::MouseMoveEvent);
			void EVENT_MOUSE_ENTERED();
			void EVENT_MOUSE_LEFT();
			void EVENT_JOYSTICK_BUTTON_PRESSED();
			void EVENT_JOYSTICK_BUTTON_RELEASED();
			void EVENT_JOYSTICK_MOVED();
			void EVENT_JOYSTICK_CONNECTED();
			void EVENT_JOYSTICK_DISCONNECTED();
			#pragma endregion

			void _EVENT_HANDLERS(sf::Event);
			void _RESET_EVENTS();

			bool m_DrawDebug;

			Game* game;
			GLuint m_vao;

			void _Update(float);
			void _Render();

		public:
			EngineClass(std::string name, unsigned int width, unsigned int height);
			~EngineClass();

			void Run();
	};
};
#endif