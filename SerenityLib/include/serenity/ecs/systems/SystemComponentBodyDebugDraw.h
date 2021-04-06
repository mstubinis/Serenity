#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_BODY_DEBUG_DRAW_H
#define ENGINE_ECS_SYSTEM_COMPONENT_BODY_DEBUG_DRAW_H

class  ComponentTransform;
class  ComponentModel;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemComponentBodyDebugDraw final : public SystemCRTP<SystemComponentBodyDebugDraw, ComponentTransform, ComponentModel> {
    friend class  ComponentTransform;
    friend class  ComponentModel;
    private:

    public:
        SystemComponentBodyDebugDraw(Engine::priv::ECS& ecs);
};

#endif