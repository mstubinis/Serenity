#include "Engine.h"
#include "Engine_Events.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"

#include <boost/algorithm/string.hpp> 

#include <glm/vec2.hpp>

using namespace Engine;
using namespace std;

class epriv::EventManager::impl final{
    public:
		unordered_map<string,uint> m_KeyMap, m_MouseMap;
		unordered_map<uint,bool> m_KeyStatus, m_MouseStatus;

		float m_Delta;
		glm::vec2 m_Position, m_Position_Previous, m_Difference;

		uint m_currentKey, m_previousKey;
		uint m_currentButton, m_previousButton;

		void _init(const char* name,uint& w,uint& h){
			_initVars();
			_initKeyMap();
			_initMouseMap();
		}
		void _postInit(const char* name,uint& w,uint& h){
		}
		void _initVars(){
			m_Delta = 0;
			m_Position = m_Position_Previous = m_Difference = glm::vec2(0.0f);

		    m_currentKey = m_previousKey = sf::Keyboard::Unknown;
		    m_currentButton = m_previousButton = 100;  //we will use 100 as the "none" key
		}
		void _initKeyMap(){
			if (m_KeyMap.size() > 0) return;
			m_KeyMap["a"] = sf::Keyboard::A;
			m_KeyMap["add"] = sf::Keyboard::Add;
			m_KeyMap["b"] = sf::Keyboard::B;
			m_KeyMap["backslash"] = sf::Keyboard::BackSlash;
			m_KeyMap["back slash"] = sf::Keyboard::BackSlash;
			m_KeyMap["bslash"] = sf::Keyboard::BackSlash;
			m_KeyMap["b slash"] = sf::Keyboard::BackSlash;
			m_KeyMap["\\"] = sf::Keyboard::BackSlash;
			m_KeyMap["backspace"] = sf::Keyboard::BackSpace;
			m_KeyMap["back space"] = sf::Keyboard::BackSpace;
			m_KeyMap["bspace"] = sf::Keyboard::BackSpace;
			m_KeyMap["b space"] = sf::Keyboard::BackSpace;
			m_KeyMap["c"] = sf::Keyboard::C;
			m_KeyMap["comma"] = sf::Keyboard::Comma;
			m_KeyMap[","] = sf::Keyboard::Comma;
			m_KeyMap["d"] = sf::Keyboard::D;
			m_KeyMap["dash"] = sf::Keyboard::Dash;
			m_KeyMap["-"] = sf::Keyboard::Dash;
			m_KeyMap["delete"] = sf::Keyboard::Delete;
			m_KeyMap["del"] = sf::Keyboard::Delete;
			m_KeyMap["divide"] = sf::Keyboard::Divide;
			m_KeyMap["down"] = sf::Keyboard::Down;
			m_KeyMap["down arrow"] = sf::Keyboard::Down;
			m_KeyMap["downarrow"] = sf::Keyboard::Down;
			m_KeyMap["darrow"] = sf::Keyboard::Down;
			m_KeyMap["d arrow"] = sf::Keyboard::Down;
			m_KeyMap["e"] = sf::Keyboard::E;
			m_KeyMap["end"] = sf::Keyboard::End;
			m_KeyMap["equal"] = sf::Keyboard::Equal;
			m_KeyMap["="] = sf::Keyboard::Equal;
			m_KeyMap["esc"] = sf::Keyboard::Escape;
			m_KeyMap["escape"] = sf::Keyboard::Escape;
			m_KeyMap["f"] = sf::Keyboard::F;

			m_KeyMap["f1"] = sf::Keyboard::F1;
			m_KeyMap["f2"] = sf::Keyboard::F2;
			m_KeyMap["f3"] = sf::Keyboard::F3;
			m_KeyMap["f4"] = sf::Keyboard::F4;
			m_KeyMap["f5"] = sf::Keyboard::F5;
			m_KeyMap["f6"] = sf::Keyboard::F6;
			m_KeyMap["f7"] = sf::Keyboard::F7;
			m_KeyMap["f8"] = sf::Keyboard::F8;
			m_KeyMap["f9"] = sf::Keyboard::F9;
			m_KeyMap["f10"] = sf::Keyboard::F10;
			m_KeyMap["f11"] = sf::Keyboard::F11;
			m_KeyMap["f12"] = sf::Keyboard::F12;

			m_KeyMap["g"] = sf::Keyboard::G;
			m_KeyMap["h"] = sf::Keyboard::H;
			m_KeyMap["home"] = sf::Keyboard::Home;
			m_KeyMap["i"] = sf::Keyboard::I;
			m_KeyMap["insert"] = sf::Keyboard::Insert;
			m_KeyMap["j"] = sf::Keyboard::J;
			m_KeyMap["k"] = sf::Keyboard::K;
			m_KeyMap["l"] = sf::Keyboard::L;

			m_KeyMap["lalt"] = sf::Keyboard::LAlt;
			m_KeyMap["leftalt"] = sf::Keyboard::LAlt;
			m_KeyMap["left alt"] = sf::Keyboard::LAlt;
			m_KeyMap["l alt"] = sf::Keyboard::LAlt;

			m_KeyMap["lbracket"] = sf::Keyboard::LBracket;
			m_KeyMap["l bracket"] = sf::Keyboard::LBracket;
			m_KeyMap["leftbracket"] = sf::Keyboard::LBracket;
			m_KeyMap["left bracket"] = sf::Keyboard::LBracket;
			m_KeyMap["["] = sf::Keyboard::LBracket;

			m_KeyMap["lcontrol"] = sf::Keyboard::LControl;
			m_KeyMap["l control"] = sf::Keyboard::LControl;
			m_KeyMap["leftcontrol"] = sf::Keyboard::LControl;
			m_KeyMap["left control"] = sf::Keyboard::LControl;

			m_KeyMap["left"] = sf::Keyboard::Left;
			m_KeyMap["larrow"] = sf::Keyboard::Left;
			m_KeyMap["l arrow"] = sf::Keyboard::Left;
			m_KeyMap["leftarrow"] = sf::Keyboard::Left;
			m_KeyMap["left arrow"] = sf::Keyboard::Left;

			m_KeyMap["lshift"] = sf::Keyboard::LShift;
			m_KeyMap["l shift"] = sf::Keyboard::LShift;
			m_KeyMap["leftshift"] = sf::Keyboard::LShift;
			m_KeyMap["left shift"] = sf::Keyboard::LShift;

			m_KeyMap["lsystem"] = sf::Keyboard::LSystem;
			m_KeyMap["l system"] = sf::Keyboard::LSystem;
			m_KeyMap["leftsystem"] = sf::Keyboard::LSystem;
			m_KeyMap["left system"] = sf::Keyboard::LSystem;

			m_KeyMap["m"] = sf::Keyboard::M;
			m_KeyMap["menu"] = sf::Keyboard::Menu;
			m_KeyMap["multiply"] = sf::Keyboard::Multiply;

			m_KeyMap["n"] = sf::Keyboard::N;
			m_KeyMap["num0"] = sf::Keyboard::Num0;
			m_KeyMap["num1"] = sf::Keyboard::Num1;
			m_KeyMap["num2"] = sf::Keyboard::Num2;
			m_KeyMap["num3"] = sf::Keyboard::Num3;
			m_KeyMap["num4"] = sf::Keyboard::Num4;
			m_KeyMap["num5"] = sf::Keyboard::Num5;
			m_KeyMap["num6"] = sf::Keyboard::Num6;
			m_KeyMap["num7"] = sf::Keyboard::Num7;
			m_KeyMap["num8"] = sf::Keyboard::Num8;
			m_KeyMap["num9"] = sf::Keyboard::Num9;
			m_KeyMap["0"] = sf::Keyboard::Num0;
			m_KeyMap["1"] = sf::Keyboard::Num1;
			m_KeyMap["2"] = sf::Keyboard::Num2;
			m_KeyMap["3"] = sf::Keyboard::Num3;
			m_KeyMap["4"] = sf::Keyboard::Num4;
			m_KeyMap["5"] = sf::Keyboard::Num5;
			m_KeyMap["6"] = sf::Keyboard::Num6;
			m_KeyMap["7"] = sf::Keyboard::Num7;
			m_KeyMap["8"] = sf::Keyboard::Num8;
			m_KeyMap["9"] = sf::Keyboard::Num9;
			m_KeyMap["numpad0"] = sf::Keyboard::Numpad0;
			m_KeyMap["numpad1"] = sf::Keyboard::Numpad1;
			m_KeyMap["numpad2"] = sf::Keyboard::Numpad2;
			m_KeyMap["numpad3"] = sf::Keyboard::Numpad3;
			m_KeyMap["numpad4"] = sf::Keyboard::Numpad4;
			m_KeyMap["numpad5"] = sf::Keyboard::Numpad5;
			m_KeyMap["numpad6"] = sf::Keyboard::Numpad6;
			m_KeyMap["numpad7"] = sf::Keyboard::Numpad7;
			m_KeyMap["numpad8"] = sf::Keyboard::Numpad8;
			m_KeyMap["numpad9"] = sf::Keyboard::Numpad9;
			m_KeyMap["o"] = sf::Keyboard::O;

			m_KeyMap["p"] = sf::Keyboard::P;
			m_KeyMap["pagedown"] = sf::Keyboard::PageDown;
			m_KeyMap["page down"] = sf::Keyboard::PageDown;
			m_KeyMap["pdown"] = sf::Keyboard::PageDown;
			m_KeyMap["p down"] = sf::Keyboard::PageDown;

			m_KeyMap["pageup"] = sf::Keyboard::PageUp;
			m_KeyMap["page up"] = sf::Keyboard::PageUp;
			m_KeyMap["pup"] = sf::Keyboard::PageUp;
			m_KeyMap["p up"] = sf::Keyboard::PageUp;

			m_KeyMap["pause"] = sf::Keyboard::Pause;
			m_KeyMap["period"] = sf::Keyboard::Period;
			m_KeyMap["."] = sf::Keyboard::Period;

			m_KeyMap["q"] = sf::Keyboard::Q;
			m_KeyMap["quote"] = sf::Keyboard::Quote;
			m_KeyMap["'"] = sf::Keyboard::Quote;
			m_KeyMap["\""] = sf::Keyboard::Quote;

			m_KeyMap["r"] = sf::Keyboard::R;
			m_KeyMap["ralt"] = sf::Keyboard::RAlt;
			m_KeyMap["r alt"] = sf::Keyboard::RAlt;
			m_KeyMap["rightalt"] = sf::Keyboard::RAlt;
			m_KeyMap["right alt"] = sf::Keyboard::RAlt;

			m_KeyMap["rbracket"] = sf::Keyboard::RBracket;
			m_KeyMap["r bracket"] = sf::Keyboard::RBracket;
			m_KeyMap["rightbracket"] = sf::Keyboard::RBracket;
			m_KeyMap["right bracket"] = sf::Keyboard::RBracket;
			m_KeyMap["]"] = sf::Keyboard::RBracket;

			m_KeyMap["rcontrol"] = sf::Keyboard::RControl;
			m_KeyMap["r control"] = sf::Keyboard::RControl;
			m_KeyMap["rightcontrol"] = sf::Keyboard::RControl;
			m_KeyMap["right control"] = sf::Keyboard::RControl;

			m_KeyMap["return"] = sf::Keyboard::Return;
			m_KeyMap["enter"] = sf::Keyboard::Return;

			m_KeyMap["right"] = sf::Keyboard::Right;
			m_KeyMap["rarrow"] = sf::Keyboard::Right;
			m_KeyMap["r arrow"] = sf::Keyboard::Right;
			m_KeyMap["rightarrow"] = sf::Keyboard::Right;
			m_KeyMap["right arrow"] = sf::Keyboard::Right;

			m_KeyMap["rshift"] = sf::Keyboard::RShift;
			m_KeyMap["r shift"] = sf::Keyboard::RShift;
			m_KeyMap["rightshift"] = sf::Keyboard::RShift;
			m_KeyMap["right shift"] = sf::Keyboard::RShift;

			m_KeyMap["rsystem"] = sf::Keyboard::RSystem;
			m_KeyMap["r system"] = sf::Keyboard::RSystem;
			m_KeyMap["rightsystem"] = sf::Keyboard::RSystem;
			m_KeyMap["right system"] = sf::Keyboard::RSystem;

			m_KeyMap["s"] = sf::Keyboard::S;
			m_KeyMap["semicolon"] = sf::Keyboard::SemiColon;
			m_KeyMap["semi-colon"] = sf::Keyboard::SemiColon;
			m_KeyMap["scolon"] = sf::Keyboard::SemiColon;
			m_KeyMap["semi colon"] = sf::Keyboard::SemiColon;
			m_KeyMap["s colon"] = sf::Keyboard::SemiColon;

			m_KeyMap["slash"] = sf::Keyboard::Slash;
			m_KeyMap["/"] = sf::Keyboard::Slash;
			m_KeyMap[" "] = sf::Keyboard::Space;
			m_KeyMap["space"] = sf::Keyboard::Space;
			m_KeyMap["subtract"] = sf::Keyboard::Subtract;
			m_KeyMap["-"] = sf::Keyboard::Subtract;
			m_KeyMap["minus"] = sf::Keyboard::Subtract;

			m_KeyMap["t"] = sf::Keyboard::T;
			m_KeyMap["tab"] = sf::Keyboard::Tab;
			m_KeyMap["tilde"] = sf::Keyboard::Tilde;
			m_KeyMap["`"] = sf::Keyboard::Tilde;

			m_KeyMap["u"] = sf::Keyboard::U;
			m_KeyMap["unknown"] = sf::Keyboard::Unknown;

			m_KeyMap["up"] = sf::Keyboard::Up;
			m_KeyMap["uarrow"] = sf::Keyboard::Up;
			m_KeyMap["u arrow"] = sf::Keyboard::Up;
			m_KeyMap["uparrow"] = sf::Keyboard::Up;
			m_KeyMap["up arrow"] = sf::Keyboard::Up;

			m_KeyMap["v"] = sf::Keyboard::V;
			m_KeyMap["w"] = sf::Keyboard::W;
			m_KeyMap["x"] = sf::Keyboard::X;
			m_KeyMap["y"] = sf::Keyboard::Y;
			m_KeyMap["z"] = sf::Keyboard::Z;
		}
		void _initMouseMap(){
			if(m_MouseMap.size() > 0) return;
			m_MouseMap["l"] = sf::Mouse::Button::Left;
			m_MouseMap["left"] = sf::Mouse::Button::Left;
			m_MouseMap["left button"] = sf::Mouse::Button::Left;
			m_MouseMap["leftbutton"] = sf::Mouse::Button::Left;

			m_MouseMap["r"] = sf::Mouse::Button::Right;
			m_MouseMap["right"] = sf::Mouse::Button::Right;
			m_MouseMap["right button"] = sf::Mouse::Button::Right;
			m_MouseMap["rightbutton"] = sf::Mouse::Button::Right;

			m_MouseMap["m"] = sf::Mouse::Button::Middle;
			m_MouseMap["middle"] = sf::Mouse::Button::Middle;
			m_MouseMap["middle button"] = sf::Mouse::Button::Middle;
			m_MouseMap["middlebutton"] = sf::Mouse::Button::Middle;

			m_MouseMap["none"] = uint(5);
		}
		void _destruct(){
			m_KeyStatus.clear();
			m_MouseStatus.clear();
			m_KeyMap.clear();
			m_MouseMap.clear();
		}
		bool _isMouseButtonDown(string& str){
			boost::algorithm::to_lower(str);
			if(m_MouseStatus[m_MouseMap.at(str)]) return true; return false;
		}
		bool _isMouseButtonDownOnce(string& str){
			bool res = _isMouseButtonDown(str);
			if(res && m_currentButton == m_MouseMap.at(str) && (m_currentButton != m_previousButton)) return true; return false;
		}
		void _setMousePositionInternal(float x,float y,bool& resetDifference,bool& resetPrevious){
			glm::vec2 newPos = glm::vec2(x,y);
			if(resetPrevious == true) m_Position_Previous = newPos;
			else                      m_Position_Previous = m_Position;
			m_Position = newPos;
			m_Difference.x += (m_Position.x - m_Position_Previous.x);
			m_Difference.y += (m_Position.y - m_Position_Previous.y);
			if(resetDifference == true) m_Difference = glm::vec2(0.0f);
		}
		bool _isKeyDown(string& str){
			boost::algorithm::to_lower(str);
			if(m_KeyStatus[m_KeyMap.at(str)]) return true; return false;
		}
		bool _isKeyUp(string& str){
			boost::algorithm::to_lower(str);
			if(!m_KeyStatus[m_KeyMap.at(str)]) return true; return false;
		}
		bool _isKeyDownOnce(string& str){
			bool res = _isKeyDown(str);
			if(res && m_currentKey == m_KeyMap.at(str) && (m_currentKey != m_previousKey)) return true; return false;
		}
		void _onEventKeyPressed(uint& key){
			m_previousKey = m_currentKey;
			m_currentKey = key;
			m_KeyStatus[key] = true;
		}
		void _onEventKeyReleased(uint& key){
			m_previousKey = sf::Keyboard::Unknown;
			m_currentKey = sf::Keyboard::Unknown;
			m_KeyStatus[key] = false;
		}
		void _onEventMouseWheelMoved(int& delta){
            m_Delta += (delta * 10);
		}
		void _onEventMouseButtonPressed(uint& mouseButton){
			m_previousButton = m_currentButton;
			m_currentButton = mouseButton;
			m_MouseStatus[mouseButton] = true;
		}
		void _onEventMouseButtonReleased(uint& mouseButton){
			m_previousButton = 100; //we will use 100 as the "none" key
			m_currentButton = 100;  //we will use 100 as the "none" key
			m_MouseStatus[mouseButton] = false;
		}
		void _onResetEvents(){
			m_previousKey = sf::Keyboard::Unknown;
			m_currentKey = sf::Keyboard::Unknown;
			for(auto iterator:m_KeyStatus){ iterator.second = false; }
			for(auto iterator:m_MouseStatus){ iterator.second = false; }
			m_Delta *= 0.97f * (1.0f-Resources::dt());
		}
		void _update(float& dt){
			m_Difference *= (0.975f);
		}
};
epriv::EventManager::EventManager(const char* name,uint w,uint h):m_i(new impl){
	m_i->_init(name,w,h);
}
epriv::EventManager::~EventManager(){
	m_i->_destruct();
}
void epriv::EventManager::_init(const char* name,uint w,uint h){
	m_i->_postInit(name,w,h);
}
void epriv::EventManager::_onEventKeyPressed(uint& key){
	m_i->_onEventKeyPressed(key);
}
void epriv::EventManager::_onEventKeyReleased(uint& key){
	m_i->_onEventKeyReleased(key);
}
void epriv::EventManager::_onEventMouseButtonPressed(uint mouseButton){
	m_i->_onEventMouseButtonPressed(mouseButton);
}
void epriv::EventManager::_onEventMouseButtonReleased(uint mouseButton){
	m_i->_onEventMouseButtonReleased(mouseButton);
}
void epriv::EventManager::_onEventMouseWheelMoved(int& delta){
	m_i->_onEventMouseWheelMoved(delta);
}
void epriv::EventManager::_onResetEvents(){
	m_i->_onResetEvents();
}
void epriv::EventManager::_update(float dt){
	m_i->_update(dt);
}
void epriv::EventManager::_setMousePosition(float x,float y,bool resetDifference,bool resetPreviousPosition){
	m_i->_setMousePositionInternal(x,y,resetDifference,resetPreviousPosition);
}
bool Engine::isKeyDown(string str){
	return epriv::Core::m_Engine->m_EventManager->m_i->_isKeyDown(str);
}
bool Engine::isKeyDownOnce(string str){
	return epriv::Core::m_Engine->m_EventManager->m_i->_isKeyDownOnce(str);
}
bool Engine::isKeyUp(string str){
	return epriv::Core::m_Engine->m_EventManager->m_i->_isKeyUp(str);
}
bool Engine::isMouseButtonDown(string str){
	return epriv::Core::m_Engine->m_EventManager->m_i->_isMouseButtonDown(str);
}
bool Engine::isMouseButtonDownOnce(string str){
	return epriv::Core::m_Engine->m_EventManager->m_i->_isMouseButtonDownOnce(str);
}
const glm::vec2& Engine::getMouseDifference(){
	return epriv::Core::m_Engine->m_EventManager->m_i->m_Difference;
}
const glm::vec2& Engine::getMousePositionPrevious(){
	return epriv::Core::m_Engine->m_EventManager->m_i->m_Position_Previous;
}
const glm::vec2& Engine::getMousePosition(){
	return epriv::Core::m_Engine->m_EventManager->m_i->m_Position;
}
const float Engine::getMouseWheelDelta(){
	return epriv::Core::m_Engine->m_EventManager->m_i->m_Delta;
}
void Engine::setMousePosition(float x,float y,bool resetDifference,bool resetPreviousPosition){
	sf::Mouse::setPosition(sf::Vector2i(int(x),int(y)),*Resources::getWindow()->getSFMLHandle());
	epriv::Core::m_Engine->m_EventManager->_setMousePosition(x,y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(uint x,uint y,bool resetDifference,bool resetPreviousPosition){
	sf::Mouse::setPosition(sf::Vector2i(x,y),*Resources::getWindow()->getSFMLHandle());
	epriv::Core::m_Engine->m_EventManager->_setMousePosition((float)x,(float)y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(glm::vec2 pos,bool resetDifference,bool resetPreviousPosition){
	sf::Mouse::setPosition(sf::Vector2i(int(pos.x),int(pos.y)),*Resources::getWindow()->getSFMLHandle());
	epriv::Core::m_Engine->m_EventManager->_setMousePosition(pos.x,pos.y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(glm::uvec2 pos,bool resetDifference,bool resetPreviousPosition){
	sf::Mouse::setPosition(sf::Vector2i(pos.x,pos.y),*Resources::getWindow()->getSFMLHandle());
	epriv::Core::m_Engine->m_EventManager->_setMousePosition((float)pos.x,(float)pos.y,resetDifference,resetPreviousPosition);
}
