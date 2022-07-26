#pragma once
#ifndef ENGINE_EVENTS_JOYSTICK_MODULE_H
#define ENGINE_EVENTS_JOYSTICK_MODULE_H

#include <serenity/input/Joystick/JoystickAxises.h>
#include <serenity/system/TypeDefs.h>

namespace Engine::priv {
    class JoystickModule final {
        using JoystickStatusArray = std::vector<bool>;
        private:
                
        public:
            void onWindowLostFocus() noexcept;
            void onWindowGainedFocus() noexcept;
    };
}


#endif