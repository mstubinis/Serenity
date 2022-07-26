#include <serenity/input/Mouse/MouseButtons.h>
#include <array>

constexpr std::array<const char*, MouseButton::_TOTAL> MOUSE_BUTTON_NAMES { {
    "Unknown",
    "Left Button",
    "Right Button",
    "Middle Button",
    "X Button 1",
    "X Button 2",
} };

constexpr std::array<const char*, MouseWheel::_TOTAL> MOUSE_WHEEL_NAMES { {
    "Unknown",
    "Vertical",
    "Horizontal",
} };


std::string MouseButton::toString(MouseButton mouse_button) {
    return MOUSE_BUTTON_NAMES[mouse_button];
}
std::string MouseWheel::toString(MouseWheel mouse_wheel) {
    return MOUSE_WHEEL_NAMES[mouse_wheel];
}

const char* MouseButton::toCharArray(MouseButton mouse_button) {
    return MOUSE_BUTTON_NAMES[mouse_button];
}
const char* MouseWheel::toCharArray(MouseWheel mouse_wheel) {
    return MOUSE_WHEEL_NAMES[mouse_wheel];
}
