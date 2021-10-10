#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_TRANSFORM_H
#define ENGINE_ECS_SYSTEM_COMPONENT_TRANSFORM_H

class  ComponentTransform;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

//computes initial local and world matrices from transform component's local position, rotation, and scale
class SystemComponentTransform final : public SystemCRTP<SystemComponentTransform, ComponentTransform> {
    friend class  ComponentTransform;
    public:
        SystemComponentTransform(Engine::priv::ECS&);
};

#endif