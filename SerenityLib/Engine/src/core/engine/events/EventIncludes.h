#pragma once
#ifndef ENGINE_EVENT_INCLUDES_H
#define ENGINE_EVENT_INCLUDES_H

#include <SFML/System.hpp>
#include <string>

#include <core/engine/events/Keyboard/KeyboardKeys.h>
#include <core/engine/events/Mouse/MouseButtons.h>
#include <core/engine/events/Joystick/JoystickAxises.h>

struct EventType final { enum Type : unsigned int {
    Unknown = 0,
    WindowResized,
    WindowGainedFocus,
    WindowLostFocus,

    EnumChanged,
    EnumChanged2,
    EnumChanged3,
    EnumChanged4,
    EnumChanged5,
    EnumChanged6,
    EnumChanged7,
    EnumChanged8,
    EnumChanged9,

    SocketConnected,
    SocketDisconnected,
    ServerStarted,
    ServerShutdowned,
    ClientConnected,
    ClientDisconnected,

    PacketSent,
    PacketReceived,

    WindowRequestedToBeClosed,
    WindowHasClosed,
    GameEnded,

    MeshLoaded,
    MaterialLoaded,
    TextureLoaded,

    WindowFullscreenChanged,
    SoundStatusChanged,
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
    _TOTAL,
};};

#endif