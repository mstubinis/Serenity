#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/input/Mouse/MouseButtons.h>

constexpr std::array<const char*, MouseButton::_TOTAL> MOUSE_BUTTON_NAMES { {
    "Left Button",
    "Right Button",
    "Middle Button",
    "X Button 1",
    "X Button 2",
} };

constexpr std::array<const char*, MouseWheel::_TOTAL> MOUSE_WHEEL_NAMES { {
    "Vertical",
    "Horizontal",
} };


std::string MouseButton::toString(MouseButton::Button mouse_button) {
    return (mouse_button == MouseButton::Unknown) ? "Unknown Mouse Button" : MOUSE_BUTTON_NAMES[mouse_button];
}
std::string MouseWheel::toString(MouseWheel::Wheel mouse_wheel) {
    return (mouse_wheel == MouseWheel::Unknown) ? "Unknown Mouse Button" : MOUSE_WHEEL_NAMES[mouse_wheel];
}

const char* MouseButton::toCharArray(MouseButton::Button mouse_button) {
    return (mouse_button == MouseButton::Unknown) ? "Unknown Mouse Button" : MOUSE_BUTTON_NAMES[mouse_button];
}
const char* MouseWheel::toCharArray(MouseWheel::Wheel mouse_wheel) {
    return (mouse_wheel == MouseWheel::Unknown) ? "Unknown Mouse Button" : MOUSE_WHEEL_NAMES[mouse_wheel];
}