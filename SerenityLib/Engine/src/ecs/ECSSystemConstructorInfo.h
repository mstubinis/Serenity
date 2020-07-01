#pragma once
#ifndef ENGINE_ECS_SYSTEM_CONSTRUCTOR_INFO_H
#define ENGINE_ECS_SYSTEM_CONSTRUCTOR_INFO_H

#include <functional>
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

        ECSSystemCI()                                        = default;
        virtual ~ECSSystemCI()                               = default;

        ECSSystemCI(const ECSSystemCI&)                      = delete;
        ECSSystemCI& operator=(const ECSSystemCI&)           = delete;
        ECSSystemCI(ECSSystemCI&& other) noexcept            = delete;
        ECSSystemCI& operator=(ECSSystemCI&& other) noexcept = delete;

        void setUpdateFunction(std_func_update func) noexcept {
            onUpdateFunction = func;
        }
        void setOnComponentAddedToEntityFunction(std_func_component func) noexcept {
            onComponentAddedToEntityFunction = func;
        }
        void setOnComponentRemovedFromEntityFunction(std_func_component_removed func) noexcept {
            onComponentRemovedFromEntityFunction = func;
        }
        void setOnEntityAddedToSceneFunction(std_func_entity func) noexcept {
            onEntityAddedToSceneFunction = func;
        }
        void setOnSceneEnteredFunction(std_func_scene func) noexcept {
            onSceneEnteredFunction = func;
        }
        void setOnSceneLeftFunction(std_func_scene func) noexcept {
            onSceneLeftFunction = func;
        }
    };
};
#endif