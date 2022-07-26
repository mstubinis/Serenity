#pragma once
#ifndef ENGINE_EVENTS_MOUSE_BUTTONS_H
#define ENGINE_EVENTS_MOUSE_BUTTONS_H

#include <string>
#include <serenity/system/Macros.h>

class MouseButton final {
    public:
        enum Type : uint32_t {
            Unknown = 0,
            Left = 1,
            Right,
            Middle,
            XButton1,
            XButton2,

            _TOTAL,
        };
        [[nodiscard]] static const char* toCharArray(MouseButton);
        [[nodiscard]] static std::string toString(MouseButton);

        BUILD_ENUM_CLASS_MEMBERS(MouseButton, Type)
};

class MouseWheel final {
    public:
        enum Type : uint32_t {
            Unknown = 0,
            VerticalWheel = 1,
            HorizontalWheel,

            _TOTAL,
        };
        [[nodiscard]] static const char* toCharArray(MouseWheel);
        [[nodiscard]] static std::string toString(MouseWheel);

        BUILD_ENUM_CLASS_MEMBERS(MouseWheel, Type)
};

#endif