#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_TRANSFORM_H
#define ENGINE_ECS_SYSTEM_COMPONENT_TRANSFORM_H

class  ComponentTransform;
class  SystemTransformParentChild;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

//computes initial local and world matrices from transform component's LOCAL position, rotation, and scale
//does NOT put into account any sort of parent child relationship or rigid body collision positioning relationship
class SystemComponentTransform final : public SystemCRTP<SystemComponentTransform, ComponentTransform> {
    friend class  ComponentTransform;
    public:
        SystemComponentTransform(Engine::priv::ECS&);

        static void syncLocalVariablesToTransforms(SystemTransformParentChild*, Entity, ComponentTransform*);
};

#endif