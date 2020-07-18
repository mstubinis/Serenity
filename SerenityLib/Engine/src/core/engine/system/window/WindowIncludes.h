#pragma once
#ifndef ENGINE_WINDOW_INCLUDES_H
#define ENGINE_WINDOW_INCLUDES_H

class Window_Flags final {
    public: enum Flag : unsigned short {
        WindowedFullscreen = 1 << 0,
        Windowed           = 1 << 1,
        Fullscreen         = 1 << 2,
        MouseVisible       = 1 << 3,
        Active             = 1 << 4,
        Vsync              = 1 << 5,
        MouseGrabbed       = 1 << 6,
        KeyRepeat          = 1 << 7,
    };
};

class WindowEventThreadOnlyCommands final {
    public: enum Command : unsigned int {
        ShowMouse,
        HideMouse,
        RequestFocus,
        KeepMouseInWindow,
        FreeMouseFromWindow,
    _TOTAL};
};

#endif