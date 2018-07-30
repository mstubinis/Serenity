#pragma once
#ifndef ENGINE_EVENT_OBJECT_H
#define ENGINE_EVENT_OBJECT_H

#include <string>
#include <cstdint>
#include "Engine_EventEnums.h"

struct EventType final{enum Type{
    WindowResized,
    WindowGainedFocus,
    WindowLostFocus,
    WindowClosed,
    SoundStartedPlaying,
    SoundStoppedPlaying,
    SoundPaused,
    KeyPressed,
    KeyReleased,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseWheelMoved,
    MouseMoved,
    MouseEnteredWindow,
    MouseLeftWindow,
    JoystickConnected,
    JoystickDisconnected,
    JoystickButtonPressed,
    JoystickButtonReleased,
    JoystickMoved,
    TextEntered,
_TOTAL};};

namespace Engine{
    namespace epriv{
        struct EventWindowResized final{ uint width,height; };
        struct EventKeyboard final{ KeyboardKey::Key key; bool alt; bool control; bool shift; bool system; };
        struct EventTextEntered final{ std::uint32_t unicode; };
        struct EventMouseButton final{ MouseButton::Button button; float x, y; };
        struct EventMouseMove final{ float x,y; };
        struct EventMouseWheel final{ int delta; };
        struct EventJoystickMoved final{ uint joystickID; JoystickAxis::Axis axis; float position; };
        struct EventJoystickButton final{ uint joystickID; uint button; };
        struct EventJoystickConnection final{ uint joystickID; };
    };
};

class Event final{
    public:
        EventType::Type type;

        union{
            Engine::epriv::EventWindowResized eventWindowResized;
            Engine::epriv::EventKeyboard eventKeyboard;
            Engine::epriv::EventTextEntered eventTextEntered;
            Engine::epriv::EventMouseButton eventMouseButton;
            Engine::epriv::EventMouseMove eventMouseMoved;
            Engine::epriv::EventMouseWheel eventMouseWheel;
            Engine::epriv::EventJoystickMoved eventJoystickMoved;
            Engine::epriv::EventJoystickButton eventJoystickButton;
            Engine::epriv::EventJoystickConnection eventJoystickConnection;
        };
};


#endif