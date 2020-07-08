#include <core/engine/input/Joystick/JoystickAxises.h>
#include <array>

constexpr std::array<const char*, JoystickAxis::_TOTAL> JOYSTICK_CONTROL_NAMES{ {
    "Joystick X",
    "Joystick Y",
    "Joystick Z",
    "Joystick R",
    "Joystick U",
    "Joystick V",
    "Joystick Pov X",
    "Joystick Pov Y",
} };


std::string JoystickAxis::toString(JoystickAxis::Axis joystick_axis) {
    return (joystick_axis == JoystickAxis::Unknown) ? "Unknown Joystick Axis" : JOYSTICK_CONTROL_NAMES[joystick_axis];
}
const char* JoystickAxis::toCharArray(JoystickAxis::Axis joystick_axis) {
    return (joystick_axis == JoystickAxis::Unknown) ? "Unknown Joystick Axis" : JOYSTICK_CONTROL_NAMES[joystick_axis];
}