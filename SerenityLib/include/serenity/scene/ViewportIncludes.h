#pragma once
#ifndef ENGINE_SCENE_VIEWPORT_INCLUDES_H
#define ENGINE_SCENE_VIEWPORT_INCLUDES_H

#include <serenity/system/Macros.h>

struct ViewportFlag final { 
    public:
        enum Type : uint32_t {
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
        };
        BUILD_ENUM_CLASS_MEMBERS(ViewportFlag, Type)
};

#endif