#pragma once
#ifndef ENGINE_EVENTS_MOUSE_BUTTONS_H
#define ENGINE_EVENTS_MOUSE_BUTTONS_H

#include <string>
#include <cstdint>

class MouseButton final {
    public:
        enum Button : uint32_t {
            Left = 0,
            Right,
            Middle,
            XButton1,
            XButton2,

            _TOTAL,
            Unknown = 4294967295, //-1
        };
        [[nodiscard]] static const char* toCharArray(MouseButton::Button);
        [[nodiscard]] static std::string toString(MouseButton::Button);
        [[nodiscard]] static const char* toCharArray(uint32_t mouse_button);
        [[nodiscard]] static std::string toString(uint32_t mouse_button);
};
class MouseWheel final {
    public:
        enum Wheel : uint32_t {
            VerticalWheel = 0,
            HorizontalWheel,

            _TOTAL,
            Unknown = 4294967295, //-1
        };
        [[nodiscard]] static const char* toCharArray(MouseWheel::Wheel);
        [[nodiscard]] static std::string toString(MouseWheel::Wheel);
        [[nodiscard]] static const char* toCharArray(uint32_t mouse_wheel);
        [[nodiscard]] static std::string toString(uint32_t mouse_wheel);
};

#endif