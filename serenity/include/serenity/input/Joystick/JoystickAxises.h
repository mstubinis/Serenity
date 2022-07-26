#pragma once
#ifndef ENGINE_EVENTS_JOYSTICK_AXISES_H
#define ENGINE_EVENTS_JOYSTICK_AXISES_H

#include <string>
#include <serenity/system/Macros.h>

class JoystickAxis final {
    public:
        enum Type : uint32_t {
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
        [[nodiscard]] static const char* toCharArray(JoystickAxis);
        [[nodiscard]] static std::string toString(JoystickAxis);

        BUILD_ENUM_CLASS_MEMBERS(JoystickAxis, Type)
};

#endif