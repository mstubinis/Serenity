#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_TRANSFORM_DEBUG_DRAW_H
#define ENGINE_ECS_SYSTEM_COMPONENT_TRANSFORM_DEBUG_DRAW_H

class  ComponentTransform;
class  ComponentModel;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemComponentTransformDebugDraw final : public SystemCRTP<SystemComponentTransformDebugDraw, ComponentTransform, ComponentModel> {
    friend class  ComponentTransform;
    friend class  ComponentModel;
    private:

    public:
        SystemComponentTransformDebugDraw(Engine::priv::ECS&);
};

#endif