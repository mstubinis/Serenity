#pragma once
#ifndef ENGINE_ECS_INCLUDES_H
#define ENGINE_ECS_INCLUDES_H

constexpr std::uint32_t ID_BIT_POSITIONS      = 21U;
constexpr std::uint32_t SCENE_BIT_POSITIONS   = 7U;
constexpr std::uint32_t VERSION_BIT_POSITIONS = 4U;
constexpr std::uint32_t ENTITY_SIZE           = sizeof(std::uint32_t) * 8U;

#endif