#include "Engine_Events.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include <boost/algorithm/string.hpp> 

using namespace Engine::Events;

//return string dictionaries of keys / mouse buttons
std::unordered_map<std::string,uint> _getkeymap(){
    std::unordered_map<std::string,uint> k;
    k["a"] = sf::Keyboard::A;
    k["add"] = sf::Keyboard::Add;
    k["b"] = sf::Keyboard::B;
    k["backslash"] = sf::Keyboard::BackSlash;
    k["back slash"] = sf::Keyboard::BackSlash;
    k["bslash"] = sf::Keyboard::BackSlash;
    k["b slash"] = sf::Keyboard::BackSlash;
    k["\\"] = sf::Keyboard::BackSlash;
    k["backspace"] = sf::Keyboard::BackSpace;
    k["back space"] = sf::Keyboard::BackSpace;
    k["bspace"] = sf::Keyboard::BackSpace;
    k["b space"] = sf::Keyboard::BackSpace;
    k["c"] = sf::Keyboard::C;
    k["comma"] = sf::Keyboard::Comma;
    k[","] = sf::Keyboard::Comma;
    k["d"] = sf::Keyboard::D;
    k["dash"] = sf::Keyboard::Dash;
    k["-"] = sf::Keyboard::Dash;
    k["delete"] = sf::Keyboard::Delete;
    k["del"] = sf::Keyboard::Delete;
    k["divide"] = sf::Keyboard::Divide;
    k["down"] = sf::Keyboard::Down;
    k["down arrow"] = sf::Keyboard::Down;
    k["downarrow"] = sf::Keyboard::Down;
    k["darrow"] = sf::Keyboard::Down;
    k["d arrow"] = sf::Keyboard::Down;
    k["e"] = sf::Keyboard::E;
    k["end"] = sf::Keyboard::End;
    k["equal"] = sf::Keyboard::Equal;
    k["="] = sf::Keyboard::Equal;
    k["esc"] = sf::Keyboard::Escape;
    k["escape"] = sf::Keyboard::Escape;
    k["f"] = sf::Keyboard::F;

    k["f1"] = sf::Keyboard::F1;
    k["f2"] = sf::Keyboard::F2;
    k["f3"] = sf::Keyboard::F3;
    k["f4"] = sf::Keyboard::F4;
    k["f5"] = sf::Keyboard::F5;
    k["f6"] = sf::Keyboard::F6;
    k["f7"] = sf::Keyboard::F7;
    k["f8"] = sf::Keyboard::F8;
    k["f9"] = sf::Keyboard::F9;
    k["f10"] = sf::Keyboard::F10;
    k["f11"] = sf::Keyboard::F11;
    k["f12"] = sf::Keyboard::F12;

    k["g"] = sf::Keyboard::G;
    k["h"] = sf::Keyboard::H;
    k["home"] = sf::Keyboard::Home;
    k["i"] = sf::Keyboard::I;
    k["insert"] = sf::Keyboard::Insert;
    k["j"] = sf::Keyboard::J;
    k["k"] = sf::Keyboard::K;
    k["l"] = sf::Keyboard::L;

    k["lalt"] = sf::Keyboard::LAlt;
    k["leftalt"] = sf::Keyboard::LAlt;
    k["left alt"] = sf::Keyboard::LAlt;
    k["l alt"] = sf::Keyboard::LAlt;

    k["lbracket"] = sf::Keyboard::LBracket;
    k["l bracket"] = sf::Keyboard::LBracket;
    k["leftbracket"] = sf::Keyboard::LBracket;
    k["left bracket"] = sf::Keyboard::LBracket;
    k["["] = sf::Keyboard::LBracket;

    k["lcontrol"] = sf::Keyboard::LControl;
    k["l control"] = sf::Keyboard::LControl;
    k["leftcontrol"] = sf::Keyboard::LControl;
    k["left control"] = sf::Keyboard::LControl;

    k["left"] = sf::Keyboard::Left;
    k["larrow"] = sf::Keyboard::Left;
    k["l arrow"] = sf::Keyboard::Left;
    k["leftarrow"] = sf::Keyboard::Left;
    k["left arrow"] = sf::Keyboard::Left;

    k["lshift"] = sf::Keyboard::LShift;
    k["l shift"] = sf::Keyboard::LShift;
    k["leftshift"] = sf::Keyboard::LShift;
    k["left shift"] = sf::Keyboard::LShift;

    k["lsystem"] = sf::Keyboard::LSystem;
    k["l system"] = sf::Keyboard::LSystem;
    k["leftsystem"] = sf::Keyboard::LSystem;
    k["left system"] = sf::Keyboard::LSystem;

    k["m"] = sf::Keyboard::M;
    k["menu"] = sf::Keyboard::Menu;
    k["multiply"] = sf::Keyboard::Multiply;

    k["n"] = sf::Keyboard::N;
    k["num0"] = sf::Keyboard::Num0;
    k["num1"] = sf::Keyboard::Num1;
    k["num2"] = sf::Keyboard::Num2;
    k["num3"] = sf::Keyboard::Num3;
    k["num4"] = sf::Keyboard::Num4;
    k["num5"] = sf::Keyboard::Num5;
    k["num6"] = sf::Keyboard::Num6;
    k["num7"] = sf::Keyboard::Num7;
    k["num8"] = sf::Keyboard::Num8;
    k["num9"] = sf::Keyboard::Num9;
    k["0"] = sf::Keyboard::Num0;
    k["1"] = sf::Keyboard::Num1;
    k["2"] = sf::Keyboard::Num2;
    k["3"] = sf::Keyboard::Num3;
    k["4"] = sf::Keyboard::Num4;
    k["5"] = sf::Keyboard::Num5;
    k["6"] = sf::Keyboard::Num6;
    k["7"] = sf::Keyboard::Num7;
    k["8"] = sf::Keyboard::Num8;
    k["9"] = sf::Keyboard::Num9;
    k["numpad0"] = sf::Keyboard::Numpad0;
    k["numpad1"] = sf::Keyboard::Numpad1;
    k["numpad2"] = sf::Keyboard::Numpad2;
    k["numpad3"] = sf::Keyboard::Numpad3;
    k["numpad4"] = sf::Keyboard::Numpad4;
    k["numpad5"] = sf::Keyboard::Numpad5;
    k["numpad6"] = sf::Keyboard::Numpad6;
    k["numpad7"] = sf::Keyboard::Numpad7;
    k["numpad8"] = sf::Keyboard::Numpad8;
    k["numpad9"] = sf::Keyboard::Numpad9;
    k["o"] = sf::Keyboard::O;

    k["p"] = sf::Keyboard::P;
    k["pagedown"] = sf::Keyboard::PageDown;
    k["page down"] = sf::Keyboard::PageDown;
    k["pdown"] = sf::Keyboard::PageDown;
    k["p down"] = sf::Keyboard::PageDown;

    k["pageup"] = sf::Keyboard::PageUp;
    k["page up"] = sf::Keyboard::PageUp;
    k["pup"] = sf::Keyboard::PageUp;
    k["p up"] = sf::Keyboard::PageUp;

    k["pause"] = sf::Keyboard::Pause;
    k["period"] = sf::Keyboard::Period;
    k["."] = sf::Keyboard::Period;

    k["q"] = sf::Keyboard::Q;
    k["quote"] = sf::Keyboard::Quote;
    k["'"] = sf::Keyboard::Quote;
    k["\""] = sf::Keyboard::Quote;

    k["r"] = sf::Keyboard::R;
    k["ralt"] = sf::Keyboard::RAlt;
    k["r alt"] = sf::Keyboard::RAlt;
    k["rightalt"] = sf::Keyboard::RAlt;
    k["right alt"] = sf::Keyboard::RAlt;

    k["rbracket"] = sf::Keyboard::RBracket;
    k["r bracket"] = sf::Keyboard::RBracket;
    k["rightbracket"] = sf::Keyboard::RBracket;
    k["right bracket"] = sf::Keyboard::RBracket;
    k["]"] = sf::Keyboard::RBracket;

    k["rcontrol"] = sf::Keyboard::RControl;
    k["r control"] = sf::Keyboard::RControl;
    k["rightcontrol"] = sf::Keyboard::RControl;
    k["right control"] = sf::Keyboard::RControl;

    k["return"] = sf::Keyboard::Return;
    k["enter"] = sf::Keyboard::Return;

    k["right"] = sf::Keyboard::Right;
    k["rarrow"] = sf::Keyboard::Right;
    k["r arrow"] = sf::Keyboard::Right;
    k["rightarrow"] = sf::Keyboard::Right;
    k["right arrow"] = sf::Keyboard::Right;

    k["rshift"] = sf::Keyboard::RShift;
    k["r shift"] = sf::Keyboard::RShift;
    k["rightshift"] = sf::Keyboard::RShift;
    k["right shift"] = sf::Keyboard::RShift;

    k["rsystem"] = sf::Keyboard::RSystem;
    k["r system"] = sf::Keyboard::RSystem;
    k["rightsystem"] = sf::Keyboard::RSystem;
    k["right system"] = sf::Keyboard::RSystem;

    k["s"] = sf::Keyboard::S;
    k["semicolon"] = sf::Keyboard::SemiColon;
    k["semi-colon"] = sf::Keyboard::SemiColon;
    k["scolon"] = sf::Keyboard::SemiColon;
    k["semi colon"] = sf::Keyboard::SemiColon;
    k["s colon"] = sf::Keyboard::SemiColon;

    k["slash"] = sf::Keyboard::Slash;
    k["/"] = sf::Keyboard::Slash;
    k[" "] = sf::Keyboard::Space;
    k["space"] = sf::Keyboard::Space;
    k["subtract"] = sf::Keyboard::Subtract;
    k["-"] = sf::Keyboard::Subtract;
    k["minus"] = sf::Keyboard::Subtract;

    k["t"] = sf::Keyboard::T;
    k["tab"] = sf::Keyboard::Tab;
    k["tilde"] = sf::Keyboard::Tilde;
    k["`"] = sf::Keyboard::Tilde;

    k["u"] = sf::Keyboard::U;
    k["unknown"] = sf::Keyboard::Unknown;

    k["up"] = sf::Keyboard::Up;
    k["uarrow"] = sf::Keyboard::Up;
    k["u arrow"] = sf::Keyboard::Up;
    k["uparrow"] = sf::Keyboard::Up;
    k["up arrow"] = sf::Keyboard::Up;

    k["v"] = sf::Keyboard::V;
    k["w"] = sf::Keyboard::W;
    k["x"] = sf::Keyboard::X;
    k["y"] = sf::Keyboard::Y;
    k["z"] = sf::Keyboard::Z;
    return k;
}
std::unordered_map<std::string,uint> _getmousemap(){
    std::unordered_map<std::string,uint> m;

    m["l"] = sf::Mouse::Button::Left;
    m["left"] = sf::Mouse::Button::Left;
    m["left button"] = sf::Mouse::Button::Left;
    m["leftbutton"] = sf::Mouse::Button::Left;

    m["r"] = sf::Mouse::Button::Right;
    m["right"] = sf::Mouse::Button::Right;
    m["right button"] = sf::Mouse::Button::Right;
    m["rightbutton"] = sf::Mouse::Button::Right;

    m["m"] = sf::Mouse::Button::Middle;
    m["middle"] = sf::Mouse::Button::Middle;
    m["middle button"] = sf::Mouse::Button::Middle;
    m["middlebutton"] = sf::Mouse::Button::Middle;

    m["none"] = static_cast<uint>(5);
    return m;
}

