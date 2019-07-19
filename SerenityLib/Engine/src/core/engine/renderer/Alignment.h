#pragma once
#ifndef ENGINE_RENDERER_ALIGNMENT_H
#define ENGINE_RENDERER_ALIGNMENT_H
struct Alignment final {enum Type {
    TopLeft,
    TopCenter,
    TopRight,
    Left,
    Center,
    Right,
    BottomLeft,
    BottomCenter,
    BottomRight,
};};
#endif