#pragma once
#ifndef ENGINE_ECS_SYSTEM_COMPONENT_LOGIC3_H
#define ENGINE_ECS_SYSTEM_COMPONENT_LOGIC3_H

class  ComponentLogic3;

#include <serenity/ecs/systems/SystemBaseClass.h>

class SystemComponentLogic3 final : public SystemCRTP<SystemComponentLogic3, ComponentLogic3> {
    private:

    public:
        SystemComponentLogic3(Engine::priv::ECS& ecs);
};

#endif