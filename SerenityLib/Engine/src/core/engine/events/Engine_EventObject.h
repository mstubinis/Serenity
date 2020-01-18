#pragma once
#ifndef ENGINE_EVENT_OBJECT_H
#define ENGINE_EVENT_OBJECT_H

#include <core/engine/events/Engine_EventIncludes.h>

class  Scene;
namespace Engine{
namespace epriv{
    struct EventWindowResized final{ 
        unsigned int  width;
        unsigned int  height;
        EventWindowResized() = default;
        EventWindowResized(const unsigned int& _width, const unsigned int& _height) {
            width = _width;
            height = _height;
        }
    };
    struct EventWindowFullscreenChanged final{ 
        bool isFullscreen; 
        EventWindowFullscreenChanged() = default;
        EventWindowFullscreenChanged(const bool& _isFullscreen) {
            isFullscreen = _isFullscreen;
        }
    };
    struct EventKeyboard final{ 
        KeyboardKey::Key  key;
        bool              alt, control, shift, system;
        EventKeyboard() = default;
        EventKeyboard(const KeyboardKey::Key& _key, const bool& _alt, const bool& _control, const bool& _shift, const bool& _system) {
            key     = _key;
            alt     = _alt;
            control = _control;
            shift   = _shift;
            system  = _system;
        }
    };
    struct EventTextEntered final{ 
        std::uint32_t  unicode;
        EventTextEntered() = default;
        EventTextEntered(const std::uint32_t& _unicode) {
            unicode = _unicode;
        }
        const std::string convert() const {
            if (unicode == 27 || unicode == 13 || unicode == 9 || unicode == 8) { //27 = esc, 13 = enter, 9 = tab, 8 = backspace
                return "";
            }
            wchar_t c = static_cast<wchar_t>(unicode);
            std::wstring ws(&c);
            std::string res;
            for (auto& c : ws) {
                res += static_cast<char>(c);
            }
            res = res[0];
            return res;
        }
    };
    struct EventMouseButton final{ 
        MouseButton::Button button; float x, y; 
        EventMouseButton() = default;
        EventMouseButton(const MouseButton::Button& _button, const float& _x, const float& _y) {
            button = _button;
            x      = _x;
            y      = _y;
        }
    };
    struct EventMouseMove final{ 
        float x,y;
        EventMouseMove() = default;
        EventMouseMove(const float& _x, const float& _y) {
            x = _x;
            y = _y;
        }
    };
    struct EventMouseWheel final{ 
        int delta; 
        int x, y;
        EventMouseWheel() = default;
        EventMouseWheel(const int& delta_, const int& x_, const int& y_) {
            delta = delta_;
            x = x_;
            y = y_;
        }
    };
    struct EventJoystickMoved final{ 
        unsigned int        joystickID;
        JoystickAxis::Axis  axis;
        float               position;
        EventJoystickMoved() = default;
        EventJoystickMoved(const unsigned int& _joystickID, const JoystickAxis::Axis& _axis, const float& _position) {
            joystickID = _joystickID;
            axis       = _axis;
            position   = _position;
        }
    };
    struct EventJoystickButton final{ 
        unsigned int  joystickID;
        unsigned int  button;
        EventJoystickButton() = default;
        EventJoystickButton(const unsigned int& _joystickID, const unsigned int& _button) {
            joystickID = _joystickID;
            button     = _button;
        }
    };
    struct EventJoystickConnection final{ 
        unsigned int  joystickID;
        EventJoystickConnection() = default;
        EventJoystickConnection(const unsigned int& _joystickID) {
            joystickID = _joystickID;
        }
    };
    struct EventSoundStatusChanged final {
        unsigned int  status;
        EventSoundStatusChanged() = default;
        EventSoundStatusChanged(const unsigned int& _status) {
            status = _status;
        }
    };
    struct EventSceneChanged final{ 
        Scene *oldScene, *newScene; 
        EventSceneChanged() = default;
        EventSceneChanged(Scene* _old, Scene* _new) {
            oldScene = _old; newScene = _new;
        }
    };
};
};
struct Event final{
    EventType::Type type;
    union{
        Engine::epriv::EventWindowResized              eventWindowResized;
        Engine::epriv::EventWindowFullscreenChanged    eventWindowFullscreenChanged;
        Engine::epriv::EventKeyboard                   eventKeyboard;
        Engine::epriv::EventTextEntered                eventTextEntered;
        Engine::epriv::EventMouseButton                eventMouseButton;
        Engine::epriv::EventMouseMove                  eventMouseMoved;
        Engine::epriv::EventMouseWheel                 eventMouseWheel;
        Engine::epriv::EventJoystickMoved              eventJoystickMoved;
        Engine::epriv::EventJoystickButton             eventJoystickButton;
        Engine::epriv::EventJoystickConnection         eventJoystickConnection;
        Engine::epriv::EventSoundStatusChanged         eventSoundStatusChanged;
        Engine::epriv::EventSceneChanged               eventSceneChanged;
    };
    Event() = delete;
    Event(const EventType::Type& type_) {
        type = type_;
    }

};
/*
Inherit from this struct to expose your class to events and event dispatching, specifically the following functions:
    void registerEvent(const EventType::Type& type)    -  register this object as an observer to the parameterized event type
    void unregisterEvent(const EventType::Type& type)  -  unregister this object as an observer to the parameterized event type
    virtual void onEvent(const Event& e)               -  execute this function when the parameter event occurs
*/
struct EventObserver{
    virtual ~EventObserver(){}

    void registerEvent(const EventType::Type& type);
    void unregisterEvent(const EventType::Type& type);
    virtual void onEvent(const Event& e) {}
};
#endif