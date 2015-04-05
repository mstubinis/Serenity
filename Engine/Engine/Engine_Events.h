#ifndef ENGINE_EVENTS_H
#define ENGINE_EVENTS_H

#include <string>
#include <SFML\Window.hpp>

#include <unordered_map>
#include <boost/algorithm/string.hpp> 

namespace Engine{
	namespace Events{
		namespace Mouse{
			enum MouseButton{
				MOUSE_BUTTON_NONE,
				MOUSE_BUTTON_LEFT,
				MOUSE_BUTTON_RIGHT,
				MOUSE_BUTTON_MIDDLE,
				MOUSE_BUTTON_TYPE_NUMBER
			};
			class MouseProcessing{
				private:
					static std::unordered_map<std::string,MouseButton> m_MouseMap;
				public:
					static int m_Delta;
					static MouseButton m_currentButton;
					static MouseButton m_previousButton;
					static std::unordered_map<MouseButton,bool> m_MouseStatus;

					static bool MouseProcessing::_IsMouseButtonDown(std::string str){
						boost::algorithm::to_lower(str);
						MouseButton key = MouseProcessing::m_MouseMap[str];
						if(MouseProcessing::m_MouseStatus[key] == true)
							return true;
						return false;
					}
					static bool MouseProcessing::_IsMouseButtonDownOnce(std::string str){
						bool result = MouseProcessing::_IsMouseButtonDown(str);
						MouseButton key = MouseProcessing::m_MouseMap[str];
						if(result == true && m_currentButton == key && (m_currentButton != m_previousButton))
							return true;
						return false;
					}
					static int MouseProcessing::_GetMouseWheelDelta(){ return m_Delta; }
					static void MouseProcessing::_SetMouseWheelDelta(int d){ m_Delta = d; }
			};
			static bool IsMouseButtonDown(std::string str){ return MouseProcessing::_IsMouseButtonDown(str); }
			static bool IsMouseButtonDownOnce(std::string str){ return MouseProcessing::_IsMouseButtonDownOnce(str); }
			static int GetMouseWheelDelta(){ return MouseProcessing::_GetMouseWheelDelta(); }
		};
		namespace Keyboard{
			class KeyProcessing{
				private:
					static std::unordered_map<std::string,sf::Keyboard::Key> m_KeyMap;
				public:
					static sf::Keyboard::Key m_currentKey;
					static sf::Keyboard::Key m_previousKey;
					static std::unordered_map<sf::Keyboard::Key,bool> m_KeyStatus;

					static bool KeyProcessing::_IsKeyDown(std::string str){
						boost::algorithm::to_lower(str);
						sf::Keyboard::Key key = KeyProcessing::m_KeyMap[str];
						if(KeyProcessing::m_KeyStatus[key] == true)
							return true;
						return false;
					}
					static bool KeyProcessing::_IsKeyUp(std::string str){
						boost::algorithm::to_lower(str);
						sf::Keyboard::Key key = KeyProcessing::m_KeyMap[str];
						if(KeyProcessing::m_KeyStatus[key] == false)
							return true;
						return false;
					}
					static bool KeyProcessing::_IsKeyDownOnce(std::string str){
						bool result = KeyProcessing::_IsKeyDown(str);
						sf::Keyboard::Key key = KeyProcessing::m_KeyMap[str];
						if(result == true && m_currentKey == key && (m_currentKey != m_previousKey))
							return true;
						return false;
					}
			};
			static bool IsKeyDown(std::string str){ return KeyProcessing::_IsKeyDown(str); }
			static bool IsKeyDownOnce(std::string str){ return KeyProcessing::_IsKeyDownOnce(str); }
			static bool IsKeyUp(std::string str){ return KeyProcessing::_IsKeyUp(str); }
		};
	};
};
#endif