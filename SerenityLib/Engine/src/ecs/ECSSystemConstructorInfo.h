#pragma once
#ifndef ENGINE_ECS_SYSTEM_CONSTRUCTOR_INFO_H
#define ENGINE_ECS_SYSTEM_CONSTRUCTOR_INFO_H

#include <ecs/Entity.h>

using std_func_update            = std::function<void(void*, void*, const float, Scene&)>;
using std_func_entity            = std::function<void(void*, void*, Entity, Scene&)>;
using std_func_component         = std::function<void(void*, void*, Entity)>;
using std_func_component_removed = std::function<void(void*, Entity)>;
using std_func_scene             = std::function<void(void*, void*, Scene&)>;

namespace Engine::priv {
    struct ECSSystemCI {
        std_func_update               onUpdateFunction                     = [](void*, void*, const float, Scene&) {};
        std_func_component            onComponentAddedToEntityFunction     = [](void*, void*, Entity) {};
        std_func_component_removed    onComponentRemovedFromEntityFunction = [](void*, Entity) {};
        std_func_entity               onEntityAddedToSceneFunction         = [](void*, void*, Entity, Scene&) {};
        std_func_scene                onSceneEnteredFunction               = [](void*, void*, Scene&) {};
        std_func_scene                onSceneLeftFunction                  = [](void*, void*, Scene&) {};

        virtual ~ECSSystemCI() = default;

        inline void setUpdateFunction(std_func_update&& func) noexcept {
            onUpdateFunction = std::move(func);
        }
        inline void setOnComponentAddedToEntityFunction(std_func_component&& func) noexcept {
            onComponentAddedToEntityFunction = std::move(func);
        }
        inline void setOnComponentRemovedFromEntityFunction(std_func_component_removed&& func) noexcept {
            onComponentRemovedFromEntityFunction = std::move(func);
        }
        inline void setOnEntityAddedToSceneFunction(std_func_entity&& func) noexcept {
            onEntityAddedToSceneFunction = std::move(func);
        }
        inline void setOnSceneEnteredFunction(std_func_scene&& func) noexcept {
            onSceneEnteredFunction = std::move(func);
        }
        inline void setOnSceneLeftFunction(std_func_scene&& func) noexcept {
            onSceneLeftFunction = std::move(func);
        }
    };
};
#endif