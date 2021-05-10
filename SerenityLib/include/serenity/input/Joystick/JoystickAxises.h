#pragma once
#ifndef ENGINE_EVENTS_JOYSTICK_AXISES_H
#define ENGINE_EVENTS_JOYSTICK_AXISES_H

#include <string>
#include <cstdint>

class JoystickAxis final {
    public:
        enum Axis : uint32_t {
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
        [[nodiscard]] static const char* toCharArray(JoystickAxis::Axis);
        [[nodiscard]] static std::string toString(JoystickAxis::Axis);
};

#endif