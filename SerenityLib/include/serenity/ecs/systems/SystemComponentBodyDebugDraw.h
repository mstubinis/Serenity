#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_BODY_DEBUG_DRAW_H
#define ENGINE_ECS_SYSTEM_COMPONENT_BODY_DEBUG_DRAW_H

class  ComponentBody;
class  ComponentBodyRigid;
class  ComponentModel;

#include <serenity/ecs/systems/SystemBaseClass.h>
#include <cstdint>
#include <serenity/dependencies/glm.h>

class SystemComponentBodyDebugDraw final : public SystemCRTP<SystemComponentBodyDebugDraw, ComponentBody, ComponentBodyRigid, ComponentModel> {
    friend class  ComponentBody;
    friend class  ComponentBodyRigid;
    friend class  ComponentModel;
    private:

    public:
        SystemComponentBodyDebugDraw(Engine::priv::ECS& ecs);
};

#endif