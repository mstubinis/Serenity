#pragma once
#ifndef ENGINE_SOUND_INCLUDES_H
#define ENGINE_SOUND_INCLUDES_H

#include <serenity/system/Macros.h>
#include <glm/vec3.hpp>

enum class SoundStatus : uint8_t {
    Fresh,
    Playing,
    PlayingLooped,
    Paused,
    Stopped,
};

#endif