//init static values with actual objects

std::unordered_map<std::string,uint> Keyboard::KeyProcessing::m_KeyMap = _getkeymap();
std::unordered_map<std::string,uint> Mouse::MouseProcessing::m_MouseMap = _getmousemap();
std::unordered_map<uint,bool> Keyboard::KeyProcessing::m_KeyStatus;
std::unordered_map<uint,bool> Mouse::MouseProcessing::m_MouseStatus;
float Mouse::MouseProcessing::m_Delta = 0;
glm::vec2 Mouse::MouseProcessing::m_Position = glm::vec2(0);
glm::vec2 Mouse::MouseProcessing::m_Position_Previous = glm::vec2(0);
glm::vec2 Mouse::MouseProcessing::m_Difference = glm::vec2(0);

//init prev / current variables
uint Keyboard::KeyProcessing::m_currentKey = sf::Keyboard::Unknown;
uint Keyboard::KeyProcessing::m_previousKey = sf::Keyboard::Unknown;
uint Mouse::MouseProcessing::m_currentButton = 100;  //we will use 100 as the "none" key
uint Mouse::MouseProcessing::m_previousButton = 100; //we will use 100 as the "none" key

bool Mouse::MouseProcessing::_IsMouseButtonDown(std::string str){
    boost::algorithm::to_lower(str);
    uint key = MouseProcessing::m_MouseMap[str];
    if(MouseProcessing::m_MouseStatus[key]) return true; return false;
}
bool Mouse::MouseProcessing::_IsMouseButtonDownOnce(std::string str){
    bool res = MouseProcessing::_IsMouseButtonDown(str);
    uint key = MouseProcessing::m_MouseMap[str];
    if(res && m_currentButton == key && (m_currentButton != m_previousButton)) return true; return false;
}
void Mouse::MouseProcessing::_SetMousePositionInternal(float x, float y){
    m_Position_Previous = m_Position;
    m_Position = glm::vec2(x,y);
    m_Difference.x += (m_Position.x - m_Position_Previous.x);
    m_Difference.y += (m_Position.y - m_Position_Previous.y);
}
bool Keyboard::KeyProcessing::_IsKeyDown(std::string str){
    boost::algorithm::to_lower(str);
    uint key = KeyProcessing::m_KeyMap[str];
    if(KeyProcessing::m_KeyStatus[key]) return true; return false;
}
bool Keyboard::KeyProcessing::_IsKeyUp(std::string str){
    boost::algorithm::to_lower(str);
    uint key = KeyProcessing::m_KeyMap[str];
    if(!KeyProcessing::m_KeyStatus[key]) return true; return false;
}
bool Keyboard::KeyProcessing::_IsKeyDownOnce(std::string str){
    bool res = KeyProcessing::_IsKeyDown(str);
    uint key = KeyProcessing::m_KeyMap[str];
    if(res && m_currentKey == key && (m_currentKey != m_previousKey)) return true; return false;
}
const glm::vec2& Mouse::getMouseDifference(){ return MouseProcessing::m_Difference; }
const glm::vec2& Mouse::getMousePositionPrevious(){ return MouseProcessing::m_Position_Previous; }
const glm::vec2& Mouse::getMousePosition(){ return MouseProcessing::m_Position; }
const float Mouse::getMouseWheelDelta(){ return MouseProcessing::m_Delta; }
bool Mouse::isMouseButtonDown(std::string str){ return MouseProcessing::_IsMouseButtonDown(str); }
bool Mouse::isMouseButtonDownOnce(std::string str){ return MouseProcessing::_IsMouseButtonDownOnce(str); }

