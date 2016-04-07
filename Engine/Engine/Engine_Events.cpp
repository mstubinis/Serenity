#include "Engine_Events.h"

#ifdef _WIN32
#include <Windows.h>
#endif

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
std::unordered_map<uint,uint> _getWindowskeymap(){
	std::unordered_map<uint,uint> k;

	k['A'] = sf::Keyboard::A;
    k[VK_ADD] = sf::Keyboard::Add;
    k['B'] = sf::Keyboard::B;
    k[VK_OEM_5] = sf::Keyboard::BackSlash;
    k[VK_BACK] = sf::Keyboard::BackSpace;
    k['C'] = sf::Keyboard::C;
    k[VK_OEM_COMMA] = sf::Keyboard::Comma;
    k['D'] = sf::Keyboard::D;
    k[VK_OEM_MINUS] = sf::Keyboard::Dash;
    k[VK_DELETE] = sf::Keyboard::Delete;
    k[VK_DIVIDE] = sf::Keyboard::Divide;
    k[VK_DOWN] = sf::Keyboard::Down;
    k['E'] = sf::Keyboard::E;
    k[VK_END] = sf::Keyboard::End;
    k[VK_OEM_PLUS] = sf::Keyboard::Equal;
    k[VK_ESCAPE] = sf::Keyboard::Escape;
    k['F'] = sf::Keyboard::F;
    k[VK_F1] = sf::Keyboard::F1;
    k[VK_F2] = sf::Keyboard::F2;
    k[VK_F3] = sf::Keyboard::F3;
    k[VK_F4] = sf::Keyboard::F4;
    k[VK_F5] = sf::Keyboard::F5;
    k[VK_F6] = sf::Keyboard::F6;
    k[VK_F7] = sf::Keyboard::F7;
    k[VK_F8] = sf::Keyboard::F8;
    k[VK_F9] = sf::Keyboard::F9;
    k[VK_F10] = sf::Keyboard::F10;
    k[VK_F11] = sf::Keyboard::F11;
    k[VK_F12] = sf::Keyboard::F12;
    k['G'] = sf::Keyboard::G;
    k['H'] = sf::Keyboard::H;
    k[VK_HOME] = sf::Keyboard::Home;
    k['I'] = sf::Keyboard::I;
    k[VK_INSERT] = sf::Keyboard::Insert;
    k['J'] = sf::Keyboard::J;
    k['K'] = sf::Keyboard::K;
    k['L'] = sf::Keyboard::L;
    k[VK_LMENU] = sf::Keyboard::LAlt;
    k[VK_OEM_4] = sf::Keyboard::LBracket;
    k[VK_LCONTROL] = sf::Keyboard::LControl;
    k[VK_LEFT] = sf::Keyboard::Left;
    k[VK_LSHIFT] = sf::Keyboard::LShift;
    k[VK_LWIN] = sf::Keyboard::LSystem;
    k['M'] = sf::Keyboard::M;
    k[VK_APPS] = sf::Keyboard::Menu;
    k[VK_MULTIPLY] = sf::Keyboard::Multiply;
    k['N'] = sf::Keyboard::N;
    k['0'] = sf::Keyboard::Num0;
    k['1'] = sf::Keyboard::Num1;
    k['2'] = sf::Keyboard::Num2;
    k['3'] = sf::Keyboard::Num3;
    k['4'] = sf::Keyboard::Num4;
    k['5'] = sf::Keyboard::Num5;
    k['6'] = sf::Keyboard::Num6;
    k['7'] = sf::Keyboard::Num7;
    k['8'] = sf::Keyboard::Num8;
    k['9'] = sf::Keyboard::Num9;
    k[VK_NUMPAD0] = sf::Keyboard::Numpad0;
    k[VK_NUMPAD1] = sf::Keyboard::Numpad1;
    k[VK_NUMPAD2] = sf::Keyboard::Numpad2;
    k[VK_NUMPAD3] = sf::Keyboard::Numpad3;
    k[VK_NUMPAD4] = sf::Keyboard::Numpad4;
    k[VK_NUMPAD5] = sf::Keyboard::Numpad5;
    k[VK_NUMPAD6] = sf::Keyboard::Numpad6;
    k[VK_NUMPAD7] = sf::Keyboard::Numpad7;
    k[VK_NUMPAD8] = sf::Keyboard::Numpad8;
    k[VK_NUMPAD9] = sf::Keyboard::Numpad9;
    k['O'] = sf::Keyboard::O;
    k['P'] = sf::Keyboard::P;
    k[VK_NEXT] = sf::Keyboard::PageDown;
    k[VK_PRIOR] = sf::Keyboard::PageUp;
    k[VK_PAUSE] = sf::Keyboard::Pause;
    k[VK_OEM_PERIOD] = sf::Keyboard::Period;
    k['Q'] = sf::Keyboard::Q;
    k[VK_OEM_7] = sf::Keyboard::Quote;
    k['R'] = sf::Keyboard::R;
    k[VK_RMENU] = sf::Keyboard::RAlt;
    k[VK_OEM_6] = sf::Keyboard::RBracket;
    k[VK_RCONTROL] = sf::Keyboard::RControl;
    k[VK_RETURN] = sf::Keyboard::Return;
    k[VK_RIGHT] = sf::Keyboard::Right;
    k[VK_RSHIFT] = sf::Keyboard::RShift;
    k[VK_RWIN] = sf::Keyboard::RSystem;
    k['S'] = sf::Keyboard::S;
    k[VK_OEM_1] = sf::Keyboard::SemiColon;
    k[VK_OEM_2] = sf::Keyboard::Slash;
    k[VK_SPACE] = sf::Keyboard::Space;
    k[VK_SUBTRACT] = sf::Keyboard::Subtract;
    k['T'] = sf::Keyboard::T;
    k[VK_TAB] = sf::Keyboard::Tab;
    k[VK_OEM_3] = sf::Keyboard::Tilde;
    k['U'] = sf::Keyboard::U;
    //k[999] = sf::Keyboard::Unknown;
    k[VK_UP] = sf::Keyboard::Up;
    k['V'] = sf::Keyboard::V;
    k['W'] = sf::Keyboard::W;
    k['X'] = sf::Keyboard::X;
    k['Y'] = sf::Keyboard::Y;
    k['Z'] = sf::Keyboard::Z;

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
std::unordered_map<uint,uint> Keyboard::KeyProcessing::m_WindowsKeyMap = _getWindowskeymap();
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