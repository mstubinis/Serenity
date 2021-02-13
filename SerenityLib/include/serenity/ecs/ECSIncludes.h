#pragma once
#ifndef ENGINE_ECS_INCLUDES_H
#define ENGINE_ECS_INCLUDES_H

#include <serenity/system/TypeDefs.h>

constexpr uint32_t ID_BIT_POSITIONS      = 21U;
constexpr uint32_t SCENE_BIT_POSITIONS   = 7U;
constexpr uint32_t VERSION_BIT_POSITIONS = 4U;
constexpr uint32_t ENTITY_BIT_SIZE       = sizeof(uint32_t) * 8U;

#endif