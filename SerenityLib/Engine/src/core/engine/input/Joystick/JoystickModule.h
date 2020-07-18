#pragma once
#ifndef ENGINE_EVENTS_JOYSTICK_MODULE_H
#define ENGINE_EVENTS_JOYSTICK_MODULE_H

#include <core/engine/input/Joystick/JoystickAxises.h>

namespace Engine::priv {
    class JoystickModule {
        private:

        public:
            JoystickModule();
            virtual ~JoystickModule();
    };
}


#endif