#pragma once
#ifndef ENGINE_EVENTS_JOYSTICK_AXISES_H
#define ENGINE_EVENTS_JOYSTICK_AXISES_H

struct JoystickAxis final {
    enum Axis : unsigned int {
        X = 0,
        Y,
        Z,
        R,
        U,
        V,
        PovX,
        PovY,

        _TOTAL,
        Unknown = 4294967295, //-1
    };
    static std::string toString(JoystickAxis::Axis joystick_axis);
    static const char* toCharArray(JoystickAxis::Axis joystick_axis);
};

#endif