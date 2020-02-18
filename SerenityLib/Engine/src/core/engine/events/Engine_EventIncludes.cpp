#include <core/engine/events/Engine_EventIncludes.h>

#include <unordered_map>

using namespace std;

std::vector<std::string> KEYBOARD_KEY_NAMES;
std::vector<std::string> MOUSE_BUTTON_NAMES;
std::vector<std::string> MOUSE_WHEEL_NAMES;
std::vector<std::string> JOYSTICK_CONTROL_NAMES;

std::string              EMPTY_STRING = "";

std::vector<std::string> INIT_KEYBOARD_KEY_NAMES() {
    std::vector<std::string> keyboard_key_names_;
    keyboard_key_names_.resize(KeyboardKey::_TOTAL, EMPTY_STRING);

    keyboard_key_names_[KeyboardKey::A] = "A";
    keyboard_key_names_[KeyboardKey::B] = "B";
    keyboard_key_names_[KeyboardKey::C] = "C";
    keyboard_key_names_[KeyboardKey::D] = "D";
    keyboard_key_names_[KeyboardKey::E] = "E";
    keyboard_key_names_[KeyboardKey::F] = "F";
    keyboard_key_names_[KeyboardKey::G] = "G";
    keyboard_key_names_[KeyboardKey::H] = "H";
    keyboard_key_names_[KeyboardKey::I] = "I";
    keyboard_key_names_[KeyboardKey::J] = "J";
    keyboard_key_names_[KeyboardKey::K] = "K";
    keyboard_key_names_[KeyboardKey::L] = "L";
    keyboard_key_names_[KeyboardKey::M] = "M";
    keyboard_key_names_[KeyboardKey::N] = "N";
    keyboard_key_names_[KeyboardKey::O] = "O";
    keyboard_key_names_[KeyboardKey::P] = "P";
    keyboard_key_names_[KeyboardKey::Q] = "Q";
    keyboard_key_names_[KeyboardKey::R] = "R";
    keyboard_key_names_[KeyboardKey::S] = "S";
    keyboard_key_names_[KeyboardKey::T] = "T";
    keyboard_key_names_[KeyboardKey::U] = "U";
    keyboard_key_names_[KeyboardKey::V] = "V";
    keyboard_key_names_[KeyboardKey::W] = "W";
    keyboard_key_names_[KeyboardKey::X] = "X";
    keyboard_key_names_[KeyboardKey::Y] = "Y";
    keyboard_key_names_[KeyboardKey::Z] = "Z";


    keyboard_key_names_[KeyboardKey::Num0] = "0";
    keyboard_key_names_[KeyboardKey::Num1] = "1";
    keyboard_key_names_[KeyboardKey::Num2] = "2";
    keyboard_key_names_[KeyboardKey::Num3] = "3";
    keyboard_key_names_[KeyboardKey::Num4] = "4";
    keyboard_key_names_[KeyboardKey::Num5] = "5";
    keyboard_key_names_[KeyboardKey::Num6] = "6";
    keyboard_key_names_[KeyboardKey::Num7] = "7";
    keyboard_key_names_[KeyboardKey::Num8] = "8";
    keyboard_key_names_[KeyboardKey::Num9] = "9";
    keyboard_key_names_[KeyboardKey::Escape] = "Esc";
    keyboard_key_names_[KeyboardKey::LeftControl] = "Ctrl";
    keyboard_key_names_[KeyboardKey::LeftShift] = "Shift";
    keyboard_key_names_[KeyboardKey::LeftAlt] = "Alt";
    keyboard_key_names_[KeyboardKey::LeftSystem] = "System";
    keyboard_key_names_[KeyboardKey::RightControl] = "Ctrl";
    keyboard_key_names_[KeyboardKey::RightShift] = "Shift";
    keyboard_key_names_[KeyboardKey::RightAlt] = "Alt";
    keyboard_key_names_[KeyboardKey::RightSystem] = "System";
    keyboard_key_names_[KeyboardKey::Menu] = "Menu";
    keyboard_key_names_[KeyboardKey::LeftBracket] = "[";
    keyboard_key_names_[KeyboardKey::RightBracket] = "]";
    keyboard_key_names_[KeyboardKey::SemiColon] = ";";
    keyboard_key_names_[KeyboardKey::Comma] = ",";
    keyboard_key_names_[KeyboardKey::Period] = ".";
    keyboard_key_names_[KeyboardKey::Quote] = "'";
    keyboard_key_names_[KeyboardKey::Slash] = "/";
    keyboard_key_names_[KeyboardKey::BackSlash] = "\\";
    keyboard_key_names_[KeyboardKey::Tilde] = "~";
    keyboard_key_names_[KeyboardKey::Equal] = "=";
    keyboard_key_names_[KeyboardKey::Dash] = "-";
    keyboard_key_names_[KeyboardKey::Space] = "Spacebar";
    keyboard_key_names_[KeyboardKey::Return] = "Enter";
    keyboard_key_names_[KeyboardKey::BackSpace] = "Backspace";
    keyboard_key_names_[KeyboardKey::Tab] = "Tab";
    keyboard_key_names_[KeyboardKey::PageUp] = "Page Up";
    keyboard_key_names_[KeyboardKey::PageDown] = "Page Down";
    keyboard_key_names_[KeyboardKey::End] = "End";
    keyboard_key_names_[KeyboardKey::Home] = "Home";
    keyboard_key_names_[KeyboardKey::Insert] = "Insert";
    keyboard_key_names_[KeyboardKey::Delete] = "Delete";
    keyboard_key_names_[KeyboardKey::Add] = "+";
    keyboard_key_names_[KeyboardKey::Subtract] = "-";
    keyboard_key_names_[KeyboardKey::Multiply] = "*";
    keyboard_key_names_[KeyboardKey::Divide] = "/";
    keyboard_key_names_[KeyboardKey::LeftArrow] = "Left Arrow";
    keyboard_key_names_[KeyboardKey::RightArrow] = "Right Arrow";
    keyboard_key_names_[KeyboardKey::UpArrow] = "Up Arrow";
    keyboard_key_names_[KeyboardKey::DownArrow] = "Down Arrow";
    keyboard_key_names_[KeyboardKey::Numpad0] = "Numpad 0";
    keyboard_key_names_[KeyboardKey::Numpad1] = "Numpad 1";
    keyboard_key_names_[KeyboardKey::Numpad2] = "Numpad 2";
    keyboard_key_names_[KeyboardKey::Numpad3] = "Numpad 3";
    keyboard_key_names_[KeyboardKey::Numpad4] = "Numpad 4";
    keyboard_key_names_[KeyboardKey::Numpad5] = "Numpad 5";
    keyboard_key_names_[KeyboardKey::Numpad6] = "Numpad 6";
    keyboard_key_names_[KeyboardKey::Numpad7] = "Numpad 7";
    keyboard_key_names_[KeyboardKey::Numpad8] = "Numpad 8";
    keyboard_key_names_[KeyboardKey::Numpad9] = "Numpad 9";
    keyboard_key_names_[KeyboardKey::F1] = "F1";
    keyboard_key_names_[KeyboardKey::F2] = "F2";
    keyboard_key_names_[KeyboardKey::F3] = "F3";
    keyboard_key_names_[KeyboardKey::F4] = "F4";
    keyboard_key_names_[KeyboardKey::F5] = "F5";
    keyboard_key_names_[KeyboardKey::F6] = "F6";
    keyboard_key_names_[KeyboardKey::F7] = "F7";
    keyboard_key_names_[KeyboardKey::F8] = "F8";
    keyboard_key_names_[KeyboardKey::F9] = "F9";
    keyboard_key_names_[KeyboardKey::F10] = "F10";
    keyboard_key_names_[KeyboardKey::F11] = "F11";
    keyboard_key_names_[KeyboardKey::F12] = "F12";
    keyboard_key_names_[KeyboardKey::F13] = "F13";
    keyboard_key_names_[KeyboardKey::F14] = "F14";
    keyboard_key_names_[KeyboardKey::F15] = "F15";
    keyboard_key_names_[KeyboardKey::Pause] = "Pause";

    return keyboard_key_names_;
}

