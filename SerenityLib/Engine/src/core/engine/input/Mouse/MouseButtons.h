#pragma once
#ifndef ENGINE_EVENTS_MOUSE_BUTTONS_H
#define ENGINE_EVENTS_MOUSE_BUTTONS_H

#include <string>

struct MouseButton final {
    enum Button : unsigned int {
        Left = 0,
        Right,
        Middle,
        XButton1,
        XButton2,

        _TOTAL,
        Unknown = 4294967295, //-1
    };
    static const char* toCharArray(MouseButton::Button mouse_button);
    static std::string toString(MouseButton::Button mouse_button);
};
struct MouseWheel final {
    enum Wheel : unsigned int {
        VerticalWheel = 0,
        HorizontalWheel,

        _TOTAL,
        Unknown = 4294967295, //-1
    };
    static const char* toCharArray(MouseWheel::Wheel mouse_wheel);
    static std::string toString(MouseWheel::Wheel mouse_wheel);
};

#endif