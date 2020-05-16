#pragma once
#ifndef ENGINE_EVENT_ENUMS_H
#define ENGINE_EVENT_ENUMS_H

#include <SFML/System.hpp>
#include <string>

struct EventType final {
    enum Type : unsigned int {
        Unknown = 0,
        WindowResized,
        WindowGainedFocus,
        WindowLostFocus,

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
    };
};
struct KeyboardKey final{
    enum Key : unsigned int{
        A           = 0,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        Escape,
        LeftControl,
        LeftShift,
        LeftAlt,
        LeftSystem,
        RightControl,
        RightShift,
        RightAlt,
        RightSystem,
        Menu,
        LeftBracket,
        RightBracket,
        SemiColon,
        Comma,
        Period,
        Quote,
        Slash,
        BackSlash,
        Tilde,
        Equal,
        Dash,
        Space,
        Return,
        BackSpace,
        Tab,
        PageUp,
        PageDown,
        End,
        Home,
        Insert,
        Delete,
        Add,
        Subtract,
        Multiply,
        Divide,
        LeftArrow,
        RightArrow,
        UpArrow,
        DownArrow,
        Numpad0,
        Numpad1,
        Numpad2,
        Numpad3,
        Numpad4,
        Numpad5,
        Numpad6,
        Numpad7,
        Numpad8,
        Numpad9,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        Pause,

        _TOTAL,
        Unknown = 4294967295, //-1
    };
    static const std::string& toString(const KeyboardKey::Key keyboard_key);
    static const bool isModifierKey(const KeyboardKey::Key keyboard_key);
};
struct MouseButton final{
    enum Button : unsigned int{
        Left             = 0,
        Right,
        Middle,
        XButton1,
        XButton2,

        _TOTAL,
        Unknown = 4294967295, //-1
    };
    static const std::string& toString(const MouseButton::Button mouse_button);
};
struct MouseWheel final{
    enum Wheel : unsigned int{
        VerticalWheel    = 0,
        HorizontalWheel,

        _TOTAL,
        Unknown = 4294967295, //-1
    };
    static const std::string& toString(const MouseWheel::Wheel mouse_wheel);
};
struct JoystickAxis final{
    enum Axis : unsigned int{
        X                = 0,
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
    static const std::string& toString(const JoystickAxis::Axis joystick_axis);
};

#endif