void Mouse::setMousePosition(float x,float y){ 
    sf::Mouse::setPosition(sf::Vector2i(int(x),int(y)),*Resources::getWindow()->getSFMLHandle());
    Mouse::MouseProcessing::_SetMousePositionInternal(x,y); 
}
void Mouse::setMousePosition(glm::vec2 pos){ 
    sf::Mouse::setPosition(sf::Vector2i(int(pos.x),int(pos.y)),*Resources::getWindow()->getSFMLHandle());
    Mouse::MouseProcessing::_SetMousePositionInternal(pos.x,pos.y); 
}
void Mouse::setMousePosition(glm::uvec2 pos){ 
    sf::Mouse::setPosition(sf::Vector2i(pos.x,pos.y),*Resources::getWindow()->getSFMLHandle());
    Mouse::MouseProcessing::_SetMousePositionInternal(float(pos.x),float(pos.y)); 
}

bool Keyboard::isKeyDown(std::string str){ return KeyProcessing::_IsKeyDown(str); }
bool Keyboard::isKeyDownOnce(std::string str){ return KeyProcessing::_IsKeyDownOnce(str); }
bool Keyboard::isKeyUp(std::string str){ return KeyProcessing::_IsKeyUp(str); }

bool Engine::Events::isKeyDown(std::string str){ return Keyboard::KeyProcessing::_IsKeyDown(str); }
bool Engine::Events::isKeyDownOnce(std::string str){ return Keyboard::KeyProcessing::_IsKeyDownOnce(str); }
bool Engine::Events::isKeyUp(std::string str){ return Keyboard::KeyProcessing::_IsKeyUp(str); }

const glm::vec2& Engine::Events::getMousePosition(){ return Mouse::MouseProcessing::m_Position; }