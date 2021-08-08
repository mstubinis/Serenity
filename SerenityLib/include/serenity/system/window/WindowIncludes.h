#pragma once
#ifndef ENGINE_WINDOW_INCLUDES_H
#define ENGINE_WINDOW_INCLUDES_H

#include <cstdint>

class WindowMode final { public: enum Mode : uint8_t {
    Windowed = 0,
    WindowedFullscreen,
    Fullscreen,
};};

class Window_Flags final { public: enum Flag : uint16_t {
    WindowedFullscreen = 1 << 0,
    Windowed           = 1 << 1,
    Fullscreen         = 1 << 2,
    MouseVisible       = 1 << 3,
    Active             = 1 << 4,
    Vsync              = 1 << 5,
    MouseGrabbed       = 1 << 6,
    KeyRepeat          = 1 << 7,
};};

enum class WindowEventThreadOnlyCommands : uint32_t {
    ShowMouse,
    HideMouse,
    RequestFocus,
    KeepMouseInWindow,
    FreeMouseFromWindow,
_TOTAL};


#endif