std::vector<std::string> INIT_MOUSE_BUTTON_NAMES(){
    std::vector<std::string> mouse_button_names_;
    mouse_button_names_.resize(MouseButton::_TOTAL, EMPTY_STRING);

    mouse_button_names_[MouseButton::Left]     = "Left Button";
    mouse_button_names_[MouseButton::Right]    = "Right Button";
    mouse_button_names_[MouseButton::Middle]   = "Middle Button";
    mouse_button_names_[MouseButton::XButton1] = "X Button 1";
    mouse_button_names_[MouseButton::XButton2] = "X Button 2";

    return mouse_button_names_;
}

std::vector<std::string> INIT_MOUSE_WHEEL_NAMES(){
    std::vector<std::string> mouse_wheel_names_;
    mouse_wheel_names_.resize(MouseWheel::_TOTAL, EMPTY_STRING);

    mouse_wheel_names_[MouseWheel::VerticalWheel]   = "Vertical";
    mouse_wheel_names_[MouseWheel::HorizontalWheel] = "Horizontal";

    return mouse_wheel_names_;
}

std::vector<std::string> INIT_JOYSTICK_CONTROL_NAMES(){
    std::vector<std::string> joystick_control_names_;
    joystick_control_names_.resize(JoystickAxis::_TOTAL, EMPTY_STRING);

    joystick_control_names_[JoystickAxis::X]    = "Joystick X";
    joystick_control_names_[JoystickAxis::Y]    = "Joystick Y";
    joystick_control_names_[JoystickAxis::Z]    = "Joystick Z";
    joystick_control_names_[JoystickAxis::R]    = "Joystick R";
    joystick_control_names_[JoystickAxis::U]    = "Joystick U";
    joystick_control_names_[JoystickAxis::V]    = "Joystick V";
    joystick_control_names_[JoystickAxis::PovX] = "Joystick Pov X";
    joystick_control_names_[JoystickAxis::PovY] = "Joystick Pov Y";

    return joystick_control_names_;
}

