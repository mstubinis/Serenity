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


	ZZZTotal, //replace later with a constant?
};};

namespace Engine{
	namespace epriv{
	};
};

class Event final{
    public:
		EventType::Type type;

		struct EventWindowResized final{ uint width,height; };
		struct EventKeyboard final{ KeyboardKey::Key key; bool alt; bool control; bool shift; bool system; };
		struct EventTextEntered final{ std::uint32_t unicode; };
		struct EventMouseButton final{ MouseButton::Button button; float x, y; };
		struct EventMouseMove final{ float x,y; };
		struct EventMouseWheel final{ int delta; };
		struct EventJoystickMoved final{ uint joystickID; JoystickAxis::Axis axis; float position; };
		struct EventJoystickButton final{ uint joystickID; uint button; };
		struct EventJoystickConnection final{ uint joystickID; };

		union{
			EventWindowResized eventWindowResized;
			EventKeyboard eventKeyboard;
			EventTextEntered eventTextEntered;
			EventMouseButton eventMouseButton;
			EventMouseMove eventMouseMoved;
			EventMouseWheel eventMouseWheel;
			EventJoystickMoved eventJoystickMoved;
			EventJoystickButton eventJoystickButton;
			EventJoystickConnection eventJoystickConnection;
		};
};


#endif