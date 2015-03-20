#ifndef ENGINE_H
#define ENGINE_H

class Game;
#include "Engine_Resources.h"
#include "Bullet.h"

namespace Engine{

	enum ENGINE_STATE{ ENGINE_STATE_PLAY,ENGINE_STATE_EDIT,ENGINE_STATE_PAUSE,
		               ENGINE_STATE_NUMBER };

	class EngineClass{
		private:

			ENGINE_STATE m_State;

			void INIT_Window(std::string name, unsigned int width, unsigned int height);
			void INIT_Game();

			void EDIT_Update(float,sf::Event);

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
			void EVENT_MOUSE_MOVED(sf::Event::MouseMoveEvent,bool=true);
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
			void _SWITCH_STATES(ENGINE_STATE);

			bool m_MouseMoved;
			bool m_DrawDebug;
			float m_Timer;

			Game* game;
			GLuint m_vao;

			void Update(float,sf::Event);
			void Render();

		public:
			EngineClass(std::string name, unsigned int width, unsigned int height);
			~EngineClass();

			void Run();
	};
};
#endif