const std::string& KeyboardKey::toString(const KeyboardKey::Key keyboard_key) {
    if (KEYBOARD_KEY_NAMES.size() == 0) {
        KEYBOARD_KEY_NAMES = INIT_KEYBOARD_KEY_NAMES();
    }
    return (keyboard_key == KeyboardKey::Unknown) ? EMPTY_STRING : KEYBOARD_KEY_NAMES[keyboard_key];
}
const bool KeyboardKey::isModifierKey(const KeyboardKey::Key key) {
    return (key == KeyboardKey::LeftShift || key == KeyboardKey::RightShift || key == KeyboardKey::LeftControl || key == KeyboardKey::RightControl || key == KeyboardKey::LeftAlt || key == KeyboardKey::RightAlt) ? true : false;
}

const std::string& MouseButton::toString(const MouseButton::Button mouse_button) {
    if (MOUSE_BUTTON_NAMES.size() == 0) {
        MOUSE_BUTTON_NAMES = INIT_MOUSE_BUTTON_NAMES();
    }
    return (mouse_button == MouseButton::Unknown) ? EMPTY_STRING : MOUSE_BUTTON_NAMES[mouse_button];
}
const std::string& MouseWheel::toString(const MouseWheel::Wheel mouse_wheel) {
    if (MOUSE_WHEEL_NAMES.size() == 0) {
        MOUSE_WHEEL_NAMES = INIT_MOUSE_WHEEL_NAMES();
    }
    return (mouse_wheel == MouseWheel::Unknown) ? EMPTY_STRING : MOUSE_WHEEL_NAMES[mouse_wheel];
}
const std::string& JoystickAxis::toString(const JoystickAxis::Axis joystick_axis) {
    if (JOYSTICK_CONTROL_NAMES.size() == 0) {
        JOYSTICK_CONTROL_NAMES = INIT_JOYSTICK_CONTROL_NAMES();
    }
    return (joystick_axis == JoystickAxis::Unknown) ? EMPTY_STRING : JOYSTICK_CONTROL_NAMES[joystick_axis];
}