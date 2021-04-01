#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_BODY_DEBUG_DRAW_H
#define ENGINE_ECS_SYSTEM_COMPONENT_BODY_DEBUG_DRAW_H

class  ComponentBody;
class  ComponentModel;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemComponentBodyDebugDraw final : public SystemCRTP<SystemComponentBodyDebugDraw, ComponentBody, ComponentModel> {
    friend class  ComponentBody;
    friend class  ComponentModel;
    private:

    public:
        SystemComponentBodyDebugDraw(Engine::priv::ECS& ecs);
};

#endif