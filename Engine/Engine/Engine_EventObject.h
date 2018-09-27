#pragma once
#ifndef ENGINE_EVENT_OBJECT_H
#define ENGINE_EVENT_OBJECT_H

#include "Engine_EventEnums.h"
#include "Engine_Utils.h"

class Scene;

struct EventType final{enum Type{
    WindowResized,
    WindowGainedFocus,
    WindowLostFocus,
    WindowClosed,
    WindowFullscreenChanged,
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
    SceneChanged,
_TOTAL};};

namespace Engine{
    namespace epriv{
        struct EventWindowResized final{ uint width,height; };
        struct EventWindowFullscreenChanged final{ bool isFullscreen; };
        struct EventKeyboard final{ KeyboardKey::Key key; bool alt; bool control; bool shift; bool system; };
        struct EventTextEntered final{ std::uint32_t unicode; };
        struct EventMouseButton final{ MouseButton::Button button; float x, y; };
        struct EventMouseMove final{ float x,y; };
        struct EventMouseWheel final{ int delta; };
        struct EventJoystickMoved final{ uint joystickID; JoystickAxis::Axis axis; float position; };
        struct EventJoystickButton final{ uint joystickID; uint button; };
        struct EventJoystickConnection final{ uint joystickID; };
        struct EventSceneChanged final{ Scene *oldScene, *newScene; };
    };
};
struct Event final{
    EventType::Type type;
    union{
        Engine::epriv::EventWindowResized eventWindowResized;
        Engine::epriv::EventWindowFullscreenChanged eventWindowFullscreenChanged;
        Engine::epriv::EventKeyboard eventKeyboard;
        Engine::epriv::EventTextEntered eventTextEntered;
        Engine::epriv::EventMouseButton eventMouseButton;
        Engine::epriv::EventMouseMove eventMouseMoved;
        Engine::epriv::EventMouseWheel eventMouseWheel;
        Engine::epriv::EventJoystickMoved eventJoystickMoved;
        Engine::epriv::EventJoystickButton eventJoystickButton;
        Engine::epriv::EventJoystickConnection eventJoystickConnection;
        Engine::epriv::EventSceneChanged eventSceneChanged;
    };
};
/*
Inherit from this class to expose your class to events and event dispatching, specifically the following functions:
    void registerEvent(const EventType::Type& type)    -  register this object as an observer to the parameterized event type
    void unregisterEvent(const EventType::Type& type)  -  unregister this object as an observer to the parameterized event type
    virtual void onEvent(const Event& e)               -  execute this function when the parameter event occurs
*/
class EventObserver: private Engine::epriv::noncopyable{
    public:
        EventObserver();
        virtual ~EventObserver();

        void registerEvent(const EventType::Type& type);
        void unregisterEvent(const EventType::Type& type);
        virtual void onEvent(const Event& e);
};
#endif