#ifndef ENGINE_EVENTS_H
#define ENGINE_EVENTS_H

#include <string>
#include <SFML/Window.hpp>

#include <unordered_map>
#include <boost/algorithm/string.hpp> 

#include <glm/glm.hpp>

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
					static float m_Delta;
					static MouseButton m_currentButton;
					static MouseButton m_previousButton;
					static std::unordered_map<unsigned int,bool> m_MouseStatus;

					static glm::vec2 m_Position, m_Position_Previous,m_Difference;

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
			};
			static glm::vec2 getMouseDifference(){ return MouseProcessing::m_Difference; }
			static glm::vec2 getMousePositionPrevious(){ return MouseProcessing::m_Position_Previous; }
			static glm::vec2 getMousePosition(){ return MouseProcessing::m_Position; }
			static bool isMouseButtonDown(std::string str){ return MouseProcessing::_IsMouseButtonDown(str); }
			static bool isMouseButtonDownOnce(std::string str){ return MouseProcessing::_IsMouseButtonDownOnce(str); }
			static float getMouseWheelDelta(){ return MouseProcessing::m_Delta; }
		};
		namespace Keyboard{
			class KeyProcessing{
				private:
					static std::unordered_map<std::string,sf::Keyboard::Key> m_KeyMap;
				public:
					static sf::Keyboard::Key m_currentKey;
					static sf::Keyboard::Key m_previousKey;
					static std::unordered_map<unsigned int,bool> m_KeyStatus;

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
			static bool isKeyDown(std::string str){ return KeyProcessing::_IsKeyDown(str); }
			static bool isKeyDownOnce(std::string str){ return KeyProcessing::_IsKeyDownOnce(str); }
			static bool isKeyUp(std::string str){ return KeyProcessing::_IsKeyUp(str); }
		};
	};
};
#endif