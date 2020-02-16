#pragma once
#ifndef ENGINE_SCENE_VIEWPORT_INCLUDES_H_INCLUDE_GUARD
#define ENGINE_SCENE_VIEWPORT_INCLUDES_H_INCLUDE_GUARD

struct ViewportFlag final { enum Flag : unsigned int {
    None   = 0,
    _1     = 1 << 0,
    _2     = 1 << 1,
    _3     = 1 << 2,
    _4     = 1 << 3,
    _5     = 1 << 4,
    _6     = 1 << 5,
    _7     = 1 << 6,
    _8     = 1 << 7,
    _9     = 1 << 8,
    _10    = 1 << 9,
    _11    = 1 << 10,
    _12    = 1 << 11,
    _13    = 1 << 13,
    _14    = 1 << 14,
    _15    = 1 << 15,
    All    = 4294967295,
};};

#endif